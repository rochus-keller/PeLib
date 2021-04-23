#ifndef DotNetPELib_ENUM
#define DotNetPELib_ENUM

#include <PeLib/Class.h>
#include <PeLib/Field.h>

namespace DotNetPELib
{
    class Allocator;

    ///** A special kind of class: enum
    class Enum : public Class
    {
    public:
        Enum(const std::string& Name, Qualifiers Flags, Field::ValueSize Size) :
            size(Size), Class(Name, Flags.Flags() | Qualifiers::Value, -1, -1)
        {
        }
        ///** Add an enumeration, give it a name and a value
        // This creates the Field definition for the enumerated value
        Field *AddValue(Allocator &allocator, const std::string& Name, longlong Value);

        // internal functions
        virtual bool ILSrcDump(PELib &) const override;
        virtual bool PEDump(PELib &) override;
        virtual void ObjOut(PELib &, int pass) const override;
        static Enum *ObjIn(PELib &, bool definition = true);
    protected:
        Field::ValueSize size;
    };
}

#endif // DotNetPELib_ENUM

