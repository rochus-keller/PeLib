#ifndef DotNetPELib_METHOD
#define DotNetPELib_METHOD

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

#include <PeLib/CodeContainer.h>

namespace DotNetPELib
{
    class MethodSignature;
    class Local;
    class PEMethod;
    class Stream;

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
        void SetPInvoke(const std::string& name, InvokeType type = Stdcall, const std::string& importName = "");
        bool IsPInvoke() const { return invokeMode_ == PInvoke; }

        ///** Add a local variable
        void AddLocal(Local *local);

        void Instance(bool instance);
        bool Instance() const { return !!(Flags().Value & Qualifiers::Instance); }

        MethodSignature *Signature() const { return prototype_; }

        ///** is it an entry point function
        bool HasEntryPoint() const { return entryPoint_; }
        void HasEntryPoint(bool b) { entryPoint_ = b; }

        ///** Iterate through local variables
        typedef std::vector<Local *>::iterator iterator;
        iterator begin() { return varList_.begin(); }
        iterator end() { return varList_.end(); }
        size_t size() const { return varList_.size(); }
        Local* getLocal(int i) const { return varList_[i]; }

        size_t getToken() const { return token_; }

        // Internal functions
        void MaxStack(int stack) { maxStack_ = stack;  }
        virtual bool ILSrcDump(Stream &) const override;
        virtual bool PEDump(Stream &) override;
        virtual void Compile(Stream&) override;
        virtual void Optimize() override;
    protected:
        void OptimizeLocals();
        void CalculateMaxStack();
        void CalculateLive();
        MethodSignature *prototype_;
        std::vector<Local *> varList_;
        std::string pInvokeName_, importName_;
        InvokeMode invokeMode_;
        InvokeType pInvokeType_;
        int maxStack_;
        bool entryPoint_;
        PEMethod *rendering_;
        size_t token_; // redundant from rendering_->methodDef_ because PEWriter deletes all PEMethods
    };
}

#endif // DotNetPELib_METHOD

