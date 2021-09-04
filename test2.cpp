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


    AssemblyDef* mscorlib = peFile.MSCorLibAssembly();

    Resource* r;
    Class* console;
#if 1
    Namespace* system;
    Q_ASSERT( peFile.Find("System", &r) == PELib::s_namespace );
    system = (Namespace*)r;
    if( peFile.Find("Console", &r) != PELib::s_class )
    {
        console = new Class("Console",Qualifiers::Public,-1,-1 );
        system->Add(console);
    }else
        console = (Class*)r;
#else
    console = new Class("System.Console",Qualifiers::Public,-1,-1 );
    mscorlib->Add(console);
#endif

    MethodSignature* sigWriteLine = new MethodSignature("WriteLine", MethodSignature::Managed, console);
    sigWriteLine->ReturnType( new Type(Type::Void) );
    sigWriteLine->AddParam(new Param("", new Type(Type::string)));


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

    peFile.DumpOutputFile("HiThere.il", PELib::ilasm, false);
    peFile.DumpOutputFile("HiThere.exe", PELib::peexe, false);


}
