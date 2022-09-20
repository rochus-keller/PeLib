/* Software License Agreement
 *
 *     Copyright(C) 1994-2020 David Lindauer, (LADSoft)
 *     With modifications by me@rochus-keller.ch (2021)
 *
 *     This file is part of the Orange C Compiler package.
 *
 *     The Orange C Compiler package is free software: you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation, either version 3 of the License, or
 *     (at your option) any later version.
 *
 *     The Orange C Compiler package is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU General Public License for more details.
 *
 *     You should have received a copy of the GNU General Public License
 *     along with Orange C.  If not, see <http://www.gnu.org/licenses/>.
 *
 *     contact information:
 *         email: TouchStone222@runbox.com <David Lindauer>
 *
 */

#include "Method.h"
#include "Stream.h"
#include "PEWriter.h"
#include "MethodSignature.h"
#include "Value.h"
#include "Type.h"
#include "DataContainer.h"
#include "PELibError.h"
#include "Instruction.h"
#include "Operand.h"
#include <search.h>
#include <stdio.h>
#include <set>
#include <typeinfo>
#include <algorithm>
#include "SignatureGenerator.h"
#ifdef QT_CORE_LIB
#include <QtDebug>
#include "AssemblyDef.h"
#endif

namespace DotNetPELib
{
Method::Method(MethodSignature* Prototype, Qualifiers flags, bool entry) :
    CodeContainer(flags),
    prototype_(Prototype),
    maxStack_(100),
    invokeMode_(CIL),
    pInvokeType_(Stdcall),
    entryPoint_(entry),
    rendering_(nullptr),
    token_(0)
{
    if (!(flags_.Flags() & Qualifiers::Static))
        prototype_->Instance(true);
}

void Method::SetPInvoke(const std::string& name, Method::InvokeType type, const std::string& importName)
{
    invokeMode_ = PInvoke;
    pInvokeName_ = name;
    importName_ = importName;
    pInvokeType_ = type;
}
void Method::Instance(bool instance)
{
    if (instance)
    {
        Flags().Flags(Flags().Flags() | Qualifiers::Instance);
        Flags().Flags(Flags().Flags() & ~Qualifiers::Static);
    }
    else
    {
        Flags().Flags(Flags().Flags() & ~Qualifiers::Instance);
        Flags().Flags(Flags().Flags() | Qualifiers::Static);
    }
    if (prototype_)
        prototype_->Instance(instance);
}

void Method::AddLocal(Local* local)
{
    local->Index(varList_.size());
    varList_.push_back(local);
}

bool Method::ILSrcDump(Stream& peLib) const
{
    peLib.Out() << ".method";
    flags_.ILSrcDumpBeforeFlags(peLib);
    if (invokeMode_ == PInvoke)
    {
        peLib.Out() << " pinvokeimpl(\"" << pInvokeName_ << "\" " << (pInvokeType_ == Cdecl ? "cdecl) " : "stdcall) ");
    }
    else
    {
        peLib.Out() << " ";
    }
    prototype_->ILSrcDump(peLib, invokeMode_ != PInvoke, false, invokeMode_ == PInvoke);
    flags_.ILSrcDumpAfterFlags(peLib);
    if (invokeMode_ != PInvoke)
    {
        peLib.Out() << "{" << std::endl;
        if ((prototype_->Flags() & MethodSignature::Vararg) && (prototype_->Flags() & MethodSignature::Managed))
        {
            // allow C# to use ...
            peLib.Out() << "\t.param\t[" << prototype_->ParamCount() << "]" << std::endl;
            peLib.Out() << "\t.custom instance void [mscorlib]System.ParamArrayAttribute::.ctor() = ( 01 00 00 00 )" << std::endl;
        }
        if (varList_.size())
        {
            peLib.Out() << "\t.locals (" << std::endl;
            for (auto it = varList_.begin(); it != varList_.end();)
            {

                peLib.Out() << "\t\t[" << (*it)->Index() << "]\t";
                if ((*it)->GetType()->GetBasicType() == Type::ClassRef)
                {
                    if ((*it)->GetType()->GetClass()->Flags().Flags() & Qualifiers::Value)
                        peLib.Out() << "valuetype ";
                    else
                        peLib.Out() << "class ";
                }
                (*it)->GetType()->ILSrcDump(peLib);
                peLib.Out() << " ";
                (*it)->ILSrcDump(peLib);
                ++it;
                if (it != varList_.end())
                    peLib.Out() << "," << std::endl;
                else
                    peLib.Out() << std::endl;
            }
            peLib.Out() << "\t)" << std::endl;
        }
        if (entryPoint_)
            peLib.Out() << "\t.entrypoint" << std::endl;
        peLib.Out() << "\t.maxstack " << maxStack_ << std::endl << std::endl;
        CodeContainer::ILSrcDump(peLib);
        peLib.Out() << "}" << std::endl;
    }
    else
    {
        peLib.Out() << "{}" << std::endl;
    }
    return true;
}

bool Method::PEDump(Stream& peLib)
{
    if (!IsPInvoke() && InAssemblyRef())
    {
        prototype_->PEDump(peLib, false);
    }
    else
    {
        if( rendering_ )
        {
#ifdef QT_CORE_LIB
            qWarning() << "already dumped" << GetContainer()->Name().c_str() << Signature()->Name().c_str();
#endif
            return true;
        }
        size_t sz;
        size_t methodSignature = 0;
        Byte* sig = nullptr;
        TableEntryBase* table;
        if( prototype_->ReturnType() )
        {
            if ( prototype_->ReturnType()->GetBasicType() == Type::ClassRef)
            {
                if (prototype_->ReturnType()->GetClass()->InAssemblyRef())
                {
                    prototype_->ReturnType()->GetClass()->PEDump(peLib);
                }
            }
            if( prototype_->ReturnType()->Modopt() && prototype_->ReturnType()->Modopt()->GetBasicType() == Type::ClassRef )
            {
                if (prototype_->ReturnType()->Modopt()->GetClass()->InAssemblyRef())
                {
                    prototype_->ReturnType()->Modopt()->GetClass()->PEDump(peLib);
                }
            }
        }
        if (prototype_->ParamCount())
        {
            // assign an index to any params...
            for (auto it = prototype_->begin(); it != prototype_->end(); ++it)
            {
                auto param = *it;
                Type* tp = param->GetType();
                if (tp->GetBasicType() == Type::ClassRef)
                {
                    if (!tp->PEIndex())
                    {
                        Byte buf[256];
                        tp->Render(peLib, buf);
                    }
                }
            }
        }
        if (varList_.size())
        {
            // assign type indexes to any types that haven't already been defined
            for (auto local : varList_)
            {
                Type* tp = local->GetType();
                if (tp->GetBasicType() == Type::ClassRef)
                {
                    if (!tp->PEIndex())
                    {
                        Byte buf[256];
                        tp->Render(peLib, buf);
                    }
                }
            }
            sig = SignatureGenerator::LocalVarSig(this, sz);
            methodSignature = peLib.PEOut().HashBlob(sig, sz);
            table = new StandaloneSigTableEntry(methodSignature);
            methodSignature = peLib.PEOut().AddTableEntry(table);
        }
        Instruction* last = nullptr;
        if (instructions_.size())
            last = instructions_.back();

        int peflags = 0;
        const bool isRuntime = flags_.Flags() & Qualifiers::Runtime;
        if(entryPoint_ )
            peflags |= PEMethod::EntryPoint;
        if( invokeMode_ == CIL && !isRuntime )
            peflags |= PEMethod::CIL;

#ifdef QT_CORE_LIB
        Q_ASSERT( rendering_ == 0 );
#endif
        rendering_ = new PEMethod( hasSEH_, peflags,
                                  peLib.PEOut().NextTableIndex(tMethodDef), maxStack_, varList_.size(),
                                  last ? last->Offset() + last->InstructionSize() : 0,
                                  methodSignature ? methodSignature | (tStandaloneSig << 24) : 0);
        token_ = rendering_->methodDef_ | (tMethodDef << 24);
        if (invokeMode_ == CIL)
        {
#ifdef QT_CORE_LIB
            if( isRuntime && !instructions_.empty() || !isRuntime && instructions_.empty() )
                qWarning() << "Invalid method\t" << GetContainer()->getAssembly()->Name().c_str() << GetContainer()->Name().c_str() << Signature()->Name().c_str();
#endif
            peLib.PEOut().AddMethod(rendering_);
            peLib.addMethod(this);
        }
        delete[] sig;

        int implFlags = 0;
        int MFlags = 0;
        if (flags_.Flags() & Qualifiers::CIL)
            implFlags |= MethodDefTableEntry::IL;
        else if (flags_.Flags() & Qualifiers::Runtime)
            implFlags |= MethodDefTableEntry::Runtime;
        if (flags_.Flags() & Qualifiers::Managed)
            implFlags |= MethodDefTableEntry::Managed;
        if (flags_.Flags() & Qualifiers::PreserveSig)
            implFlags |= MethodDefTableEntry::PreserveSig;
        if (flags_.Flags() & Qualifiers::Public)
            MFlags |= MethodDefTableEntry::Public;
        else if (flags_.Flags() & Qualifiers::Private)
            MFlags |= MethodDefTableEntry::Private;
        if (flags_.Flags() & Qualifiers::Virtual)
            MFlags |= MethodDefTableEntry::Virtual;
        if (flags_.Flags() & Qualifiers::NewSlot)
            MFlags |= MethodDefTableEntry::NewSlot;
        if (flags_.Flags() & Qualifiers::Static)
            MFlags |= MethodDefTableEntry::Static;
        if (flags_.Flags() & Qualifiers::SpecialName)
            MFlags |= MethodDefTableEntry::SpecialName;
        if (flags_.Flags() & Qualifiers::RTSpecialName)
            MFlags |= MethodDefTableEntry::RTSpecialName;
        if (flags_.Flags() & Qualifiers::HideBySig)
            MFlags |= MethodDefTableEntry::HideBySig;
        if (invokeMode_ == PInvoke)
        {
            MFlags |= MethodDefTableEntry::PinvokeImpl;
        }
        size_t nameIndex = peLib.PEOut().HashString(prototype_->Name());
        size_t importNameIndex = nameIndex;
        if( !importName_.empty() )
            importNameIndex = peLib.PEOut().HashString(importName_);
        size_t paramIndex = peLib.PEOut().NextTableIndex(tParam);

        sig = SignatureGenerator::MethodDefSig(prototype_, sz);
        methodSignature = peLib.PEOut().HashBlob(sig, sz);
        delete[] sig;

        table = new MethodDefTableEntry(rendering_, implFlags, MFlags, nameIndex, methodSignature, paramIndex);
        prototype_->PEIndex(peLib.PEOut().AddTableEntry(table));
        int i = 1;
        size_t lastParamIndex = 0;
        for (auto it = prototype_->begin(); it != prototype_->end(); ++it)
        {
            int flags = 0;
            size_t nameIndex = peLib.PEOut().HashString((*it)->Name());
            TableEntryBase* table = new ParamTableEntry(flags, i++, nameIndex);
            lastParamIndex = peLib.PEOut().AddTableEntry(table);
        }

        if (invokeMode_ == PInvoke)
        {
            int Flags = 0;
            //            Flags |= ImplMapTableEntry::CharSetAnsi;
            if (pInvokeType_ == Cdecl)
                Flags |= ImplMapTableEntry::CallConvCdecl;
            else
                Flags |= ImplMapTableEntry::CallConvStdcall;
            size_t moduleName = peLib.PEOut().HashString(pInvokeName_);
            size_t moduleRef = peLib.moduleRefs[moduleName];
            if (moduleRef == 0)
            {
                TableEntryBase* table = new ModuleRefTableEntry(moduleName);
                moduleRef = peLib.PEOut().AddTableEntry(table);
                peLib.moduleRefs[moduleName] = moduleRef;
            }
            MemberForwarded methodIndex(MemberForwarded::MethodDef, prototype_->PEIndex());
            ImplMapTableEntry* table = new ImplMapTableEntry(Flags, methodIndex, importNameIndex, moduleRef);
            peLib.PEOut().AddTableEntry(table);
        }
        if ((prototype_->Flags() & MethodSignature::Vararg) && (prototype_->Flags() & MethodSignature::Managed))
        {
            size_t attributeType = peLib.PEOut().ParamAttributeType();
            size_t attributeData = peLib.PEOut().ParamAttributeData();
            if (!attributeType && !attributeData)
            {
                size_t ctor_index = 0;
                Resource* result = nullptr;
                peLib.Find("System.ParamArrayAttribute::.ctor", &result);
                if (result)
                {
                    static_cast<Method*>(result)->PEDump(peLib);
                    ctor_index = static_cast<Method*>(result)->Signature()->PEIndexCallSite();
                }
                static Byte data[] = {1, 0, 0, 0};
                size_t data_sig = peLib.PEOut().HashBlob(data, sizeof(data));
                peLib.PEOut().ParamAttribute(ctor_index, data_sig);
                attributeType = peLib.PEOut().ParamAttributeType();
                attributeData = peLib.PEOut().ParamAttributeData();
            }
            CustomAttribute attribute(CustomAttribute::ParamDef, lastParamIndex);
            CustomAttributeType type(CustomAttributeType::MethodRef, attributeType);
            TableEntryBase* table = new CustomAttributeTableEntry(attribute, type, attributeData);
            peLib.PEOut().AddTableEntry(table);
        }
    }
    return true;
}
void Method::Compile(Stream& peLib)
{

    rendering_->code_ = CodeContainer::Compile(peLib, rendering_->codeSize_);
    // code_ and codeSize_ are zero if no instruction, e.g. in delegate impls; seems a legal outcome
    CodeContainer::CompileSEH(rendering_->sehData_);
}
void Method::Optimize()
{
    //CalculateLive();
    //CalculateMaxStack();
    //OptimizeLocals();
    CodeContainer::Optimize();
}
void Method::CalculateLive()
{
    std::set<std::string> labelsReached;
    bool done = false;
    bool skipping = false;
    while (!done)
    {
        done = true;
        for (auto instruction : instructions_)
        {
            if (instruction->OpCode() == Instruction::i_SEH && instruction->SEHBegin())  // all SEH blocks are always live
            {
                instruction->Live(true);
                skipping = false;
            }
            else if (!skipping)
            {
                instruction->Live(true);
                if (instruction->IsBranch())
                {
                    if (labelsReached.find(instruction->GetOperand()->StringValue()) == labelsReached.end())
                    {
                        done = false;
                        labelsReached.insert(instruction->GetOperand()->StringValue());
                    }
                    if (instruction->OpCode() == Instruction::i_br)
                        skipping = true;
                }
                else if (instruction->OpCode() == Instruction::i_switch)
                {
                    if (instruction->GetSwitches())
                    {
                        for (auto its = instruction->GetSwitches()->begin(); its != instruction->GetSwitches()->end(); ++its)
                        {
                            if (labelsReached.find(*its) == labelsReached.end())
                            {
                                done = false;
                                labelsReached.insert(*its);
                            }
                        }
                    }
                }
            }
            else if (instruction->OpCode() == Instruction::i_label)
            {
                if (labelsReached.find(instruction->Label()) != labelsReached.end())
                {
                    instruction->Live(true);
                    skipping = false;
                }
            }
        }
    }
}
void Method::CalculateMaxStack()
{
    std::map<std::string, int> labels;
    maxStack_ = 0;
    int n = 0;
    bool lastBranch = false;
    for (Instruction * instruction : instructions_)
    {
        if (instruction->IsLive())
        {
            int m = instruction->StackUsage();
            if (m == -127)
                n = 0;
            else
                n += m;
            if (n > maxStack_)
                maxStack_ = n;
            if (n < 0)
            {
                throw PELibError(PELibError::StackUnderflow);
            }
            if (instruction->IsBranch())
            {
                lastBranch = true;
                auto it1 = labels.find(instruction->GetOperand()->StringValue());
                if (it1 != labels.end())
                {
                    if (it1->second != n)
                    {
                        throw PELibError(PELibError::MismatchedStack, instruction->GetOperand()->StringValue());
                    }
                }
                else
                {
                    labels[instruction->GetOperand()->StringValue()] = n;
                }
            }
            else if (instruction->OpCode() == Instruction::i_switch)
            {
                if (instruction->GetSwitches())
                {
                    for (auto its = instruction->GetSwitches()->begin(); its != instruction->GetSwitches()->end(); ++its)
                    {
                        auto it1 = labels.find(*its);
                        if (it1 != labels.end())
                        {
                            if (it1->second != n)
                            {
                                throw PELibError(PELibError::MismatchedStack, *its);
                            }
                        }
                        else
                        {
                            labels[*its] = n;
                        }
                    }
                }
            }
            else if (instruction->OpCode() == Instruction::i_label)
            {
                if (lastBranch)
                {
                    auto it1 = labels.find(instruction->Label());
                    if (it1 != labels.end())
                    {
                        n = it1->second;
                    }
                    else
                    {
                        n = 0;
                    }
                }
                else
                {
                    auto it1 = labels.find(instruction->Label());
                    if (it1 != labels.end())
                    {
                        if (it1->second != n)
                        {
                            throw PELibError(PELibError::MismatchedStack, instruction->Label());
                        }
                    }
                    else
                    {
                        labels[instruction->Label()] = n;
                    }
                }
            }
            else if (instruction->OpCode() == Instruction::i_comment)
            {
                // placeholder
            }
            else
            {
                lastBranch = false;
            }
        }
    }
    if (n != 0)
    {
        if (n != 1 || prototype_->ReturnType()->IsVoid())
            throw PELibError(PELibError::StackNotEmpty, " at end of function");
    }
}
void Method::OptimizeLocals()
{
    for (Instruction * instruction : instructions_)
    {
        Operand* op = instruction->GetOperand();
        if (op)
        {
            Value* v = op->GetValue();
            if (v && typeid(*v) == typeid(Local))
            {
                ((Local*)v)->IncrementUses();
            }
        }
    }
    std::sort(varList_.begin(), varList_.end(), [](const Local* left, const Local* right) { return left->Uses() > right->Uses(); });
    int index = 0;
    for (Local* a : varList_)
    {
        a->Index(index++);
    }
}
}  // namespace DotNetPELib
