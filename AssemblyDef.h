#ifndef DotNetPELib_ASSEMBLYDEF
#define DotNetPELib_ASSEMBLYDEF

#include <PeLib/DataContainer.h>
#include <PeLib/CustomAttributeContainer.h>

namespace DotNetPELib
{
    typedef unsigned char Byte; /* 1 byte */
    class Class;
    class Namespace;
    class PEReader;

    ///** base class for assembly definitions
    // this holds the main assembly ( as a non-external assembly)
    // or can hold an external assembly
    class AssemblyDef : public DataContainer
    {
    public:
        AssemblyDef(const std::string& Name, bool External, Byte * KeyToken = nullptr);
        void SetVersion(int major, int minor, int build, int revision)
        {
            major_ = major;
            minor_ = minor;
            build_ = build;
            revision_ = revision;
        }
        virtual ~AssemblyDef() { }
        ///** get name of strong name key file (will be "" by default)
        const std::string& SNKFile() const { return snkFile_; }
        ///** set name of strong name key file
        void SNKFile(const std::string& file) { snkFile_ = file; }
        ///** root for Load assembly from file
        void Load(PELib &lib, PEReader &reader);
        ///** lookup or create a class
        Class *LookupClass(PELib &lib, const std::string& nameSpace, const std::string& name);
        ///** Set a public key
        void SetPublicKey(PEReader &reader, size_t index);

        const CustomAttributeContainer &CustomAttributes() const { return customAttributes_;  }
        virtual bool InAssemblyRef() const override { return external_; }
        bool IsLoaded() { return loaded_; }
        void SetLoaded() { loaded_ = true; }
        bool ILHeaderDump(PELib &);
        bool PEHeaderDump(PELib &);
        virtual void ObjOut(PELib &, int pass) const override;
        static AssemblyDef *ObjIn(PELib &, bool definition = true);

    protected:
        Namespace *InsertNameSpaces(PELib &lib, std::map<std::string, Namespace *> &nameSpaces, const std::string& name);
        Namespace *InsertNameSpaces(PELib &lib, Namespace *nameSpace, std::string nameSpaceName);
        Class *InsertClasses(PELib &lib, Namespace *nameSpace, Class *cls, std::string name);
    private:
        std::string snkFile_;
        bool external_;
        Byte publicKeyToken_[8];
        int major_, minor_, build_, revision_;
        bool loaded_;
        CustomAttributeContainer customAttributes_;
        std::map<std::string, Namespace *> namespaceCache;
        std::map<std::string, Class *> classCache;
    };
}

#endif // DotNetPELib_ASSEMBLYDEF

