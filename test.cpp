#include "PublicApi.h"
#include <QCoreApplication>
#include <QtDebug>
using namespace DotNetPELib;

int main()
{
    PELib peFile("HiThere");

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
        return -1;
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

    return 0;
}
