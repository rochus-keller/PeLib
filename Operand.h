#ifndef DotNetPELib_OPERAND
#define DotNetPELib_OPERAND

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
    class Value;
    class Stream;
    typedef long long longlong;
    typedef unsigned char Byte; /* 1 byte */

    ///** the operand to an instruction
    // this can contain a number, a string, or a reference to value
    // a value can be a field, methodsignature, local, or param reference
    //
    class Operand : public Resource
    {
    public:
        enum OpSize { any, i8, u8, i16, u16, i32, u32, i64, u64, inative, r4, r8 };
        enum OpType { t_none, t_value, t_int, t_real, t_string, t_label };

        Operand() : type_(t_none), intValue_(0), sz_(i8), refValue_(nullptr), floatValue_(0), property_(0) {} // no operand

        ///** Operand is a complex value
        Operand(Value *V);

        ///** Operand is an integer constant
        Operand(longlong Value, OpSize Size) : type_(t_int), intValue_(Value), sz_(Size), refValue_(nullptr), floatValue_(0), property_(0){}
        Operand(int Value, OpSize Size) : Operand((longlong)Value, Size) { }
        Operand(unsigned Value, OpSize Size) : Operand((longlong)Value, Size) { }

        ///** Operand is a floating point constant
        Operand(double Value, OpSize Size) : type_(t_real), floatValue_(Value), sz_(Size), intValue_(0), refValue_(nullptr), property_(0){}

        ///** Operand is a string
        Operand(const std::string& Value, bool) : type_(t_string), intValue_(0), sz_(i8), refValue_(nullptr), floatValue_(0), property_(0) { stringValue_ = Value; }

        ///** Operand is a label
        Operand(const std::string& Value) : type_(t_label), intValue_(0), sz_(i8), refValue_(nullptr), floatValue_(0), property_(0) { stringValue_ = Value; }

        OpType OperandType() const { return type_; }

        ///** When operand is a complex value, return it
        Value * GetValue() const { return type_ == t_value ? refValue_ : nullptr; }

        ///** return the int value
        longlong IntValue() const { return intValue_; }

        ///** return the string value
        std::string StringValue() const { return stringValue_; }

        ///** return the float value
        double FloatValue() const { return floatValue_; }

        ///** return/set the is-a-property flag
        ///** only has meaning for 'value' operands
        bool Property() const { return property_;  }
        void Property(bool state) { property_ = state;  }

        ///** Internal functions
        virtual bool ILSrcDump(Stream &) const;
        size_t Render(Stream& peLib, int opcode, int operandType, Byte *);
        std::string EscapedString() const;

    protected:
        OpType type_;
        OpSize sz_;
        Value *refValue_;
        std::string stringValue_;
        longlong intValue_;
        double floatValue_;
        bool property_;
        bool isnanorinf() const;
    };
}

#endif // DotNetPELib_OPERAND

