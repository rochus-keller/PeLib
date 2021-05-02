#ifndef DotNetPELib_VALUE
#define DotNetPELib_VALUE

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
#include <string>

namespace DotNetPELib
{
    class Type;
    class PELib;
    class Field;
    class MethodSignature;
    typedef unsigned char Byte; /* 1 byte */

    ///** a value, typically to be used as an operand
    // various other classes derive from this to make specific types of operand values
    class Value : public Resource
    {
    public:
        Value(const std::string& Name, Type *tp) : name_(Name), type_(tp) { }
        Value(Type *tp) : type_(tp) { }

        ///** get/set type of value
        Type *GetType() const { return type_; }
        void SetType(Type *tp) { type_ = tp; }

        const std::string &Name() const { return name_; }
        void Name(const std::string name) { name_ = name; }

        ///** internal functions
        virtual bool ILSrcDump(PELib &) const;
        virtual size_t Render(PELib &peLib, int opcode, int OperandType, Byte *);
        virtual void ObjOut(PELib &, int pass) const;
        static Value *ObjIn(PELib &, bool definition = true);
    protected:
        std::string name_;
        Type *type_;
    };

    // value = local variable
    class Local : public Value
    {
    public:
        Local(const std::string& Name, Type *Tp) : Value(Name, Tp), uses_(0), index_(-1) { }

        ///** return index of variable
        int Index() const { return index_; }

        // internal functions
        void IncrementUses() { uses_++; }
        int Uses() const { return uses_; }
        void Index(int Index) { index_ = Index; }
        virtual bool ILSrcDump(PELib &) const override;
        virtual size_t Render(PELib &peLib, int opcode, int OperandType, Byte *) override;
        virtual void ObjOut(PELib &, int pass) const override;
        static Local *ObjIn(PELib &, bool definition = true);
    private:
        int index_;
        int uses_;
    };

    // value: a parameter
    // noticably missing is support for [in][out][opt] and default values
    class Param : public Value
    {
    public:
        Param(const std::string& Name, Type *Tp, int index = -1) : Value(Name, Tp), index_(index) { }

        ///** return index of argument
        void Index(int Index) { index_ = Index; }

        // internal functions
        int Index() const { return index_; }
        virtual bool ILSrcDump(PELib &) const override;
        virtual size_t Render(PELib &peLib, int opcode, int OperandType, Byte *) override;
        virtual void ObjOut(PELib &, int pass) const override;
        static Param *ObjIn(PELib &, bool definition = true);
    private:
        int index_;
    };

    // value: a field name (used as an operand)
    class FieldName : public Value
    {
    public:
        ///** constructor.  Can be used to make the field a reference to another
        // assembly, in a rudimentary way
        FieldName(Field *F) : field_(F), Value("", nullptr){}

        ///** Get the field reference
        Field *GetField() const { return field_; }

        // Internal functions
        virtual bool ILSrcDump(PELib &) const override;
        virtual size_t Render(PELib &peLib, int opcode, int OperandType, Byte *) override;
        virtual void ObjOut(PELib &, int pass) const override;
        static FieldName *ObjIn(PELib &, bool definition = true);
    protected:
        Field *field_;
    };

    // value: a method name (used as an operand)
    class MethodName : public Value
    {
    public:
        MethodName(MethodSignature *M);

        MethodSignature *Signature() const { return signature_; }

        // internal stuff
        virtual bool ILSrcDump(PELib &) const override;
        virtual size_t Render(PELib &peLib, int opcode, int OperandType, Byte *) override;
        virtual void ObjOut(PELib &, int pass) const override;
        static MethodName *ObjIn(PELib &, bool definition = true);
    protected:
        MethodSignature *signature_;
    };
}

#endif // DotNetPELib_VALUE

