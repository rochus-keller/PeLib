#ifndef DotNetPELib_PROPERTY
#define DotNetPELib_PROPERTY

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
#include <vector>

namespace DotNetPELib
{
    class Method;
    class DataContainer;
    class Type;
    class PELib;
    class CodeContainer;
    class AssemblyDef;
    class PEReader;
    class Class;

    /* a property, note we are only supporting classic properties here, not any
     * extensions that are allowed in the image file format
     */
    class Property : public Resource
    {
    public:
        enum {
            SpecialName = 0x200,
            RTSpecialName = 0x400,
            HasDefault = 0x1000
        };
        Property(PELib &peLib, const std::string& name, Type *type, std::vector<Type *>& indices,
                 bool hasSetter = true, DataContainer *parent = nullptr);
        Property() : parent_(NULL), type_(nullptr), flags_(SpecialName),
            instance_(true), getter_(nullptr), setter_(nullptr) { }

        ///** Set/Get the parent container (always a class)
        void SetContainer(DataContainer *parent, bool add = true);
        DataContainer* GetContainer() const { return parent_; }

        ///** choose whether it is an instance member or static property
        void Instance(bool instance);
        bool Instance() const { return instance_;  }

        ///** set/get the name
        void Name(const std::string& name) { name_ = name; }
        const std::string &Name() const { return name_; }

        ///* set/get the type
        void SetType(Type *type) { type_ = type;  }
        Type *GetType() const { return type_;  }

        ///** Call the getter, leaving property on stack
        /// If you had other arguments you should push them before the call
        void CallGet(PELib &peLib, CodeContainer *code);

        ///** Call the setter,
        /// If you had other arguments you should push them before the call
        /// then push the value you want to set
        void CallSet(PELib &peLib, CodeContainer *code);

        Method *Getter() { return getter_;  }
        void Getter(Method *getter) { getter_ = getter; }

        Method *Setter() { return setter_;  }
        void Setter(Method *setter) { setter_ = setter; }

        // internal functions
        ///** root for Load assembly from file
        void Load(PELib &lib, AssemblyDef &assembly, PEReader &reader, size_t propIndex, size_t startIndex, size_t startSemantics, size_t endSemantics, std::vector<Method *>& methods);
        virtual bool ILSrcDump(PELib &) const;
        virtual bool PEDump(PELib &);
        virtual void ObjOut(PELib &, int pass) const;
        static Property *ObjIn(PELib &);
    protected:
        void CreateFunctions(PELib &peLib, std::vector<Type *>& indices, bool hasSetter);
    private:
        int flags_;
        bool instance_;
        std::string name_;
        Type *type_;
        DataContainer *parent_;
        Method *getter_;
        Method *setter_;
    };
}
#endif // DotNetPELib_PROPERTY

