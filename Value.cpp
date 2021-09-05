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

#include "Value.h"
#include "PEWriter.h"
#include "Type.h"
#include "PELibError.h"
#include "Stream.h"
#include "Instruction.h"
#include "Field.h"
#include "DataContainer.h"
#include "MethodSignature.h"
namespace DotNetPELib
{
bool Value::ILSrcDump(Stream& peLib) const
{
    // used for types
    type_->ILSrcDump(peLib);
    return true;
}

size_t Value::Render(Stream& peLib, int opcode, int operandType, Byte* result) { return type_->Render(peLib, result); }

bool Local::ILSrcDump(Stream& peLib) const
{
    peLib.Out() << "'" << name_ << "/" << index_ << "'";
    return true;
}

size_t Local::Render(Stream& peLib, int opcode, int operandType, Byte* result)
{
    int sz = 0;
    if (operandType == Instruction::o_index1)
    {
        *(Byte*)result = index_;
        sz = 1;
    }
    else if (operandType == Instruction::o_index2)
    {
        *(unsigned short*)result = index_;
        sz = 2;
    }
    return sz;
}

bool Param::ILSrcDump(Stream& peLib) const
{
    peLib.Out() << "'" << name_ << "'";
    return true;
}

size_t Param::Render(Stream& peLib, int opcode, int operandType, Byte* result)
{
    int sz = 0;
    if (operandType == Instruction::o_index1)
    {
        *(Byte*)result = index_;
        sz = 1;
    }
    else if (operandType == Instruction::o_index2)
    {
        *(unsigned short*)result = index_;
        sz = 2;
    }
    return sz;
}
bool FieldName::ILSrcDump(Stream& peLib) const
{
    if (field_->FieldType()->GetBasicType() == Type::ClassRef)
    {
        if (field_->FieldType()->GetClass()->Flags().Flags() & Qualifiers::Value)
            peLib.Out() << "valuetype ";
        else
            peLib.Out() << "class ";
    }
    field_->FieldType()->ILSrcDump(peLib);
    peLib.Out() << " ";
    peLib.Out() << Qualifiers::GetName(field_->Name(), field_->GetContainer());
    return true;
}

size_t FieldName::Render(Stream& peLib, int opcode, int operandType, Byte* result)
{
    if (field_->GetContainer() && field_->GetContainer()->InAssemblyRef())
    {
        field_->PEDump(peLib);
        *(DWord*)result = field_->PEIndex() | (tMemberRef << 24);
    }
    else
    {
        *(DWord*)result = field_->PEIndex() | (tField << 24);
    }
    return 4;
}
MethodName::MethodName(MethodSignature* M) : signature_(M), Value("", nullptr) {}
bool MethodName::ILSrcDump(Stream& peLib) const
{
    signature_->ILSrcDump(peLib, false, false, false);
    return true;
}

size_t MethodName::Render(Stream& peLib, int opcode, int operandType, Byte* result)
{
    if (opcode == Instruction::i_calli)
    {
        if (signature_->PEIndexType() == 0)
            signature_->PEDump(peLib, true);
        *(DWord*)result = signature_->PEIndexType() | (tStandaloneSig << 24);
    }
    else
    {
        if (signature_->PEIndex() == 0 && signature_->PEIndexCallSite() == 0)
            signature_->PEDump(peLib, false);
        if (signature_->PEIndex())
            *(DWord*)result = signature_->PEIndex() | (tMethodDef << 24);
        else if (signature_->Generic().size())
            *(DWord*)result = signature_->PEIndexCallSite() | (tMethodSpec << 24);
        else
            *(DWord*)result = signature_->PEIndexCallSite() | (tMemberRef << 24);
    }
    return 4;
}
}  // namespace DotNetPELib
