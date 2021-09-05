#ifndef DotNetPELib_CLASS
#define DotNetPELib_CLASS

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

#include <PeLib/DataContainer.h>

namespace DotNetPELib
{
    class Property;
    class AssemblyDef;
    class Stream;

    /* a class, note that it cannot contain namespaces which is enforced at compile time*/
    /* note that all classes have to eventually derive from one of the System base classes
     * but that is handled internally */
     /* enums derive from this */
    class Class : public DataContainer
    {
    public:
        Class(const std::string& Name, Qualifiers Flags, int Pack, int Size) : DataContainer(Name, Flags),
            pack_(Pack), size_(Size), extendsFrom_(nullptr), external_(false), genericParent_(nullptr) { }

        Class(const Class&) = default;

        Class& operator=(const Class&) = default;

        ///** set the structure packing
        void pack(int pk) { pack_ = pk; }

        ///** set the structure size
        void size(int sz) { size_ = sz; }
        int size() { return size_; }

        ///**set the class we are extending from, if this is unset
        // a system class will be chosen based on whether or not the class is a valuetype
        // this may be unset when reading in an assembly, in that case ExtendsName may give the name of a class which is being extended from
        void Extends(Class *extendsFrom) { extendsFrom_ = extendsFrom; }
        Class *Extends() const { return extendsFrom_;  }
        void ExtendsName(std::string&name) { extendsName_ = name; }
        std::string ExtendsName() const { return extendsName_;  }

        ///** not locally defined
        bool External() const { return external_; }
        void External(bool external) { external_ = external; }

        ///** add a property to this container
        void Add(Property *property, bool add = true);
        using DataContainer::Add;

        void GenericParent(Class* cls) { genericParent_ = cls; }
        Class* GenericParent() const { return genericParent_; }

        ///** Traverse the declaration tree
        virtual bool Traverse(Callback &callback) const override;

        ///** return the list of properties
        const std::vector<Property *>& Properties() const { return properties_;  }

        ///** return the list of generics
        std::deque<Type*>& Generic() { return generic_; }
        const std::deque<Type*>& Generic() const { return generic_; }

        virtual bool ILSrcDump(Stream &) const override;

        virtual bool PEDump(Stream &) override;

        void ILSrcDumpClassHeader(Stream&) const;

        std::string AdornGenerics(Stream& peLib, bool names = false) const;

        bool MatchesGeneric(std::deque<Type*>* generics) const;
    protected:
        int TransferFlags() const;
        int pack_;
        int size_;
        Class *extendsFrom_;
        std::string extendsName_;
        std::vector<Property *>properties_;
        bool external_;
        std::deque<Type*> generic_;
        Class* genericParent_;
    };
}

#endif // DotNetPELib_CLASS

