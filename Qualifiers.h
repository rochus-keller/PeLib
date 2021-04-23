#ifndef DOTNETPELIB_QUALIFIERS
#define DOTNETPELIB_QUALIFIERS

#include <string>

namespace DotNetPELib
{
    class PELib;
    class DataContainer;

    // Qualifiers is a generic class that holds all the 'tags' you would see on various objects in
    // the assembly file.   Where possible things are handled implicitly for example 'nested'
    // will automatically be added when a class is nested in another class.
    class Qualifiers
    {
    public:
        enum {
            Public = 0x1,
            Private = 0x2,
            Static = 0x4,
            Instance = 0x8,
            Explicit = 0x10,
            Ansi = 0x20,
            Sealed = 0x40,
            Enum = 0x80,
            Value = 0x100,
            Sequential = 0x200,
            Auto = 0x400,
            Literal = 0x800,
            HideBySig = 0x1000,
            PreserveSig = 0x2000,
            SpecialName = 0x4000,
            RTSpecialName = 0x8000,
            CIL = 0x10000,
            Managed = 0x20000,
            Runtime = 0x40000,
            Virtual = 0x100000, // sealed
            NewSlot = 0x200000 // value
        };
        enum
        {
            // settings appropriate for occil, e.g. everything is static.  add public/private...
            MainClass = Ansi | Sealed,
            ClassClass = Value | Sequential | Ansi | Sealed,
            ClassUnion = Value | Explicit | Ansi | Sealed,
            ClassField = 0,
            FieldInitialized = Static,
            EnumClass = Enum | Auto | Ansi | Sealed,
            EnumField = Static | Literal | Public,
            PInvokeFunc = HideBySig | Static | PreserveSig,
            ManagedFunc = HideBySig | Static | CIL | Managed
        };
        Qualifiers() : flags_(0)
        {
        }
        Qualifiers(int Flags) : flags_(Flags)
        {
        }
        Qualifiers(const Qualifiers &old)
        {
            flags_ = old.flags_;
        }
        Qualifiers &operator |=(int flags)
        {
            flags_ |= flags;
            return *this;
        }
        ///** most qualifiers come before the name of the item
        void ILSrcDumpBeforeFlags(PELib &) const;
        ///** but a couple of the method qualifiers come after the method definition
        void ILSrcDumpAfterFlags(PELib &) const;
        ///** get a name for a DataContainer object, suitable for use in an ASM file
        // The main problem is there is a separator character between the first class encountered
        // and its members, which is different depending on whether it is a type or a field
        static std::string GetName(const std::string& root, const DataContainer *parent, bool type = false);
        static std::string GetObjName(const std::string& root, const DataContainer *parent);
        virtual void ObjOut(PELib &, int pass) const;
        void ObjIn(PELib &, bool definition = true);
        int Flags() const { return flags_; }
        void Flags(int flags) { flags_ = flags;  }
    protected:
        static void ReverseNamePrefix(std::string&rv, const DataContainer *parent, int &pos, bool type);
        static std::string GetNamePrefix(const DataContainer *parent, bool type);
    private:
        static const char * qualifierNames_[];
        static int afterFlags_;
        int flags_;
    };
}

#endif // DOTNETPELIB_QUALIFIERS

