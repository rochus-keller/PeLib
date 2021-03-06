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

#include "Class.h"
#include "Callback.h"
#include "PEWriter.h"
#include "Stream.h"
#include "Property.h"
#include "Type.h"
#include "PELibError.h"
#include "Field.h"
#include "MethodSignature.h"
#include "Method.h"
#include <typeinfo>
#include <climits>
#include <sstream>
#include "SignatureGenerator.h"

namespace DotNetPELib
{

bool Class::ILSrcDump(Stream& peLib) const
{
    ILSrcDumpClassHeader(peLib);
    if (extendsFrom_)
    {
        peLib.Out() << " extends " << Qualifiers::GetName("", extendsFrom_);
        peLib.Out() << extendsFrom_->AdornGenerics(peLib);
    }
    peLib.Out() << " {";
    if (pack_ > 0 || size_ > 0)
    {
        peLib.Out() << std::endl;
        if (pack_ > 0)
            peLib.Out() << " .pack " << pack_;
        if (size_ >= 0)
            peLib.Out() << " .size " << size_;
    }
    peLib.Out() << std::endl;
    bool rv = DataContainer::ILSrcDump(peLib);
    peLib.Out() << std::endl;
    for (auto p : properties_)
        rv &= p->ILSrcDump(peLib);
    peLib.Out() << "}" << std::endl;
    return rv;
}
int Class::TransferFlags() const
{
    int peflags = TypeDefTableEntry::Class;
    DataContainer* parent = Parent();
    if (parent && typeid(*parent) == typeid(Class))
    {
        if (flags_.Flags() & Qualifiers::Public)
        {
            peflags |= TypeDefTableEntry::NestedPublic;
        }
        else
        {
            peflags |= TypeDefTableEntry::NestedPrivate;
        }
    }
    else
    {
        if (flags_.Flags() & Qualifiers::Public)
        {
            peflags |= TypeDefTableEntry::Public;
        }
    }
    if (flags_.Flags() & Qualifiers::Sequential)
    {
        peflags |= TypeDefTableEntry::SequentialLayout;
    }
    else if (flags_.Flags() & Qualifiers::Explicit)
    {
        peflags |= TypeDefTableEntry::ExplicitLayout;
    }
    if (flags_.Flags() & Qualifiers::Sealed)
    {
        peflags |= TypeDefTableEntry::Sealed;
    }
    if (flags_.Flags() & Qualifiers::Ansi)
    {
        peflags |= TypeDefTableEntry::AnsiClass;
    }
    return peflags;
}
bool Class::PEDump(Stream& peLib)
{
    if (generic_.size() && generic_.front()->GetBasicType() != Type::TypeVar)
    {
        if (!peIndex_)
        {
            if (!genericParent_->PEIndex())
                genericParent_->PEDump(peLib);
            if (generic_.size())
            {
                for (auto g : generic_)
                {
                    if (g->GetBasicType() == Type::ClassRef && g->PEIndex() == 0)
                    {
                        size_t val;
                        g->Render(peLib, (Byte*)&val);
                        g->PEIndex(val);
                    }
                }
            }
            size_t sz;
            Type type(this);
            Byte* sig = SignatureGenerator::TypeSig(&type, sz);
            size_t signature = peLib.PEOut().HashBlob(sig, sz);
            delete[] sig;
            TypeSpecTableEntry* table = new TypeSpecTableEntry(signature);
            peIndex_ = peLib.PEOut().AddTableEntry(table);
        }
    }
    else if (InAssemblyRef())
    {
        if (!peIndex_)
        {
            if( dynamic_cast<Class*>(parent_) )
            {
                // this is a nested class
                parent_->PEDump(peLib);
                ResolutionScope resolution(ResolutionScope::TypeRef, parent_->PEIndex());
                size_t typenameIndex = peLib.PEOut().HashString(Name());
                TableEntryBase* table = new TypeRefTableEntry(resolution, typenameIndex, 0);
                peIndex_ = peLib.PEOut().AddTableEntry(table);
            }
            else
            {
                // this is a top-level class in an assembly
                ResolutionScope resolution(ResolutionScope::AssemblyRef, ParentAssembly(peLib));
                size_t typenameIndex = peLib.PEOut().HashString(Name());
                size_t namespaceIndex = ParentNamespace(peLib);
                TableEntryBase* table = new TypeRefTableEntry(resolution, typenameIndex, namespaceIndex);
                peIndex_ = peLib.PEOut().AddTableEntry(table);
            }
        }
    }
    else
    {
        int peflags = TransferFlags();
        size_t typenameIndex = peLib.PEOut().HashString(Name());
        size_t namespaceIndex = ParentNamespace(peLib);
        size_t extends = (flags_.Flags() & Qualifiers::Value) ? peLib.PEOut().ValueBaseClass() : peLib.PEOut().ObjectBaseClass();
        size_t fieldIndex = peLib.PEOut().NextTableIndex(tField);
        size_t methodIndex = peLib.PEOut().NextTableIndex(tMethodDef);
        DataContainer* parent = Parent();
        if (extendsFrom_)
        {
            if (!extendsFrom_->PEIndex())
                extendsFrom_->PEDump(peLib);
            extends = extendsFrom_->PEIndex();
        }
        if (parent && typeid(*parent) == typeid(Class))
            namespaceIndex = 0;
        int typeType = TypeDefOrRef::TypeRef;
        if (extendsFrom_ && !extendsFrom_->InAssemblyRef())
            typeType = TypeDefOrRef::TypeDef;
        TypeDefOrRef extendsClass(typeType, extends);
        TableEntryBase* table =
            new TypeDefTableEntry(peflags, typenameIndex, namespaceIndex, extendsClass, fieldIndex, methodIndex);
        peIndex_ = peLib.PEOut().AddTableEntry(table);

        if (pack_ > 0 || size_ > 0)
        {
            int mypack_ = pack_;
            int mysize_ = size_;
            if (mypack_ <= 0)
                mypack_ = 1;
            if (mysize_ <= 0)
                mysize_ = 1;
            table = new ClassLayoutTableEntry(mypack_, mysize_, peIndex_);
            peLib.PEOut().AddTableEntry(table);
        }
        if (parent && typeid(*parent) == typeid(Class))
        {
            size_t enclosing = ParentClass(peLib);
            table = new NestedClassTableEntry(peIndex_, enclosing);
            peLib.PEOut().AddTableEntry(table);
        }
        DataContainer::PEDump(peLib);
        if (properties_.size())
        {
            size_t propertyIndex = peLib.PEOut().NextTableIndex(tProperty);
            table = new PropertyMapTableEntry(peIndex_, propertyIndex);
            peLib.PEOut().AddTableEntry(table);
            for (auto p : properties_)
                p->PEDump(peLib);
        }
        if (generic_.size())
        {
            TypeOrMethodDef owner(TypeOrMethodDef::TypeDef, peIndex_);
            for (int i = 0; i < generic_.size(); i++)
            {
                std::string name;
                name += (char)(i / 26 + 'A');
                name += (char)(i % 26 + 'A');
                size_t namestr = peLib.PEOut().HashString(name);
                table = new GenericParamTableEntry(i, 0, owner, namestr);
                peLib.PEOut().AddTableEntry(table);
            }
        }
    }
    return true;
}
void Class::ILSrcDumpClassHeader(Stream& peLib) const
{
    peLib.Out() << ".class";
    DataContainer* parent = Parent();
    if (parent && typeid(*parent) == typeid(Class))
        peLib.Out() << " nested";
    flags_.ILSrcDumpBeforeFlags(peLib);
    flags_.ILSrcDumpAfterFlags(peLib);
    peLib.Out() << " '" << name_ << "'";
    peLib.Out() << AdornGenerics(peLib, true);
}

bool Class::Traverse(Callback& callback) const
{
    if (!DataContainer::Traverse(callback))
        return true;
    for (auto property : properties_)
        if (!callback.EnterProperty(property))
            return false;
    return true;
}
std::string Class::AdornGenerics(Stream& peLib, bool names) const
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
            if (count++ != generic_.size()-1)
                peLib.Out() << ",";
            else
                peLib.Out() << ">";
        }
    }
    peLib.Swap(hold);
    return static_cast<std::stringstream&>(*hold).str();
}
bool Class::MatchesGeneric(std::deque<Type*>* generics) const
{
    if (generics)
    {
        if (Generic().size() == generics->size())
        {
            auto itg1 = Generic().begin();
            auto itg2 = generics->begin();
            while (itg2 != generics->end())
            {
                if (!(*itg1)->Matches(*itg2))
                    break;
                ++itg1;
                ++itg2;
            }
            if (itg2 == generics->end())
            {
                return true;
            }
        }
    }
    return false;
}

void DotNetPELib::Class::Add(DotNetPELib::Property* property, bool add)
{
    if (property)
    {
        property->SetContainer(this, add);
        properties_.push_back(property);
    }
}

}  // namespace DotNetPELib
