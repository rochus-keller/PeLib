#ifndef DotNetPELib_OPERAND
#define DotNetPELib_OPERAND

#include <PeLib/Resource.h>
#include <string>

namespace DotNetPELib
{
    class Value;
    class PELib;
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
        ///** Default constructor
        Operand() : type_(t_none), intValue_(0), sz_(i8), refValue_(nullptr), floatValue_(0), property_(0) // no operand
        {
        }
        ///** Operand is a complex value
        Operand(Value *V) : type_(t_value), refValue_(V), property_(false), sz_(i8), intValue_(0), floatValue_(0)
        {
        }
        ///** Operand is an integer constant
        Operand(longlong Value, OpSize Size) : type_(t_int), intValue_(Value), sz_(Size), refValue_(nullptr), floatValue_(0), property_(0)
        {
        }
        Operand(int Value, OpSize Size) : Operand((longlong)Value, Size) { }
        Operand(unsigned Value, OpSize Size) : Operand((longlong)Value, Size) { }
        ///** Operand is a floating point constant
        Operand(double Value, OpSize Size) : type_(t_real), floatValue_(Value), sz_(Size), intValue_(0), refValue_(nullptr), property_(0)
        {
        }
        ///** Operand is a string
        Operand(const std::string& Value, bool) : type_(t_string), intValue_(0), sz_(i8), refValue_(nullptr), floatValue_(0), property_(0) // string
        {
            stringValue_ = Value;
        }
        ///** Operand is a label
        Operand(const std::string& Value) : type_(t_label), intValue_(0), sz_(i8), refValue_(nullptr), floatValue_(0), property_(0) // label
        {
            stringValue_ = Value;
        }
        ///** Get type of operand
        OpType OperandType() const { return type_; }
        ///** When operand is a complex value, return it
        Value * GetValue() const { return type_ == t_value ? refValue_ : nullptr; }
        ///** return the int value
        longlong IntValue() const { return intValue_; }
        ///** return the string value
        std::string StringValue() const { return stringValue_; }
        ///** return the float value
        double FloatValue() const { return floatValue_; }
        ///** return the is-a-property flag
        ///** only has meaning for 'value' operands
        bool Property() const { return property_;  }
        ///** set the is-a-property flag
        void Property(bool state) { property_ = state;  }
        ///** Internal functions
        virtual bool ILSrcDump(PELib &) const;
        size_t Render(PELib &peLib, int opcode, int operandType, Byte *);
        virtual void ObjOut(PELib &, int pass) const;
        static Operand * ObjIn(PELib &);
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

