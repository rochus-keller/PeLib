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

#include "Enum.h"
#include "Stream.h"
#include "PEWriter.h"
#include "Type.h"
#include "PELibError.h"
#include "SignatureGenerator.h"

namespace DotNetPELib
{
Field* Enum::AddValue(const std::string& Name, longlong Value)
{
    Type* type = new Type(this);
    Field* field = new Field(Name, type, Qualifiers(Qualifiers::EnumField));
    field->AddEnumValue(Value, size);
    Add(field);
    return field;
}
bool Enum::ILSrcDump(Stream& peLib) const
{
    ILSrcDumpClassHeader(peLib);
    peLib.Out() << " {" << std::endl;
    DataContainer::ILSrcDump(peLib);
    peLib.Out() << " .field public specialname rtspecialname ";
    Field::ILSrcDumpTypeName(peLib, size);
    peLib.Out() << " value__" << std::endl;
    peLib.Out() << "}" << std::endl;
    return true;
}

bool Enum::PEDump(Stream& peLib)
{
    if (!InAssemblyRef())
    {
        int peflags = TransferFlags();
        size_t typenameIndex = peLib.PEOut().HashString(Name());
        size_t namespaceIndex = ParentNamespace(peLib);
        size_t extends = peLib.PEOut().EnumBaseClass();
        size_t fieldIndex = peLib.PEOut().NextTableIndex(tField);
        size_t methodIndex = peLib.PEOut().NextTableIndex(tMethodDef);
        TypeDefOrRef extendsClass(TypeDefOrRef::TypeRef, extends);
        DataContainer* parent = Parent();
        if (parent && typeid(*parent) == typeid(Class))
            namespaceIndex = 0;
        TableEntryBase* table =
            new TypeDefTableEntry(peflags, typenameIndex, namespaceIndex, extendsClass, fieldIndex, methodIndex);
        peIndex_ = peLib.PEOut().AddTableEntry(table);

        if (parent && typeid(*parent) == typeid(Class))
        {
            size_t enclosing = ParentClass(peLib);
            table = new NestedClassTableEntry(peIndex_, enclosing);
            peLib.PEOut().AddTableEntry(table);
        }
        DataContainer::PEDump(peLib);  // should only be the enumerations
        size_t sz;
        Type::BasicType tsize;
        switch (size)
        {
            case Field::i8:
                tsize = Type::i8;
                break;
            case Field::i16:
                tsize = Type::i16;
                break;
            case Field::i32:
            default:
                tsize = Type::i32;
                break;
            case Field::i64:
                tsize = Type::i64;
                break;
        }
        // add the value member
        Type type(tsize, 0);
        Field field("value__", &type, Qualifiers(0));
        Byte* sig = SignatureGenerator::FieldSig(&field, sz);
        size_t sigindex = peLib.PEOut().HashBlob(sig, sz);
        size_t nameindex = peLib.PEOut().HashString(field.Name());
        table = new FieldTableEntry(FieldTableEntry::Public | FieldTableEntry::SpecialName | FieldTableEntry::RTSpecialName,
                                    nameindex, sigindex);
        peIndex_ = peLib.PEOut().AddTableEntry(table);
        delete[] sig;
    }
    else if (!peIndex_)
    {
        if (typeid(*parent_) == typeid(Class))
        {
            parent_->PEDump(peLib);
            ResolutionScope resolution(ResolutionScope::TypeRef, parent_->PEIndex());
            size_t typenameIndex = peLib.PEOut().HashString(Name());
            TableEntryBase* table = new TypeRefTableEntry(resolution, typenameIndex, 0);
            peIndex_ = peLib.PEOut().AddTableEntry(table);
        }
        else
        {
            ResolutionScope resolution(ResolutionScope::AssemblyRef, ParentAssembly(peLib));
            size_t typenameIndex = peLib.PEOut().HashString(Name());
            size_t namespaceIndex = ParentNamespace(peLib);
            TableEntryBase* table = new TypeRefTableEntry(resolution, typenameIndex, namespaceIndex);
            peIndex_ = peLib.PEOut().AddTableEntry(table);
        }
    }
    return true;
}
}  // namespace DotNetPELib
