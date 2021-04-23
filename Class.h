#ifndef DotNetPELib_CLASS
#define DotNetPELib_CLASS

#include <PeLib/DataContainer.h>

namespace DotNetPELib
{
    class Property;
    class AssemblyDef;
    class PEReader;

    /* a class, note that it cannot contain namespaces which is enforced at compile time*/
    /* note that all classes have to eventually derive from one of the System base classes
     * but that is handled internally */
     /* enums derive from this */
    class Class : public DataContainer
    {
    public:
        Class(const std::string& Name, Qualifiers Flags, int Pack, int Size) : DataContainer(Name, Flags),
            pack_(Pack), size_(Size), extendsFrom_(nullptr), external_(false), genericParent_(nullptr)
        {
        }
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
        ///** not locally defined
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
        ///** root for Load assembly from file
        void Load(PELib &lib, AssemblyDef &assembly, PEReader &reader, size_t index, int startField, int endField, int startMethod, int endMethod, int startSemantics, int endSemantics);
        virtual bool ILSrcDump(PELib &) const override;
        virtual bool PEDump(PELib &) override;
        void ILSrcDumpClassHeader(PELib &) const;
        virtual void ObjOut(PELib &, int pass) const override;
        static Class *ObjIn(PELib &, bool definition = true);
        std::string AdornGenerics(PELib& peLib, bool names = false) const;
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

