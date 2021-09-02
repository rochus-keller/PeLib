#include "PublicApi.h"
#include <QtDebug>
using namespace DotNetPELib;


int main()
{
    PELib peFile("test1");


    AssemblyDef* assembly = peFile.WorkingAssembly();
    MethodSignature* sigMain = new MethodSignature("$Main", MethodSignature::Managed, assembly);
    sigMain->ReturnType( new Type(Type::Void) );
    Method* methMain = new Method( sigMain, Qualifiers::Private |
                                                        Qualifiers::Static |
                                                        Qualifiers::HideBySig |
                                                        Qualifiers::CIL |
                                                        Qualifiers::Managed, true);
    assembly->Add(methMain);


    AssemblyDef* mscorlib = peFile.AddExternalAssembly("mscorlib");

    MethodSignature* sigWriteLine = new MethodSignature("System.Console.WriteLine", MethodSignature::Managed, mscorlib);
    sigWriteLine->ReturnType( new Type(Type::Void) );
    sigWriteLine->AddParam(new Param("", new Type(Type::string)));
    /*
    Method* methWriteLine = new Method( sigWriteLine, Qualifiers::Public |
                                                        Qualifiers::Static |
                                                        Qualifiers::CIL |
                                                        Qualifiers::Managed, true);
    mscorlib->Add(methWriteLine);*/


    methMain->AddInstruction(
                new Instruction(Instruction::i_ldstr, new Operand("Hi there!", true)));
    methMain->AddInstruction(
                new Instruction(Instruction::i_call, new Operand( new MethodName( sigWriteLine ))));
    methMain->AddInstruction(
                new Instruction(Instruction::i_ret));

    try
    {
        methMain->Optimize(peFile);
    }catch (PELibError exc)
    {
        qCritical() << "Optimizer error: " << exc.what();
    }

    //peFile.DumpOutputFile("HiThere.il", PELib::ilasm, false);
    peFile.DumpOutputFile("HiThere.exe", PELib::peexe, false);


}
