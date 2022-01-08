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
#include "PEWriter.h"
#include "DataContainer.h"
#include "Class.h"
#include "MethodSignature.h"
#include "Enum.h"
#include "PELibError.h"
#include "Stream.h"
#include <stdio.h>
#include "SignatureGenerator.h"

namespace DotNetPELib
{

// corresponds to BasicType: ClassRef, MethodRef, TypeVar, MethodParam, Void, Bool, ...
const char* Type::typeNames_[] = {"",        "",        "", "", "void",   "bool",       "char",
                                  "int8",    "uint8",   "int16",  "uint16",     "int32",
                                  "uint32",  "int64",   "uint64", "native int", "native unsigned int",
                                  "float32", "float64", "object", "string"};
const char* BoxedType::typeNames_[] = {"",        "",       "",       "", "", "Bool",   "Char",  "SByte",  "Byte",
                                       "Int16",   "UInt16", "Int32",  "UInt32", "Int64", "UInt64", "IntPtr",
                                       "UIntPtr", "Single", "Double", "Object", "String"};

Type::Type(Type::BasicType Tp, int PointerLevel) : tp_(Tp), arrayLevel_(0), byRef_(false), typeRef_(nullptr),
    methodRef_(nullptr), peIndex_(0), pinned_(false), showType_(false), varnum_(0),modopt_(0)
{
    if (Tp == TypeVar || Tp == MethodParam)
        varnum_ = PointerLevel;
    else
        pointerLevel_ = PointerLevel;
}

bool Type::Matches(Type* right)
{
    if (tp_ != right->tp_)
        return false;
    if (arrayLevel_ != right->arrayLevel_)
        return false;
    if (pointerLevel_ != right->pointerLevel_)
        return false;
    if (byRef_ != right->byRef_)
        return false;
    if (tp_ == ClassRef && typeRef_ != right->typeRef_)
    {
        int n1, n2;
        n1 = typeRef_->Name().find("_empty");
        n2 = right->typeRef_->Name().find("_empty");
        if (n1 != std::string::npos || n2 != std::string::npos)
        {
            bool transfer = false;
            if (n1 == std::string::npos)
            {
                n1 = typeRef_->Name().find("_array_");
            }
            else
            {
                transfer = true;
                n2 = right->typeRef_->Name().find("_array_");
            }
            if (n1 != n2)
                return false;
            if (typeRef_->Name().substr(0, n1) != right->typeRef_->Name().substr(0, n2))
                return false;
            if (transfer)
                typeRef_ = right->typeRef_;
        }
        else
            return false;
    }
    if (tp_ == MethodRef && methodRef_ != right->methodRef_)
        return false;
    return true;
}
bool Type::ILSrcDump(Stream& peLib) const
{
    if (tp_ == ClassRef)
    {
        if (showType_)
        {
            if (typeRef_->Flags().Flags() & Qualifiers::Value)
            {
                peLib.Out() << " valuetype ";
            }
            else
            {
                peLib.Out() << " class ";
            }
        }
        std::string name = Qualifiers::GetName("", typeRef_, true);
        if (name[0] != '[')
        {
            peLib.Out() << "'" << name << "'";
            peLib.Out() << static_cast<Class*>(typeRef_)->AdornGenerics(peLib);
        }
        else
        {
            int npos = name.find_first_of("]");
            if (npos != std::string::npos && npos != name.size() - 1)
            {
                peLib.Out() << name.substr(0, npos + 1) + "'" + name.substr(npos + 1) + "'";
                peLib.Out() << static_cast<Class*>(typeRef_)->AdornGenerics(peLib);
            }
            else
            {
                peLib.Out() << "'" << name << "'";
            }
        }
    }
    else if (tp_ == TypeVar)
    {
        peLib.Out() << "!" << VarNum();
    }
    else if (tp_ == MethodParam)
    {
        peLib.Out() << "!!" << VarNum();
    }
    else if (tp_ == MethodRef)
    {
        peLib.Out() << "method ";
        methodRef_->ILSrcDump(peLib, false, true, true);
    }
    else
    {
        peLib.Out() << typeNames_[tp_];
    }
    if (arrayLevel_ == 1)
    {
        peLib.Out() << " []";
    }
    else if (arrayLevel_)
    {
#if 0
        peLib.Out() << " [";
        for (int i = 0; i < arrayLevel_; i++)
        {
            if (i != 0)
                peLib.Out() << ", 0...";
            else
                peLib.Out() << "0...";
        }
        peLib.Out() << "]";
#else
        for (int i = 0; i < arrayLevel_; i++)
        {
            peLib.Out() << "[]";
        }
#endif
    }
    for (int i = 0; i < pointerLevel_; i++)
        peLib.Out() << " *";
    if (byRef_)
        peLib.Out() << "&";
    if (pinned_)
        peLib.Out() << " pinned";
    return true;
}

size_t Type::Render(Stream& peLib, Byte* result)
{
    switch (tp_)
    {
        case ClassRef:
#if 1
        // original
            if (typeRef_->InAssemblyRef())
            {
                typeRef_->PEDump(peLib);
                *(int*)result = typeRef_->PEIndex() | (tTypeRef << 24);
            }
            else
            {
                if (showType_)
                {
                    if (!peIndex_)
                    {
                        size_t sz;
                        Byte* sig = SignatureGenerator::TypeSig(this, sz);
                        size_t signature = peLib.PEOut().HashBlob(sig, sz);
                        delete[] sig;
                        TypeSpecTableEntry* table = new TypeSpecTableEntry(signature);
                        peIndex_ = peLib.PEOut().AddTableEntry(table);
                    }
                    *(int*)result = peIndex_ | (tTypeSpec << 24);
                }
                else
                {
                    *(int*)result = typeRef_->PEIndex() | (tTypeDef << 24);
                }
            }
#else
            // RK: this version works for NAppGui Test3 for array n of *C.Thread;
            // the one above generates newarr [NAppCore]NAppCore/Thread,
            // this one correctly newarr [NAppCore]NAppCore/Thread*
            // but it fails for NAppGui Fractals with an invalid assembly! TODO
            if (typeRef_->InAssemblyRef())
                typeRef_->PEDump(peLib);
            if (showType_)
            {
                if (!peIndex_)
                {
                    size_t sz;
                    Byte* sig = SignatureGenerator::TypeSig(this, sz);
                    size_t signature = peLib.PEOut().HashBlob(sig, sz);
                    delete[] sig;
                    TypeSpecTableEntry* table = new TypeSpecTableEntry(signature);
                    peIndex_ = peLib.PEOut().AddTableEntry(table);
                }
                *(int*)result = peIndex_ | (tTypeSpec << 24);
            }
            else
                *(int*)result = typeRef_->PEIndex() | (tTypeDef << 24);
#endif
            return 4;
            break;
        case MethodRef:
        default:
        {
            if (!peIndex_)
            {
                // if rendering a method as a type we are always going to put the sig
                // in the type spec table
                size_t sz;
                Byte* sig = SignatureGenerator::TypeSig(this, sz);
                size_t signature = peLib.PEOut().HashBlob(sig, sz);
                delete[] sig;
                TypeSpecTableEntry* table = new TypeSpecTableEntry(signature);
                peIndex_ = peLib.PEOut().AddTableEntry(table);
            }
            *(int*)result = peIndex_ | (tTypeSpec << 24);
            return 4;
        }
        break;
    }
    return true;
}
bool BoxedType::ILSrcDump(Stream& peLib) const
{
    // no point in looking up the type name in the assembly for this...
    peLib.Out() << "[mscorlib]System." << typeNames_[tp_];
    return true;
}

size_t BoxedType::Render(Stream& peLib, Byte* result)
{
    if (!peIndex_)
    {
        size_t system = peLib.PEOut().SystemName();
        size_t name = peLib.PEOut().HashString(typeNames_[tp_]);
        Resource* result = nullptr;
        peLib.Find(std::string("System.") + typeNames_[tp_], &result);
        if (result)
        {
            static_cast<Class*>(result)->PEDump(peLib);
            peIndex_ = static_cast<Class*>(result)->PEIndex();
        }
    }
    *(int*)result = peIndex_ | (tTypeRef << 24);
    return 4;
}
}  // namespace DotNetPELib
