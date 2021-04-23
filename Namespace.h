#ifndef DotNetPELib_NAMESPACE
#define DotNetPELib_NAMESPACE

#include <PeLib/DataContainer.h>

namespace DotNetPELib
{
    ///** a namespace
    class Namespace : public DataContainer
    {
    public:
        Namespace(const std::string& Name) : DataContainer(Name, Qualifiers(0))
        {
        }

        ///** Get the full namespace name including all parents
        std::string ReverseName(DataContainer *child);
        virtual bool ILSrcDump(PELib &) const override;
        virtual bool PEDump(PELib &) override;
        virtual void ObjOut(PELib &, int pass) const override;
        static Namespace *ObjIn(PELib &, bool definition = true);
    };

}

#endif // DotNetPELib_NAMESPACE

