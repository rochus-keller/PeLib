#include "DotNetPELib.h"
#include <QCoreApplication>
#include <QtDebug>
using namespace DotNetPELib;

int main()
{
    PELib peFile("HiThere", PELib::ilonly | PELib::bits32);

    AssemblyDef* assembly = peFile.WorkingAssembly();

    peFile.LoadAssembly("mscorlib");

    MethodSignature* sigMain = peFile.AllocateMethodSignature("$Main",MethodSignature::Managed, assembly);
    sigMain->ReturnType(peFile.AllocateType(Type::Void, 0));
    Method* methMain = peFile.AllocateMethod( sigMain, Qualifiers::Private |
                                                        Qualifiers::Static |
                                                        Qualifiers::HideBySig |
                                                        Qualifiers::CIL |
                                                        Qualifiers::Managed, true);
    assembly->Add(methMain);


    Type stringType(Type::string, 0);
    std::vector<Type*> argList;
    argList.push_back(&stringType);
    Method* methWriteLine = nullptr;
    MethodSignature* sigWriteLine;
    if( peFile.Find( "System.Console.WriteLine", &methWriteLine, argList ) == PELib::s_method )
        sigWriteLine = methWriteLine->Signature();
    else
    {
        qCritical() << "cannot find WriteLine in mscorlib";
        return -1;
    }

    methMain->AddInstruction(
                peFile.AllocateInstruction(Instruction::i_ldstr,
                                             peFile.AllocateOperand("Hi there!", true)));
    methMain->AddInstruction(
                peFile.AllocateInstruction(Instruction::i_call,
                                             peFile.AllocateOperand(peFile.AllocateMethodName(sigWriteLine))));
    methMain->AddInstruction(
                peFile.AllocateInstruction(Instruction::i_ret, nullptr));

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
