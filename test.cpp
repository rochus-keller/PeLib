#include "PELib.h"
#include "MethodSignature.h"
#include "Type.h"
#include "Qualifiers.h"
#include "Method.h"
#include "AssemblyDef.h"
#include "Instruction.h"
#include "Operand.h"
#include "Value.h"
#include "PELibError.h"
#include <QCoreApplication>
#include <QtDebug>
using namespace DotNetPELib;

int main()
{
    PELib peFile("HiThere", PELib::ilonly | PELib::bits32);

    AssemblyDef* assembly = peFile.WorkingAssembly();

    peFile.LoadAssembly("mscorlib");

    MethodSignature* sigMain = new MethodSignature("$Main",MethodSignature::Managed, assembly);
    sigMain->ReturnType(new Type(Type::Void, 0));
    Method* methMain = new Method( sigMain, Qualifiers::Private |
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
                new Instruction(Instruction::i_ldstr,
                                             new Operand("Hi there!", true)));
    methMain->AddInstruction(
                new Instruction(Instruction::i_call,
                                             new Operand(new MethodName(sigWriteLine))));
    methMain->AddInstruction(
                new Instruction(Instruction::i_ret, nullptr));

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
