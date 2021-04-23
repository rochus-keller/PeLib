#ifndef DotNetPELib_ALLOCATOR
#define DotNetPELib_ALLOCATOR

#include <PeLib/Qualifiers.h>
#include <PeLib/Field.h>
#include <PeLib/Operand.h>

namespace DotNetPELib
{
    class Method;
    class AssemblyDef;
    class Namespace;
    class Class;
    class Field;
    class Enum;
    class Instruction;
    class Value;
    class Local;
    class Param;
    class MethodSignature;
    class Operand;
    class Type;
    class FieldName;
    class DataContainer;
    class PEWriter;
    class PEReader;
    class PEMethod;
    class CustomAttribute;
    class MethodSemanticsTableEntry;
    class Callback;
    class Property;
    class MethodName;
    class BoxedType;
    typedef unsigned char Byte; /* 1 byte */
    typedef long long longlong;

    ///** The allocator manages memory that various objects get constructed into
    // so that the objects can be cleanly deleted without the application having
    // to keep track of every object.
    class Allocator
    {
    public:
        Allocator() : first_(nullptr), current_(nullptr) { }
        virtual ~Allocator() { FreeAll(); }

        AssemblyDef *AllocateAssemblyDef(const std::string& Name, bool External, Byte *KeyToken = 0);
        Namespace *AllocateNamespace(const std::string& Name);
        Class *AllocateClass(const std::string& Name, Qualifiers Flags, int Pack, int Size);
        Class *AllocateClass(const Class* cls);
        Method *AllocateMethod(MethodSignature *Prototype, Qualifiers flags, bool entry = false);
        Field *AllocateField(const std::string& Name, Type *tp, Qualifiers Flags);
        Property *AllocateProperty();
        Property *AllocateProperty(PELib & peLib, const std::string& name, Type *type, std::vector<Type *>& indices, bool hasSetter = true, DataContainer *parent = nullptr);
        Enum *AllocateEnum(const std::string& Name, Qualifiers Flags, Field::ValueSize Size);
        Operand *AllocateOperand();
        Operand *AllocateOperand(Value *V);
        Operand *AllocateOperand(longlong Value, Operand::OpSize Size);
        Operand *AllocateOperand(int Value, Operand::OpSize Size) {
            return AllocateOperand((longlong)Value, Size);
        }
        Operand *AllocateOperand(unsigned Value, Operand::OpSize Size) {
            return AllocateOperand((longlong)Value, Size);
        }
        Operand *AllocateOperand(double Value, Operand::OpSize Size);
        Operand *AllocateOperand(const std::string& Value, bool);
        Operand *AllocateOperand(const std::string& Value);
        Instruction *AllocateInstruction(Instruction::iop Op, Operand *Operand = nullptr);
        Instruction *AllocateInstruction(Instruction::iop Op, const std::string& Text);
        Instruction *AllocateInstruction(Instruction::iseh type, bool begin, Type *catchType = NULL);
        Value *AllocateValue(const std::string& name, Type *tp);
        Local *AllocateLocal(const std::string& name, Type *tp);
        Param *AllocateParam(const std::string& name, Type *tp);
        FieldName *AllocateFieldName(Field *F);
        MethodName *AllocateMethodName(MethodSignature *M);
        MethodSignature *AllocateMethodSignature(const std::string& Name, int Flags, DataContainer *Container);
        MethodSignature *AllocateMethodSignature(const MethodSignature* sig);
        Type *AllocateType(Type::BasicType Tp, int PointerLevel);
        Type *AllocateType(DataContainer *clsref);
        Type *AllocateType(MethodSignature *methodref);
        BoxedType *AllocateBoxedType(Type::BasicType Tp);
        Byte *AllocateBytes(size_t sz);
        enum
        {
            AllocationSize = 0x100000,
        };
    private:
        // heap block
        struct Block
        {
            Block() : next_(nullptr), offset_(0) { memset(bytes_, 0, AllocationSize); }
            Block*next_;
            int offset_;
            Byte bytes_[AllocationSize];
        };
        void *BaseAlloc(size_t size);
        void FreeBlock(Block *b);
        void FreeAll();

        Block *first_, *current_;
    };
}

#endif // DotNetPELib_ALLOCATOR

