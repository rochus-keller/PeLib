#include "PublicApi.h"
#include "SimpleApi.h"
#include <QCoreApplication>
#include <QtDebug>
using namespace DotNetPELib;

void test1()
{
    PELib peFile("test1");

    AssemblyDef* assembly = peFile.WorkingAssembly();

    peFile.MSCorLibAssembly();

    MethodSignature* sigMain = new MethodSignature("$Main", MethodSignature::Managed, assembly);
    sigMain->ReturnType( new Type(Type::Void) );
    Method* methMain = new Method( sigMain, Qualifiers::Private |
                                                        Qualifiers::Static |
                                                        Qualifiers::HideBySig |
                                                        Qualifiers::CIL |
                                                        Qualifiers::Managed, true);
    assembly->Add(methMain);

    std::vector<Type*> argList;
    argList.push_back( new Type( Type::string) );
    Method* methWriteLine = 0;
    if( peFile.Find( "System.Console.WriteLine", &methWriteLine, argList ) != PELib::s_method )
    {
        qCritical() << "cannot find WriteLine in mscorlib";
        return;
    }

    methMain->AddInstruction(
                new Instruction(Instruction::i_ldstr, new Operand("Hi there!", true)));
    methMain->AddInstruction(
                new Instruction(Instruction::i_call, new Operand( new MethodName( methWriteLine->Signature() ))));
    methMain->AddInstruction(
                new Instruction(Instruction::i_ret));

    try
    {
        methMain->Optimize(peFile);
    }catch (PELibError exc)
    {
        qCritical() << "Optimizer error: " << exc.what();
    }

    peFile.DumpOutputFile("HiThere.il", PELib::ilasm, false);
    peFile.DumpOutputFile("HiThere.exe", PELib::peexe, false);
}

void test2()
{
    SimpleApi api;

    try
    {
        api.beginModule("HelloWorld");
        api.addModuleReference("mscorlib");
        api.beginMethod( "Main", true, true, true );
        api.LDSTR("Hello World!");
        api.CALL("System.Console.WriteLine(string)");
        api.RET();
        api.endMethod();
        api.writeByteCode("test2.exe");
        api.writeAssembler("test2.il");
        api.endModule();
    }catch( const std::runtime_error& e )
    {
        qCritical() << e.what();
    }
}

void test3()
{
    SimpleApi api;

    try
    {
        api.beginModule("Test3");
        api.beginNamespace("Gugus");
        api.addEnum( "Color", true, SimpleApi::EnumItems() << SimpleApi::EnumItem("red",10) <<
                     SimpleApi::EnumItem("blue",20) << SimpleApi::EnumItem("green",30), true );
        api.addModuleReference("mscorlib");
        api.beginMethod( "Main", true, true, true );
        //api.LDC(12345);
        api.LDSFLD( "Gugus.Color.green" );
        api.CALL("System.Console.WriteLine(int32)");
        api.RET();
        api.endMethod();
        api.endNamespace();
        api.writeByteCode("test3.exe");
        api.writeAssembler("test3.il");
        api.endModule();
    }
    catch( const std::runtime_error& e )
    {
        qCritical() << e.what();
    }
}

void test4()
{
    SimpleApi api;

    try
    {
        api.beginModule("Test4");
        api.addModuleReference("mscorlib");

        api.addField("myString", "string", false, true );

        api.beginMethod(".cctor",false,true,false);
        api.LDSTR("Hello from .cctor!");
        api.STSFLD("myString");
        api.RET();
        api.endMethod();

        api.beginMethod("Test", false, true, false );
        api.addArgument("string&");
        api.LDARG(0);
        api.LDSTR("Hello from Test1!");
        api.STIND(SimpleApi::Ref);
        api.RET();
        api.endMethod();

        api.beginMethod( "Main", true, true, true );
        api.LDSFLD("myString");
        api.CALL("System.Console.WriteLine(string)");
        api.LDSFLDA("myString");
        api.CALL("Test");
        api.LDSFLD("myString");
        api.CALL("System.Console.WriteLine(string)");
        api.RET();
        api.endMethod();

        api.writeByteCode("test4.exe");
        api.writeAssembler("test4.il");
        api.endModule();
    }
    catch( const std::runtime_error& e )
    {
        qCritical() << e.what();
    }
}

void test5()
{
    SimpleApi api;

    try
    {
        api.beginModule("Test5");
        api.addModuleReference("mscorlib");

        api.beginClass("Outer", true);

        api.beginClass("Inner",true);

        api.beginMethod("DoIt", true, true, false );
        api.addArgument("string");
        api.LDARG(0);
        api.CALL("System.Console.WriteLine(string)");
        api.RET();
        api.endMethod();

        api.endClass();

        api.endClass();

        api.beginMethod( "Main", true, true, true );
        api.LDSTR("This is a string");
        api.CALL("Outer/Inner::DoIt");
        api.RET();
        api.endMethod();

        api.writeByteCode("test5.exe");
        api.writeAssembler("test5.il");
        api.endModule();
    }
    catch( const std::runtime_error& e )
    {
        qCritical() << e.what();
    }
}

void test6()
{
    SimpleApi api;

    try
    {
        api.beginModule("Test6");
        api.addModuleReference("mscorlib");

        api.addField("array1", "int8[]", true, true );

        api.beginStruct("MyStruct");
        api.addField("alpha", "string");
        api.addField("beta", "int32" );
        api.addField("gamma", "uint16[]");
        api.endStruct();

        api.addField("array2", "MyStruct[]",false, true);

        api.beginMethod( "Main", true, true, true );
        api.addLocal("MyStruct");

        api.LDC(10);
        api.NEWARR("int8[]");
        api.STSFLD("array1");
        api.LDC(3);
        api.NEWARR("MyStruct[]");
        api.STSFLD("array2");

        api.addField("array3", "int16[,]", false, false );
        api.LDTOKEN("System.Int16");
        api.CALL("System.Type::GetTypeFromHandle(System.RuntimeTypeHandle)");
        api.LDC(3);
        api.LDC(4);
        api.CALL("System.Array::CreateInstance(System.Type, int32, int32)");
        api.STSFLD("array3");

        api.LDSFLD("array3");
        api.LDC(123);
        api.BOX("System.Int16");
        api.LDC(1);
        api.LDC(2);
        api.CALLVIRT("System.Array::SetValue(object, int32, int32)");

        api.LDSFLD("array3");
        api.LDC(1);
        api.LDC(2);
        api.CALLVIRT("System.Array::GetValue(int32,int32)");
        api.UNBOX("System.Int16");
        api.CALL("System.Console.WriteLine(int32)");

        api.RET();
        api.endMethod();

        api.writeByteCode("test6.exe");
        api.writeAssembler("test6.il");
        api.endModule();
    }
    catch( const std::runtime_error& e )
    {
        qCritical() << e.what();
    }
}

int main()
{
    test1();
    test2();
    test3();
    test4();
    test5();
    test6();
    return 0;
}
