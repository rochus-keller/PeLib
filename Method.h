#ifndef DotNetPELib_METHOD
#define DotNetPELib_METHOD

#include <PeLib/CodeContainer.h>

namespace DotNetPELib
{
    class MethodSignature;
    class Local;
    class PEMethod;

    ///** A method with code
    // CIL instructions are added with the 'Add' member of code container
    class Method : public CodeContainer
    {
    public:
        ///** a call to either managed or unmanaged code
        enum InvokeMode { CIL, PInvoke };
        ///** linkage type for unmanaged call.
        enum InvokeType { Cdecl, Stdcall };
        Method(MethodSignature *Prototype, Qualifiers flags, bool entry = false);

        ///** Set Pinvoke DLL name
        void SetPInvoke(const std::string& name, InvokeType type = Stdcall)
        {
            invokeMode_ = PInvoke;
            pInvokeName_ = name;
            pInvokeType_ = type;
        }
        bool IsPInvoke() const { return invokeMode_ == PInvoke; }
        ///** Add a local variable
        void AddLocal(Local *local);

        void Instance(bool instance);
        bool Instance() const { return !!(Flags().Value & Qualifiers::Instance); }
        ///** return the signature
        MethodSignature *Signature() const { return prototype_; }

        ///** is it an entry point function
        bool HasEntryPoint() const { return entryPoint_; }

        ///** Iterate through local variables
        typedef std::vector<Local *>::iterator iterator;
        iterator begin() { return varList_.begin(); }
        iterator end() { return varList_.end(); }
        size_t size() const { return varList_.size(); }

        // Internal functions
        void MaxStack(int stack) { maxStack_ = stack;  }
        virtual bool ILSrcDump(PELib &) const override;
        virtual bool PEDump(PELib &) override;
        virtual void Compile(PELib&) override;
        virtual void Optimize(PELib &) override;
        virtual void ObjOut(PELib &, int pass) const override;
        static Method *ObjIn(PELib &, bool definition = true, Method **found = nullptr);
    protected:
        void OptimizeLocals(PELib &);
        void CalculateMaxStack();
        void CalculateLive();
        MethodSignature *prototype_;
        std::vector<Local *> varList_;
        std::string pInvokeName_;
        InvokeMode invokeMode_;
        InvokeType pInvokeType_;
        int maxStack_;
        bool entryPoint_;
        PEMethod *rendering_;
    };
}

#endif // DotNetPELib_METHOD

