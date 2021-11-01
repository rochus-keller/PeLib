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

#include "MethodSignature.h"
#include "Stream.h"
#include "PEWriter.h"
#include "Type.h"
#include "Value.h"
#include "PELibError.h"
#include "DataContainer.h"
#include "Class.h"
#include "Method.h"
#include "AssemblyDef.h"
#include <stdio.h>
#include <sstream>
#include "SignatureGenerator.h"
#include <cassert>

namespace DotNetPELib
{
bool MethodSignature::MatchesType(Type *tpa, Type *tpp)
{
    if (!tpp)
    {
        return false;
    }
    else if (tpp->GetBasicType() == Type::TypeVar)
    {
        // nothing to do, it matches...
    }
    else if (tpp->GetBasicType() == Type::MethodParam)
    {
        // nothing to do, it matches...
    }
    else if (tpa->GetBasicType() == tpp->GetBasicType())
    {
        // this may need to deal with boxed types a little better
        if (tpa->GetBasicType() == Type::ClassRef)
            if (tpa->GetClass() != tpp->GetClass())
                return false;
    }
    else
    {
        return false;
    }
    if ((tpa->PointerLevel() != tpp->PointerLevel() && tpp->PointerLevel() != 1 && tpp->GetBasicType() != Type::Void) || tpa->ArrayLevel() != tpp->ArrayLevel())
        return false;
    return true;
}
bool MethodSignature::Matches(std::vector<Type*> args)
{
    // this is only designed for managed functions...
    if (args.size() == params.size() || (params.size() && args.size() >= params.size() - 1 && (flags_ & Vararg)))
    {
        auto it = params.begin();
        for (int i = 0, n = 0; i < args.size(); i++)
        {
            Type* tpa = args[i];
            Type* tpp = (*it)->GetType();
            if (!MatchesType(tpa, tpp))
                return false;
            if (n < params.size() - 1)
                n++, ++it;
        }
        return true;
    }
    return false;
}

void MethodSignature::AddParam(Param* param)
{
    if (varargParams_.size())
    {
        throw PELibError(PELibError::VarargParamsAlreadyDeclared);
    }
    if( param->Index() == -1 )
        param->Index(params.size());
    params.push_back(param);
}

void MethodSignature::AddVarargParam(Param* param)
{
    param->Index(params.size() + varargParams_.size());
    varargParams_.push_back(param);
}

Param*MethodSignature::getParam(int i, bool byOrdinal) const
{
    if( byOrdinal )
    {
        assert( i >= 0 && i < params.size() );
        return params[i];
    }
    std::deque<Param *>::const_iterator j;
    for( j = params.begin(); j != params.end(); ++j )
    {
        if( (*j)->Index() == i )
            return (*j);
    }
    assert(false);
    return 0;
}

void MethodSignature::Instance(bool instance)
{
    if (instance)
    {
        flags_ |= InstanceFlag;
    }
    else
        flags_ &= ~InstanceFlag;
}
bool MethodSignature::ILSrcDump(Stream& peLib, bool names, bool asType, bool PInvoke) const
{
    // this usage of vararg is for C style varargs
    // occil uses C# style varags except in pinvoke and generates
    // the associated object array argument
    if ((flags_ & Vararg) && !(flags_ & Managed))
    {
        peLib.Out() << "vararg ";
    }
    if (flags_ & InstanceFlag)
    {
        peLib.Out() << "instance ";
    }
    if (returnType_->GetBasicType() == Type::ClassRef)
    {
        if (returnType_->GetClass()->Flags().Flags() & Qualifiers::Value)
            peLib.Out() << "valuetype ";
        else
            peLib.Out() << "class ";
    }
    returnType_->ILSrcDump(peLib);
    peLib.Out() << " ";
    if (asType)
    {
        peLib.Out() << " *";
    }
    else if (name_.size())
    {
        if (arrayObject_)
        {
            arrayObject_->ILSrcDump(peLib);
            peLib.Out() << "::'" << name_ << "'";
        }
        else if (names)
        {
            peLib.Out() << "'" << name_ << "'";
        }
        else
        {
            if (container_ && typeid(*container_) == typeid(Class) && static_cast<Class*>(container_)->Generic().size())
            {
                if (container_->Flags().Flags() & Qualifiers::Value)
                    peLib.Out() << "valuetype ";
                else
                    peLib.Out() << "class ";
                peLib.Out() << Qualifiers::GetName("", container_);
                peLib.Out() << static_cast<Class*>(container_)->AdornGenerics(peLib);
                peLib.Out() << "::'" << name_ << "'";
            }
            else
            {
                peLib.Out() << Qualifiers::GetName(name_, container_);
            }
        }
    }
    peLib.Out() << AdornGenerics(peLib) << "(";
    for (std::deque<Param*>::const_iterator it = params.begin(); it != params.end();)
    {
        if ((*it)->GetType()->GetBasicType() == Type::ClassRef)
        {
            if ((*it)->GetType()->GetClass()->Flags().Flags() & Qualifiers::Value)
                peLib.Out() << "valuetype ";
            else
                peLib.Out() << "class ";
        }
        (*it)->GetType()->ILSrcDump(peLib);
        if (names && (*it)->GetType()->GetBasicType() != Type::TypeVar && (*it)->GetType()->GetBasicType() != Type::MethodParam)
            (*it)->ILSrcDump(peLib);
        ++it;
        if (it != params.end())
            peLib.Out() << ", ";
    }
    if (!PInvoke && (flags_ & Vararg))
    {
        if (!(flags_ & Managed))
        {
            peLib.Out() << ", ...";
            if (varargParams_.size())
            {
                peLib.Out() << ", ";
                for (std::deque<Param*>::const_iterator it = varargParams_.begin(); it != varargParams_.end();)
                {
                    (*it)->GetType()->ILSrcDump(peLib);
                    ++it;
                    if (it != varargParams_.end())
                        peLib.Out() << ", ";
                }
            }
        }
    }
    peLib.Out() << ")";
    return true;
}

void MethodSignature::ILSignatureDump(Stream& peLib)
{
    returnType_->ILSrcDump(peLib);
    peLib.Out() << " ";
    if (typeid(*container_) == typeid(Class) && static_cast<Class*>(container_)->Generic().size())
    {
        if (container_->Flags().Flags() & Qualifiers::Value)
            peLib.Out() << "valuetype ";
        else
            peLib.Out() << "class ";
        peLib.Out() << Qualifiers::GetName("", container_);
        peLib.Out() << static_cast<Class*>(container_)->AdornGenerics(peLib);
        peLib.Out() << "::'" << name_ << "'(";
    }
    else
    {
        peLib.Out() << Qualifiers::GetName(name_, container_);

    }
    peLib.Out() << "(";
    for (std::deque<Param*>::const_iterator it = params.begin(); it != params.end();)
    {
        if ((*it)->GetType()->GetBasicType() == Type::ClassRef)
        {
            if ((*it)->GetType()->GetClass()->Flags().Flags() & Qualifiers::Value)
                peLib.Out() << "valuetype ";
            else
                peLib.Out() << "class ";
        }
        (*it)->GetType()->ILSrcDump(peLib);
        ++it;
        if (it != params.end())
            peLib.Out() << ", ";
    }
    peLib.Out() << ")";
}
bool MethodSignature::PEDump(Stream& peLib, bool asType)
{
    if (container_ && container_->InAssemblyRef())
    {
        if (!peIndexCallSite_)
        {
            container_->PEDump(peLib);
            if (returnType_ && returnType_->GetBasicType() == Type::ClassRef)
            {
                if (returnType_->GetClass()->InAssemblyRef())
                {
                    returnType_->GetClass()->PEDump(peLib);
                }
            }

            for(Param* param : params)
            {
                if (param && param->GetType()->GetBasicType() == Type::ClassRef)
                {
                    // NOTE: original called PEDump unconditionally, which leads to
                    // rendering of the same class and its methods more than once!
                    // If this is not called at all, not all referenced classes are considered
                    // in the meta tables, e.g. Display.FrameMsg in System.Recall, which is only used as param type.
                    // This lead to nil tokens in typerefs and assemblies which are only read up to this token by Mono!
                    Class* cls = dynamic_cast<Class*>(param->GetType()->GetClass());
                    if( cls && cls->InAssemblyRef() && cls->PEIndex() == 0 )
                        cls->PEDump(peLib);
                }
            }

            size_t sz;
            if (generic_.size())
            {                
                genericParent_->PEDump(peLib, false);
                Byte* sig = SignatureGenerator::MethodSpecSig(this, sz);
                size_t methodSignature = peLib.PEOut().HashBlob(sig, sz);
                delete[] sig;
                MethodDefOrRef methodRef(MethodDefOrRef::MemberRef, genericParent_->PEIndexCallSite());
                TableEntryBase* table = new MethodSpecTableEntry(methodRef, methodSignature);
                peIndexCallSite_ = peLib.PEOut().AddTableEntry(table);
            }
            else
            {
                size_t function = peLib.PEOut().HashString(name_);
                Class* cls = nullptr;
                if (container_ && typeid(*container_) == typeid(Class))
                {
                    cls = static_cast<Class*>(container_);
                }
                Byte* sig = SignatureGenerator::MethodRefSig(this, sz);
                size_t methodSignature = peLib.PEOut().HashBlob(sig, sz);
                delete[] sig;
                MemberRefParent memberRef(cls && cls->Generic().size() ? MemberRefParent::TypeSpec : MemberRefParent::TypeRef, container_->PEIndex());
                TableEntryBase* table = new MemberRefTableEntry(memberRef, function, methodSignature);
                peIndexCallSite_ = peLib.PEOut().AddTableEntry(table);
            }
        }
    }
    else if (asType)
    {
        if (!peIndexType_)
        {
            size_t sz;
            Byte* sig = SignatureGenerator::MethodRefSig(this, sz);
            size_t methodSignature = peLib.PEOut().HashBlob(sig, sz);
            delete[] sig;
            TableEntryBase* table = new StandaloneSigTableEntry(methodSignature);
            peIndexType_ = peLib.PEOut().AddTableEntry(table);
        }
    }
    else if ((flags_ & Vararg) && !(flags_ & Managed))
    {
        size_t sz;
        size_t function = peLib.PEOut().HashString(name_);
        size_t parentIndex = methodParent_ ? methodParent_->PEIndex() : 0;
        Byte* sig = SignatureGenerator::MethodRefSig(this, sz);
        size_t methodSignature = peLib.PEOut().HashBlob(sig, sz);
        delete[] sig;
        peIndexCallSite_ = peLib.PEOut().AddTableEntry(
                    new MemberRefTableEntry(
                        MemberRefParent(MemberRefParent::MethodDef, parentIndex),
                        function, methodSignature) );
    }
    else if (!peIndexCallSite_)
    {
        int methodreftype = MemberRefParent::TypeRef;
        size_t sz;
        size_t function = peLib.PEOut().HashString(name_);
        size_t parent;
        if (returnType_ && returnType_->GetBasicType() == Type::ClassRef)
        {
            if (returnType_->GetClass()->InAssemblyRef())
            {
                returnType_->GetClass()->PEDump(peLib);
            }
        }
        if (arrayObject_)
        {
            methodreftype = MemberRefParent::TypeSpec;
            Byte buf[16];
            arrayObject_->Render(peLib, buf);
            parent = arrayObject_->PEIndex();
        }
        else if (container_)
        {
            if (!container_->PEIndex())
                container_->PEDump(peLib);
            parent = container_->PEIndex();
            if (typeid(*container_) == typeid(Class))
            {
                Class* cls = static_cast<Class*>(container_);
                if (cls->Generic().size() && cls->Generic().front()->GetBasicType() != Type::TypeVar)
                {
                    methodreftype = MemberRefParent::TypeSpec;
                }
            }
        }
        else
        {
            return false;
        }
        MemberRefParent memberRef(methodreftype, parent);
        Byte* sig = SignatureGenerator::MethodRefSig(this, sz);
        size_t methodSignature = peLib.PEOut().HashBlob(sig, sz);
        delete[] sig;
        TableEntryBase* table = new MemberRefTableEntry(memberRef, function, methodSignature);
        peIndexCallSite_ = peLib.PEOut().AddTableEntry(table);
    }
    return true;
}

std::string MethodSignature::AdornGenerics(Stream& peLib, bool names) const
{
    std::unique_ptr<std::iostream> hold( new std::stringstream() );
    peLib.Swap(hold);
    if (generic_.size())
    {
        int count = 0;
        peLib.Out() << "<";
        for (auto&& type : generic_)
        {
            if (names && type->GetBasicType() == Type::TypeVar)
            {
                peLib.Out() << (char)(type->VarNum() / 26 + 'A');
                peLib.Out() << (char)(type->VarNum() % 26 + 'A');
            }
            else
            {
                Type tp = *type;
                tp.ShowType();
                tp.ILSrcDump(peLib);
            }
            if (count++ != generic_.size() - 1)
                peLib.Out() << ",";
            else
                peLib.Out() << ">";
        }
    }
    peLib.Swap(hold);
    return static_cast<std::stringstream&>(*hold).str();
}
}  // namespace DotNetPELib
