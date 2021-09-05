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

#include "Field.h"
#include "Stream.h"
#include "PEWriter.h"
#include "DataContainer.h"
#include "Type.h"
#include "PELibError.h"
#include <iomanip>
#include "SignatureGenerator.h"

namespace DotNetPELib
{
void Field::AddEnumValue(longlong Value, ValueSize Size)
{
    if (mode_ == None)
    {
        mode_ = Enum;
        enumValue_ = Value;
        size_ = Size;
    }
}
void Field::AddInitializer(Byte* bytes, int len)
{
    if (mode_ == None)
    {
        mode_ = Bytes;
        byteValue_ = bytes;
        byteLength_ = len;
    }
}

bool Field::InAssemblyRef() const
{
    return parent_->InAssemblyRef();
}
bool Field::ILSrcDumpTypeName(Stream& peLib, Field::ValueSize size)
{
    switch (size)
    {
        case Field::i8:
            peLib.Out() << " int8";
            break;
        case Field::i16:
            peLib.Out() << " int16";
            break;
        case Field::i32:
        default:
            peLib.Out() << " int32";
            break;
        case Field::i64:
            peLib.Out() << " int64";
            break;
    }
    return true;
}
bool Field::ILSrcDump(Stream& peLib) const
{
    peLib.Out() << ".field";
    if ((parent_->Flags().Flags() & Qualifiers::Explicit) ||
        ((parent_->Flags().Flags() & Qualifiers::Sequential) && explicitOffset_))
        peLib.Out() << " [" << explicitOffset_ << "]";
    flags_.ILSrcDumpBeforeFlags(peLib);
    flags_.ILSrcDumpAfterFlags(peLib);
    if (FieldType()->GetBasicType() == Type::ClassRef)
    {
        if (FieldType()->GetClass()->Flags().Flags() & Qualifiers::Value)
        {
            peLib.Out() << " valuetype ";
            type_->ILSrcDump(peLib);
        }
        else
        {
            peLib.Out() << " class ";
            type_->ILSrcDump(peLib);
        }
    }
    else
    {
        peLib.Out() << " ";
        type_->ILSrcDump(peLib);
    }
    peLib.Out() << " '" << name_ << "'";
    switch (mode_)
    {
        case None:
            break;
        case Enum:
            peLib.Out() << " = ";
            ILSrcDumpTypeName(peLib, size_);
            peLib.Out() << "(" << (int)enumValue_ << ")";

            break;
        case Bytes:
            if (byteValue_ && byteLength_)
            {
                peLib.Out() << " at $" << name_ << std::endl;
                peLib.Out() << ".data cil $" << name_ << " = bytearray (" << std::endl << std::hex;
                int i;
                for (i = 0; i < byteLength_; i++)
                {
                    peLib.Out() << std::setw(2) << std::setfill('0') << (int)byteValue_[i] << " ";
                    if (i % 8 == 7 && i != byteLength_ - 1)
                        peLib.Out() << std::endl << "\t";
                }
                peLib.Out() << ")" << std::dec;
            }
            break;
    }
    peLib.Out() << std::endl;
    return true;
}

bool Field::PEDump(Stream& peLib)
{
    size_t sz;
    if (type_->GetBasicType() == Type::ClassRef)
    {
        if (type_->GetClass()->InAssemblyRef())
            type_->GetClass()->PEDump(peLib);
    }
    Byte* sig = SignatureGenerator::FieldSig(this, sz);
    size_t sigindex = peLib.PEOut().HashBlob(sig, sz);
    size_t nameindex = peIndex_ = peLib.PEOut().HashString(Name());
    if (InAssemblyRef())
    {
        parent_->PEDump(peLib);
        MemberRefParent refParent(MemberRefParent::TypeRef, parent_->PEIndex());
        TableEntryBase* table = new MemberRefTableEntry(refParent, nameindex, sigindex);
        peIndex_ = peLib.PEOut().AddTableEntry(table);
    }
    else
    {
        int peflags = 0;
        if (flags_.Flags() & Qualifiers::Public)
            peflags |= FieldTableEntry::Public;
        else if (flags_.Flags() & Qualifiers::Private)
            peflags |= FieldTableEntry::Private;

        if (flags_.Flags() & Qualifiers::Static)
            peflags |= FieldTableEntry::Static;
        if (flags_.Flags() & Qualifiers::Literal)
            peflags |= FieldTableEntry::Literal;
        switch (mode_)
        {
            case Enum:
                peflags |= FieldTableEntry::HasDefault;  // in the blob;
                break;
            case Bytes:
                if (byteValue_ && byteLength_)
                {
                    peflags |= FieldTableEntry::HasFieldRVA;  // in separate memory
                }
                break;
            case None:
                // should never get here
                break;
        }
        TableEntryBase* table = new FieldTableEntry(peflags, nameindex, sigindex);
        peIndex_ = peLib.PEOut().AddTableEntry(table);

        if ((parent_->Flags().Flags() & Qualifiers::Explicit) ||
            ((parent_->Flags().Flags() & Qualifiers::Sequential) && explicitOffset_))
        {
            TableEntryBase* table = new FieldLayoutTableEntry(explicitOffset_, peIndex_);
            peLib.PEOut().AddTableEntry(table);
        }
        Byte buf[8];
        *(longlong*)(buf) = enumValue_;
        int type;
        switch (mode_)
        {
            case None:
                // should never get here
                break;
            case Enum:
            {
                switch (size_)
                {
                    case Field::i8:
                        sz = 1;
                        type = ELEMENT_TYPE_I1;
                        break;
                    case Field::i16:
                        sz = 2;
                        type = ELEMENT_TYPE_I2;
                        break;
                    case Field::i32:
                    default:
                        sz = 4;
                        type = ELEMENT_TYPE_I4;
                        break;
                    case Field::i64:
                        sz = 8;
                        type = ELEMENT_TYPE_I8;
                        break;
                }
                // this is NOT compressed like the sigs are...
                size_t valueIndex = peLib.PEOut().HashBlob(&buf[0], sz);
                Constant constant(Constant::FieldDef, peIndex_);
                table = new ConstantTableEntry(type, constant, valueIndex);
                peLib.PEOut().AddTableEntry(table);
                if (byteValue_ && byteLength_)
                {
                    size_t valueIndex = peLib.PEOut().RVABytes(byteValue_, byteLength_);
                    table = new FieldRVATableEntry(valueIndex, peIndex_);
                    peLib.PEOut().AddTableEntry(table);
                }
            }
            break;
            case Bytes:
                if (byteValue_ && byteLength_)
                {
                    size_t valueIndex = peLib.PEOut().RVABytes(byteValue_, byteLength_);
                    table = new FieldRVATableEntry(valueIndex, peIndex_);
                    peLib.PEOut().AddTableEntry(table);
                }
                break;
        }
    }
    delete[] sig;
    return true;
}
}  // namespace DotNetPELib
