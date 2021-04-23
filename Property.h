#ifndef DotNetPELib_PROPERTY
#define DotNetPELib_PROPERTY

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
        Property(PELib &peLib, const std::string& name, Type *type, std::vector<Type *>& indices, bool hasSetter = true, DataContainer *parent = nullptr)
            : name_(name), parent_(parent), type_(type), flags_(SpecialName), instance_(true), getter_(nullptr), setter_(nullptr)
        {
            CreateFunctions(peLib, indices, hasSetter);
        }
        Property() : parent_(NULL), type_(nullptr), flags_(SpecialName),
            instance_(true), getter_(nullptr), setter_(nullptr) { }
            ///** Set the parent container (always a class)
        void SetContainer(DataContainer *parent, bool add = true);
        ///** Get the parent container (always a class)
        DataContainer* GetContainer() const { return parent_; }
        ///** choose whether it is an instance member or static property
        void Instance(bool instance);
        ///** return whether it is an instance member or static property
        bool Instance() const { return instance_;  }
        ///** set the name
        void Name(const std::string& name) { name_ = name; }
        ///** Get the name
        const std::string &Name() const { return name_; }
        ///* set the type
        void SetType(Type *type) { type_ = type;  }
        ///* get the type
        Type *GetType() const { return type_;  }
        ///** Call the getter, leaving property on stack
        /// If you had other arguments you should push them before the call
        void CallGet(PELib &peLib, CodeContainer *code);
        ///** Call the setter,
        /// If you had other arguments you should push them before the call
        /// then push the value you want to set
        void CallSet(PELib &peLib, CodeContainer *code);
        ///** Get the getter
        Method *Getter() { return getter_;  }
        ///** Get the setter
        Method *Setter() { return setter_;  }

        void Getter(Method *getter) { getter_ = getter; }
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

