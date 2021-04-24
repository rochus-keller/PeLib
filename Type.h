#ifndef DotNetPELib_TYPE
#define DotNetPELib_TYPE

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

#include <PeLib/Resource.h>
#include <stdlib.h>

namespace DotNetPELib
{
    class DataContainer;
    class MethodSignature;
    class PELib;
    typedef unsigned char Byte; /* 1 byte */

    ///** the type of a field or value
    class Type : public Resource
    {
    public:
        enum BasicType {
            ///** type is a reference to a class
            cls,
            ///** type is a reference to a method signature
            method,
            ///** type is a generic variable
            var,
            ///** type is a generic method param
            mvar,
            /* below this is various CIL types*/
            Void, Bool, Char, i8, u8, i16, u16, i32, u32, i64, u64, inative, unative, r32, r64, object, string
        };

        Type(BasicType Tp, int PointerLevel = 0);
        Type(DataContainer *clsref) : tp_(cls), pointerLevel_(0), arrayLevel_(0), byRef_(false), typeRef_(clsref), methodRef_(nullptr), peIndex_(0), pinned_(false), showType_(false), varnum_(0){}
        Type(MethodSignature *methodref) : tp_(method), pointerLevel_(0), arrayLevel_(0), byRef_(false), typeRef_(nullptr),
            methodRef_(methodref), peIndex_(0), pinned_(false), showType_(false), varnum_(0){}

        ///** Get/set the type of the Type object
        enum BasicType GetBasicType() const { return tp_; }
        void SetBasicType(BasicType type) { tp_ = type; }

        ///** Get the class reference for class type objects
        DataContainer *GetClass() const { return typeRef_; }

        ///** Get the signature reference for method type objects
        MethodSignature *GetMethod() const { return methodRef_; }

        void ShowType() { showType_ = true; }

        void ArrayLevel(int arrayLevel) { arrayLevel_ = arrayLevel;  }
        int ArrayLevel() const { return arrayLevel_;  }

        ///** Pointer indirection count
        void PointerLevel(int n) { pointerLevel_ = n; }
        int PointerLevel() const { return pointerLevel_; }

        ///** Generic variable number
        void VarNum(int n) { varnum_ = n; }
        int VarNum() const { return varnum_; }

        ///** ByRef flag
        void ByRef(bool val) { byRef_ = val; }
        bool ByRef() { return byRef_; }

        ///** Two types are an exact match
        bool Matches(Type *right);

        // internal functions
        virtual bool ILSrcDump(PELib &) const;
        virtual size_t Render(PELib &, Byte *);
        bool IsVoid() { return tp_ == Void && pointerLevel_ == 0; }
        size_t PEIndex() const { return peIndex_; }
        void PEIndex(size_t val) { peIndex_ = val; }
        virtual void ObjOut(PELib &, int pass) const;
        static Type *ObjIn(PELib &);
        bool Pinned() { return pinned_; }
        void Pinned(bool pinned) { pinned_ = pinned; }
    protected:
        bool pinned_;
        int pointerLevel_;
        int varnum_;
        bool byRef_;
        int  arrayLevel_;
        BasicType tp_;
        DataContainer *typeRef_;
        MethodSignature *methodRef_;
        size_t peIndex_;
        bool showType_;
    private:
        static const char *typeNames_[];
    };

    ///** A boxed type, e.g. the reference to a System::* object which
    // represents the basic type
    class BoxedType : public Type
    {
    public:
        BoxedType(BasicType Tp) : Type(Tp, 0){}

        ///** internal functions
        virtual bool ILSrcDump(PELib &) const override;
        virtual size_t Render(PELib &, Byte *) override;
        virtual void ObjOut(PELib &, int pass) const override;
        static BoxedType *ObjIn(PELib &);
    private:
        static const char *typeNames_[];
    };
}

#endif // DotNetPELib_TYPE

