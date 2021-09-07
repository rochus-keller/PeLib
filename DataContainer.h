#ifndef DotNetPELib_DATACONTAINER
#define DotNetPELib_DATACONTAINER

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
#include <deque>
#include <map>
#include <list>
#include <vector>

namespace DotNetPELib
{
    class Field;
    class Type;
    class Callback;
    class CodeContainer;
    class Stream;
    class AssemblyDef;

    ///** base class that contains other datacontainers or codecontainers
    // that means it can contain namespaces, classes, methods, or fields
    // The main assemblyref which holds everything is one of these,
    // which means it acts as the 'unnamed' namespace.
    // when this class is overridden as something other than a namespace,
    // it cannot contain namespaces
    class DataContainer : public Resource
    {
    public:
        ///** all classes have to extend from SOMETHING...
        // this is enumerations for the ones we can create by default
        enum
        {
            ///**  reference to 'System::Object'
            basetypeObject = 1,
            ///**  reference to 'System::Value'
            basetypeValue = 2,
            ///**  reference to 'System::Enum'
            basetypeEnum = 4,
            ///** reference to 'System' namespace
            baseIndexSystem = 8
        };

        DataContainer(const std::string& Name, Qualifiers Flags) : name_(Name), flags_(Flags),
            parent_(nullptr), instantiated_(false), peIndex_(0), assemblyRef_(false){}

        ///** Add another data container
        // This could be an assemblydef, namespace, class, or enumeration
        void Add(DataContainer *item);

        ///** Add a code container
        // This is always a Method definition
        void Add(CodeContainer *item);

        ///** Add a field
        void Add(Field *field);

        ///** A flag an app can use to tell if the class has been instantiated,
        // for example it might be used after a forward reference is resolved.
        // it is not used internally to the library
        bool IsInstantiated() const { return instantiated_; }
        void SetInstantiated() { instantiated_ = true; }

        ///** The immediate parent
        DataContainer *Parent() const { return parent_; }
        void Parent(DataContainer* parent) { parent_ = parent; }

        ///** The inner namespace parent
        size_t ParentNamespace(Stream& peLib) const;

        ///** The closest parent class
        size_t ParentClass(Stream &peLib) const;

        ///** The parent assembly
        size_t ParentAssembly(Stream& peLib) const;

        ///** The name
        const std::string &Name() const { return name_; }

        ///** The qualifiers
        Qualifiers &Flags() { return flags_; }

        ///** metatable index in the PE file for this data container
        size_t PEIndex() const { return peIndex_; }

        ///** metatable index in the PE file for this data container
        void PEIndex(size_t index) { peIndex_ = index; }

        ///* find a sub-container
        DataContainer *FindContainer(const std::string& name, std::deque<Type*>* generics = nullptr);

        ///** Find a sub- container
        DataContainer *FindContainer(std::vector<std::string>& split, size_t &n,
                                     std::deque<Type*>* generics = nullptr, bool method = false);

        const std::list<Field *>&Fields() const { return fields_; }

        const std::list<CodeContainer *>&Methods() const { return methods_; }

        ///** Traverse the declaration tree
        virtual bool Traverse(Callback &callback) const;

        // internal functions
        virtual bool InAssemblyRef() const { return parent_->InAssemblyRef(); }
        virtual AssemblyDef* getAssembly() { return parent_->getAssembly(); }

        virtual bool ILSrcDump(Stream &) const;

        virtual bool PEDump(Stream &);

        virtual void Compile(Stream&);

        void Number(int &n);

        // sometimes we want to traverse upwards in the tree
        void Render(Stream&);

        void BaseTypes(int &types) const;

        void Clear() { children_.clear(); methods_.clear(); sortedChildren_.clear(); fields_.clear(); }

    protected:
        std::list<DataContainer *> children_;
        std::list<CodeContainer *> methods_;
        std::map<std::string, std::deque<DataContainer *>> sortedChildren_;
        std::list<Field *> fields_;
        DataContainer *parent_;
        Qualifiers flags_;
        std::string name_;
        bool instantiated_;
        size_t peIndex_; // generic index into a table or stream
        bool assemblyRef_;
    };
}

#endif // DotNetPELib_DATACONTAINER

