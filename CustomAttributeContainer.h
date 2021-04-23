#ifndef DotNetPELib_CUSTOMATTRIBUTECONTAINER
#define DotNetPELib_CUSTOMATTRIBUTECONTAINER

#include <PeLib/Resource.h>
#include <map>
#include <set>
#include <vector>

namespace DotNetPELib
{
    class CustomAttribute;
    typedef unsigned char Byte; /* 1 byte */
    class PELib;
    class AssemblyDef;
    class PEReader;

    ///** class to hold custom attributes.  only parses them at this point, so that
    // you can retrieve attributes from .net assemblies if you want to.  if you
    // want to generate them you are on your own.
    class CustomAttributeContainer : public Resource
    {
    public:
        struct lt
        {
            bool operator()(const CustomAttribute *left, const CustomAttribute *right) const;
        };
        class CustomAttributeDescriptor
        {
        public:
            std::string name;
            Byte *data;
            size_t sz;
            CustomAttributeDescriptor() : data(nullptr), sz(0) { }
            ~CustomAttributeDescriptor() { delete data; }
            bool operator() (const CustomAttributeDescriptor *left, const CustomAttributeDescriptor *right) const;
        };
        CustomAttributeContainer() { }
        ~CustomAttributeContainer();
        void Load(PELib &peLib, AssemblyDef &assembly, PEReader &reader);
        const std::vector<CustomAttributeDescriptor *>& Lookup(CustomAttribute *attribute) const;
        bool Has(CustomAttribute &attribute, const std::string& name,  Byte *data = nullptr, size_t sz = 0) const;
    private:
        std::map<CustomAttribute *, std::vector<CustomAttributeDescriptor *>, lt> attributes;
        std::set<CustomAttributeDescriptor *, CustomAttributeDescriptor> descriptors;
    };
}

#endif // DotNetPELib_CUSTOMATTRIBUTECONTAINER

