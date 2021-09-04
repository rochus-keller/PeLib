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

#include "Type.h"
#include "Class.h"
#include "MethodSignature.h"
#include "Value.h"
#include "AssemblyDef.h"
#include "Field.h"
#include "Property.h"
#include "Method.h"
#include "PEFile.h"

// TODO: separate SignatureGenerator h/cpp

namespace DotNetPELib
{
int SignatureGenerator::workArea[400 * 1024];

int SignatureGenerator::basicTypes[] = {0,
                                        0,
                                        0,
                                        0,
                                        ELEMENT_TYPE_VOID,
                                        ELEMENT_TYPE_bool,
                                        ELEMENT_TYPE_CHAR,
                                        ELEMENT_TYPE_I1,
                                        ELEMENT_TYPE_U1,
                                        ELEMENT_TYPE_I2,
                                        ELEMENT_TYPE_U2,
                                        ELEMENT_TYPE_I4,
                                        ELEMENT_TYPE_U4,
                                        ELEMENT_TYPE_I8,
                                        ELEMENT_TYPE_U8,
                                        ELEMENT_TYPE_I,
                                        ELEMENT_TYPE_U,
                                        ELEMENT_TYPE_R4,
                                        ELEMENT_TYPE_R8,
                                        0,
                                        ELEMENT_TYPE_STRING};

int SignatureGenerator::objectBase;

size_t SignatureGenerator::EmbedType(int* buf, int offset, Type* tp)
{
    int rv = 0;
    if (tp->Pinned())
        buf[offset + rv++] = ELEMENT_TYPE_PINNED;

    if (tp->ByRef())
        buf[offset + rv++] = ELEMENT_TYPE_BYREF;

    for (int i = 0; i < tp->PointerLevel(); i++)
        buf[offset + rv++] = ELEMENT_TYPE_PTR;

    if (tp->ArrayLevel())
    {
        if (tp->ArrayLevel() == 1)
        {
            buf[offset + rv++] = ELEMENT_TYPE_SZARRAY;
        }
        else
        {
            buf[offset + rv++] = ELEMENT_TYPE_ARRAY;
        }
    }
    switch (tp->GetBasicType())
    {
        case Type::object:
            buf[offset + rv++] = ELEMENT_TYPE_OBJECT;
            break;
        case Type::MethodParam:
            buf[offset + rv++] = ELEMENT_TYPE_MVAR;
            buf[offset + rv++] = tp->VarNum();
            break;
        case Type::TypeVar:
            buf[offset + rv++] = ELEMENT_TYPE_VAR;
            buf[offset + rv++] = tp->VarNum();
            break;
        case Type::ClassRef:
        {
            Class* cls = static_cast<Class*>(tp->GetClass());
            Class* cls1 = cls;
            if (cls->Generic().size())
            {
                buf[offset + rv++] = ELEMENT_TYPE_GENERICINST;
                // the parent is expected to be the main class for the generic.
                cls1 = cls->GenericParent();
            }
            if (cls1->Flags().Flags() & Qualifiers::Value)
            {
                buf[offset + rv++] = ELEMENT_TYPE_VALUETYPE;
            }
            else
            {
                buf[offset + rv++] = ELEMENT_TYPE_CLASS;
            }
            if (cls1->InAssemblyRef())
            {
                buf[offset + rv++] = (cls1->PEIndex() << 2) | TypeDefOrRef::TypeRef;
            }
            else
            {
                buf[offset + rv++] = (cls1->PEIndex() << 2) | TypeDefOrRef::TypeDef;
            }
            if (cls->Generic().size())
            {
                buf[offset + rv++] = cls->Generic().size();
                for (auto type : cls->Generic())
                {
                    rv += EmbedType(buf, offset + rv, type);
                }
            }
            break;
        }
        case Type::MethodRef:
        {
            MethodSignature* sig = tp->GetMethod();
            buf[offset + rv++] = ELEMENT_TYPE_FNPTR;
            rv += CoreMethod(sig, sig->ParamCount() + sig->VarargParamCount(), buf, offset + rv);
            if (sig->VarargParamCount())
            {
                workArea[offset + rv++] = ELEMENT_TYPE_SENTINEL;
                for (MethodSignature::viterator it = sig->vbegin(); it != sig->vend(); ++it)
                {
                    rv += EmbedType(workArea, offset + rv, (*it)->GetType());
                }
            }
        }
        break;
        case Type::Void:
            if (tp->PEIndex())
            {
                buf[offset + rv++] = ELEMENT_TYPE_CLASS;
                buf[offset + rv++] = (tp->PEIndex() << 2) | TypeDefOrRef::TypeRef;
                break;
            }
            // fall through
        default:
            buf[offset + rv++] = basicTypes[tp->GetBasicType()];
            break;
    }
    if (tp->ArrayLevel() > 1)
    {
        buf[offset + rv++] = tp->ArrayLevel();  // rank
        buf[offset + rv++] = 0;                 // sizes = unsized
        buf[offset + rv++] = tp->ArrayLevel();  // lower bounds, set all to always zero for this
        for (int i = 0; i < tp->ArrayLevel(); i++)
            buf[offset + rv++] = 0;
    }
    return rv;
}
size_t SignatureGenerator::LoadIndex(Byte* buf, size_t& start, size_t& len)
{
    if (!(buf[start] & 0x80))
    {
        len--;
        return buf[start++];
    }
    else if (!(buf[start] & 0x40))
    {

        size_t rv = ((buf[start + 0] & 0x7f) << 8) + buf[start + 1];
        start += 2;
        len -= 2;
        return rv;
    }
    else
    {
        size_t rv = ((buf[start] & 0x3f) << 24) + (buf[start + 1] << 16) + (buf[start + 2] << 8) + buf[start + 3];
        start += 4;
        len -= 4;
        return rv;
    }
}

Type* SignatureGenerator::BasicType(PELib& lib, int typeIndex, int pointerLevel)
{
    Type::BasicType type = Type::Void;
    switch (typeIndex)
    {
        case ELEMENT_TYPE_END:
            return nullptr;
        case ELEMENT_TYPE_VOID:
            type = Type::Void;
            break;
        case ELEMENT_TYPE_bool:
            type = Type::Bool;
            break;
        case ELEMENT_TYPE_CHAR:
            type = Type::Char;
            break;
        case ELEMENT_TYPE_I1:
            type = Type::i8;
            break;
        case ELEMENT_TYPE_U1:
            type = Type::u8;
            break;
        case ELEMENT_TYPE_I2:
            type = Type::i16;
            break;
        case ELEMENT_TYPE_U2:
            type = Type::u16;
            break;
        case ELEMENT_TYPE_I4:
            type = Type::i32;
            break;
        case ELEMENT_TYPE_U4:
            type = Type::u32;
            break;
        case ELEMENT_TYPE_I8:
            type = Type::i64;
            break;
        case ELEMENT_TYPE_U8:
            type = Type::u64;
            break;
        case ELEMENT_TYPE_I:
            type = Type::inative;
            break;
        case ELEMENT_TYPE_U:
            type = Type::unative;
            break;
        case ELEMENT_TYPE_R4:
            type = Type::r32;
            break;
        case ELEMENT_TYPE_R8:
            type = Type::r64;
            break;
        case ELEMENT_TYPE_STRING:
            type = Type::string;
            break;
        case ELEMENT_TYPE_OBJECT:
            type = Type::object;
            break;
        case ELEMENT_TYPE_MVAR:
            type = Type::MethodParam;
            break;
        case ELEMENT_TYPE_VAR:
            type = Type::TypeVar;
            break;
        default:
            printf("Unknown type %x\n", typeIndex);
            break;
    }
    return new Type(type, pointerLevel);
}

Byte* SignatureGenerator::ConvertToBlob(int* buf, int size, size_t& sz)
{
    sz = 0;
    for (int i = 0; i < size; i++)
    {
        if (buf[i] > 0x3fff)
            sz += 4;
        else if (buf[i] > 0x7f)
            sz += 2;
        else
            sz += 1;
    }
    Byte* rv = new Byte[sz];
    int pos = 0;
    for (int i = 0; i < size; i++)
    {
        if (buf[i] > 0x3fff)
        {
            rv[pos++] = ((buf[i] >> 24) & 0x1f) | 0xc0;
            rv[pos++] = (buf[i] >> 16) & 0xff;
            rv[pos++] = (buf[i] >> 8) & 0xff;
            rv[pos++] = buf[i] & 0xff;
        }
        else if (buf[i] > 0x7f)
        {
            rv[pos++] = (buf[i] >> 8) | 0x80;
            rv[pos++] = buf[i] & 0xff;
        }
        else
        {
            rv[pos++] = buf[i];
        }
    }
    return rv;
}
size_t SignatureGenerator::CoreMethod(MethodSignature* method, int paramCount, int* buf, int offset)
{
    int origOffset = offset;
    int size = offset;
    int flag = 0;
    // for static members, flag will usually remain 0
    if (method->Flags() & MethodSignature::InstanceFlag)
        flag |= 0x20;
    if ((method->Flags() & MethodSignature::Vararg) && !(method->Flags() & MethodSignature::Managed))
        flag |= 5;
    if (method->GenericParamCount())
        flag |= 0x10;
    workArea[size++] = flag;
    if (method->GenericParamCount())
    {
        workArea[size++] = method->GenericParamCount();
    }
    workArea[size++] = paramCount;
    size += EmbedType(workArea, size, method->ReturnType());
    for (auto it = method->begin(); it != method->end(); ++it)
    {
        size += EmbedType(workArea, size, (*it)->GetType());
    }
    return size - origOffset;
}
Byte* SignatureGenerator::MethodDefSig(MethodSignature* method, size_t& sz)
{
    int size = 0;
    size = CoreMethod(method, method->ParamCount(), workArea, 0);
    return ConvertToBlob(workArea, size, sz);
}
Byte* SignatureGenerator::MethodRefSig(MethodSignature* method, size_t& sz)
{
    int size = 0;
    size = CoreMethod(method, method->ParamCount() + method->VarargParamCount(), workArea, 0);
    // variable length args... this is the difference from the methoddef
    if ((method->Flags() & MethodSignature::Vararg) && !(method->Flags() & MethodSignature::Managed))
    {
        if (method->VarargParamCount())
        {
            workArea[size++] = ELEMENT_TYPE_SENTINEL;
            for (MethodSignature::viterator it = method->vbegin(); it != method->vend(); ++it)
            {
                size += EmbedType(workArea, size, (*it)->GetType());
            }
        }
    }
    return ConvertToBlob(workArea, size, sz);
}
Byte *SignatureGenerator::MethodSpecSig(MethodSignature *signature, size_t &sz)
{
    int size = 0;
    workArea[size++] = 0x0a; // generic
    workArea[size++] = signature->Generic().size();
    for (auto g : signature->Generic())
    {
       size += EmbedType(workArea, size, g);
    }
    return ConvertToBlob(workArea, size, sz);
}

Byte* SignatureGenerator::FieldSig(Field* field, size_t& sz)
{
    int size = 0;
    workArea[size++] = 6;  // field sig
    // here we would put the
    size += EmbedType(workArea, size, field->FieldType());
    return ConvertToBlob(workArea, size, sz);
}
Byte* SignatureGenerator::PropertySig(Property* property, size_t& sz)
{
    int size = 0;
    // a property sig is a modification of the methoddef of the getter
    workArea[size++] = 8;

    size = CoreMethod(property->Getter()->Signature(), property->Getter()->Signature()->ParamCount(), workArea, 0);
    // set the instance flag based on whether the property is a static or nonstatic member
    if (property->Instance())
    {
        workArea[1] |= 0x20;
    }
    else
    {
        workArea[1] &= ~0x20;
    }
    return ConvertToBlob(workArea, size, sz);
}
Byte* SignatureGenerator::LocalVarSig(Method* method, size_t& sz)
{
    int size = 0;
    workArea[size++] = 7;  // locals sig
    workArea[size++] = method->size();
    for (auto it = method->begin(); it != method->end(); ++it)
    {
        size += EmbedType(workArea, size, (*it)->GetType());
    }
    return ConvertToBlob(workArea, size, sz);
}
Byte* SignatureGenerator::TypeSig(Type* type, size_t& sz)
{
    int size = 0;
    size += EmbedType(workArea, size, type);
    return ConvertToBlob(workArea, size, sz);
}




}  // namespace DotNetPELib
