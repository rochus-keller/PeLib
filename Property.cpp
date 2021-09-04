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

#include "Property.h"
#include "Method.h"
#include "DataContainer.h"
#include "MethodSignature.h"
#include "Value.h"
#include "Type.h"
#include "Instruction.h"
#include "Operand.h"
#include "PELibError.h"
#include "PEFile.h"
#include <typeinfo>
#include <sstream>

namespace DotNetPELib
{
void Property::Instance(bool instance)
{
    instance_ = instance;
    if (getter_)
        getter_->Instance(instance);
    if (setter_)
        setter_->Instance(instance);
}
Property::Property(PELib& peLib, const std::string& name, Type* type,
                   std::vector<Type*>& indices, bool hasSetter, DataContainer* parent)
    : name_(name), parent_(parent), type_(type), flags_(SpecialName), instance_(true), getter_(nullptr), setter_(nullptr)
{
    CreateFunctions(peLib, indices, hasSetter);
}

void Property::SetContainer(DataContainer* parent, bool add)
{
    if (!parent_)
    {
        parent_ = parent;
        if (add)
        {
            parent_->Add(getter_);
            getter_->Signature()->SetContainer(parent);
            if (setter_)
            {
                parent_->Add(setter_);
                setter_->Signature()->SetContainer(parent);
            }
        }
    }
}
void Property::CreateFunctions(PELib& peLib, std::vector<Type*>& indices, bool hasSetter)
{
    bool found = false;
    MethodSignature* prototype;
    std::string getter_name = "get_" + name_;
    if (parent_)
        for (auto m : parent_->Methods())
        {
            if (static_cast<Method*>(m)->Signature()->Name() == getter_name)
            {
                found = true;
                getter_ = static_cast<Method*>(m);
                break;
            }
        }
    if (!getter_)
    {
        prototype = new MethodSignature(getter_name, MethodSignature::Managed, parent_);
        getter_ = new Method(prototype, Qualifiers::Public | (instance_ ? Qualifiers::Instance : Qualifiers::Static));
    }
    if (hasSetter)
    {
        std::string setter_name = "set_" + name_;
        if (parent_)
            for (auto m : parent_->Methods())
            {
                if (static_cast<Method*>(m)->Signature()->Name() == setter_name)
                {
                    found = true;
                    setter_ = static_cast<Method*>(m);
                    break;
                }
            }
        if (!setter_)
        {
            prototype = new MethodSignature(setter_name, MethodSignature::Managed, parent_);
            setter_ = new Method(prototype, Qualifiers::Public | (instance_ ? Qualifiers::Instance : Qualifiers::Static));
        }
    }
    if (!found)
    {
        int counter = 1;
        for (auto i : indices)
        {
            std::stringstream stream;
            stream << "P" << counter++;
            char cbuf[256];
            stream.getline(cbuf, sizeof(cbuf));
            getter_->Signature()->AddParam(new Param(cbuf, i));
            if (setter_)
                setter_->Signature()->AddParam(new Param(cbuf, i));
        }
        getter_->Signature()->ReturnType(type_);
        if (setter_)
        {
            setter_->Signature()->AddParam(new Param("Value", type_));
            setter_->Signature()->ReturnType(new Type(Type::Void, 0));
        }
    }
}
void Property::CallGet(PELib& peLib, CodeContainer* code)
{
    if (getter_)
    {
        code->AddInstruction(
            new Instruction(Instruction::i_call, new Operand(new MethodName(getter_->Signature()))));
    }
}
void Property::CallSet(PELib& peLib, CodeContainer* code)
{
    if (setter_)
    {
        code->AddInstruction(
            new Instruction(Instruction::i_call, new Operand(new MethodName(setter_->Signature()))));
    }
}
bool Property::ILSrcDump(PELib& peLib) const
{
    peLib.Out() << ".property ";
    if (flags_ & SpecialName)
        peLib.Out() << "specialname ";
    if (instance_)
        peLib.Out() << "instance ";
    type_->ILSrcDump(peLib);
    peLib.Out() << " " << name_ << "() {" << std::endl;
    peLib.Out() << ".get ";
    getter_->Signature()->ILSignatureDump(peLib);
    peLib.Out() << std::endl;
    if (setter_)
    {
        peLib.Out() << ".set ";
        setter_->Signature()->ILSignatureDump(peLib);
        peLib.Out() << std::endl;
    }
    peLib.Out() << "}";
    return true;
}
void Property::ObjOut(PELib& peLib, int pass) const
{
    peLib.Out() << std::endl << "$Pb" << peLib.FormatName(name_) << instance_ << ",";
    peLib.Out() << flags_ << ",";
    type_->ObjOut(peLib, pass);
    getter_->ObjOut(peLib, -1);
    if (setter_)
    {
        peLib.Out() << std::endl << "$Sb";
        setter_->ObjOut(peLib, -1);
        peLib.Out() << std::endl << "$Se";
    }
    peLib.Out() << std::endl << "$Pe";
}
Property* Property::ObjIn(PELib& peLib)
{
    std::string name = peLib.UnformatName();
    int instance = peLib.ObjInt();
    char ch;
    ch = peLib.ObjChar();
    if (ch != ',')
        peLib.ObjError(oe_syntax);
    int flags = peLib.ObjInt();
    ch = peLib.ObjChar();
    if (ch != ',')
        peLib.ObjError(oe_syntax);
    Property* rv = new Property();
    Type* type = Type::ObjIn(peLib);
    if (peLib.ObjBegin() != 'm')
        peLib.ObjError(oe_syntax);
    Method* getter = nullptr;
    Method::ObjIn(peLib, false, &getter);
    Method* setter = nullptr;
    if (peLib.ObjBegin() == 'S')
    {
        if (peLib.ObjBegin() != 'm')
            peLib.ObjError(oe_syntax);
        Method::ObjIn(peLib, false, &setter);
        if (peLib.ObjEnd() != 'S')
            peLib.ObjError(oe_syntax);
        if (peLib.ObjEnd() != 'P')
            peLib.ObjError(oe_syntax);
    }
    else if (peLib.ObjEnd(false) != 'P')
    {
        peLib.ObjError(oe_syntax);
    }
    rv->Name(name);
    rv->flags_ = flags;
    rv->SetType(type);
    rv->Instance(instance);
    rv->Getter(getter);
    rv->Setter(setter);
    return rv;
}
bool Property::PEDump(PELib& peLib)
{
    size_t propertyIndex = peLib.PEOut().NextTableIndex(tProperty);
    size_t nameIndex = peLib.PEOut().HashString(name_);
    size_t sz;
    Byte* sig = SignatureGenerator::PropertySig(this, sz);
    size_t propertySignature = peLib.PEOut().HashBlob(sig, sz);
    delete[] sig;
    TableEntryBase* table = new PropertyTableEntry(flags_, nameIndex, propertySignature);
    peLib.PEOut().AddTableEntry(table);

    Semantics semantics = Semantics(Semantics::Property, propertyIndex);
    // FIXME : Coverity complains that the following 'new' statements leak memory, however, I think
    // the design is that the related constructors have side effects and the whole point of the new is to invoke those
    // however, this is an awkard design that is hard to maintain and should probably be reworked.

    table = new MethodSemanticsTableEntry(MethodSemanticsTableEntry::Getter, getter_->Signature()->PEIndex(), semantics);
    peLib.PEOut().AddTableEntry(table);
    if (setter_)
    {
        table = new MethodSemanticsTableEntry(MethodSemanticsTableEntry::Setter, setter_->Signature()->PEIndex(), semantics);
        peLib.PEOut().AddTableEntry(table);
    }
    return true;
}

}  // namespace DotNetPELib
