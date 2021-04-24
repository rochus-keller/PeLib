#ifndef DotNetPELib_FIELD
#define DotNetPELib_FIELD

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
#include <PeLib/Qualifiers.h>

namespace DotNetPELib
{
    class Type;
    typedef unsigned char Byte; /* 1 byte */
    typedef long long longlong;

    ///** a field, could be either static or non-static
    class Field : public Resource
    {
    public:
        ///** Size for enumerated values
        enum ValueSize { i8, i16, i32, i64 };
        ///** Mode for the initialized value
        enum ValueMode {
            ///** No initialized value
            None,
            ///** Enumerated value, goes into the constant table
            Enum,
            ///** Byte stream, goes into the sdata
            Bytes
        };
        Field(const std::string& Name, Type *tp, Qualifiers Flags) : mode_(Field::None), name_(Name), flags_(Flags), parent_(nullptr), size_(i8),
            type_(tp), /*enumValue_(0),*/ byteValue_(nullptr), byteLength_(0), ref_(0), peIndex_(0),
            explicitOffset_(0), external_(false), definitions_(0) {}

        ///** Add an enumeration constant
        // Note that the field does need to be part of an enumeration
        void AddEnumValue(longlong Value, ValueSize Size);
        longlong EnumValue() const { return enumValue_;  }

        ///** Add an SDATA initializer
        void AddInitializer(Byte *bytes, int len); // this will be readonly in ILONLY assemblies

        ///** Field Name
        const std::string &Name() const { return name_; }

        ///** Set the field's container
        void SetContainer(DataContainer *Parent) { parent_ = Parent; }
        DataContainer *GetContainer() const { return parent_; }

        ///** Type of field
        Type *FieldType() const { return type_; }
        void FieldType(Type *tp) { type_ = tp; }

        ///** Field qualifiers
        const Qualifiers &Flags() const { return flags_; }

        //* Is field referenced
        void Ref(bool Ref) { ref_ = Ref; }

        //* Is field referenced
        bool IsRef() const { return ref_; }

        ///** not locally defined
        bool External() const { return external_; }
        void External(bool external) { external_ = external; }

        ///** increment definition count
        ///** get definition count
        void Definition() { definitions_++; }
        size_t Definitions() const { return definitions_;  }

        //* field offset for explicit structures
        void ExplicitOffset(size_t offset) { explicitOffset_ = offset;}
        size_t ExplicitOffset() const { return explicitOffset_; }

        ///** Index in the fielddef table
        size_t PEIndex() const { return peIndex_; }
        void PEIndex(size_t val) { peIndex_ = val; }

        // internal functions
        bool InAssemblyRef() const;
        static bool ILSrcDumpTypeName(PELib &peLib, ValueSize size);
        virtual bool ILSrcDump(PELib &) const;
        virtual bool PEDump(PELib &);
        virtual void ObjOut(PELib &, int pass) const;
        static Field *ObjIn(PELib &, bool definition = true);
    protected:
        DataContainer *parent_;
        std::string name_;
        Qualifiers flags_;
        ValueMode mode_;
        Type *type_;
        union {
            longlong enumValue_;
            Byte *byteValue_;
        };
        int byteLength_;
        ValueSize size_;
        size_t peIndex_;
        size_t explicitOffset_;
        bool ref_;
        bool external_;
        size_t definitions_;
    };
}

#endif // DotNetPELib_FIELD

