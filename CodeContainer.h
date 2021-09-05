#ifndef DotNetPELib_CODECONTAINER
#define DotNetPELib_CODECONTAINER

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
#include <map>
#include <vector>
#include <list>
#include "SEHData.h"

namespace DotNetPELib
{
    class PELib;
    class Instruction;
    typedef unsigned char Byte; /* 1 byte */

    ///** base class that contains instructions/ labels
    // will be further overridden later to make a 'method'
    // definition
    class CodeContainer : public Resource
    {
    public:
        CodeContainer(Qualifiers Flags) :flags_(Flags), hasSEH_(false), parent_(nullptr) { }

        ///** This is the interface to add a single CIL instruction
        void AddInstruction(Instruction *instruction);

        ///** it is possible to remove the last instruction
        Instruction *RemoveLastInstruction() {
            Instruction *rv = instructions_.back();
            instructions_.pop_back();
            return rv;
        }

        ///** Retrieve the last instruction
        Instruction *LastInstruction() const {
            return instructions_.back();
        }

        ///** Validate instructions
        void ValidateInstructions();

        ///** Validate SEH tags, e.g. make sure there are matching begin and end tags and that filters are organized properly
        void ValidateSEH();

        ///** Validate one level of tags (recursive)
        int ValidateSEHTags(std::vector<Instruction *>&tags, int offset);

        ///** Validate that SEH filter expressions are in the proper place
        void ValidateSEHFilters(std::vector<Instruction *>&tags);

        ///** Validate the epilogue for each SEH section
        void ValidateSEHEpilogues();

        ///** return flags member
        Qualifiers &Flags() { return flags_; }
        const Qualifiers &Flags() const { return flags_; }

        ///** set/get parent
        void SetContainer(DataContainer *parent) { parent_ = parent; }
        DataContainer *GetContainer() const { return parent_; }

        // some internal functions
        bool InAssemblyRef() const;

        void BaseTypes(int &types) const;

        virtual void Optimize(PELib &);

        virtual bool ILSrcDump(PELib &) const;

        virtual bool PEDump(PELib &) { return false; }

        virtual void Render(PELib&) { }

        virtual void ObjOut(PELib &, int pass) const;

        void ObjIn(PELib &);

        Byte *Compile(PELib &, size_t &sz);

        int CompileSEH(std::vector<Instruction *>tags, int offset, std::vector<SEHData> &sehData);

        void CompileSEH(PELib &, std::vector<SEHData> &sehData);

        virtual void Compile(PELib&) { }

        std::list<Instruction *>::iterator begin() { return instructions_.begin(); }
        std::list<Instruction *>::iterator end() { return instructions_.end(); }

    protected:
        std::map<std::string, Instruction *> labels;
        void LoadLabels();
        void OptimizeLDC(PELib &);
        void OptimizeLDLOC(PELib &);
        void OptimizeLDARG(PELib &);
        void OptimizeBranch(PELib &);
        void CalculateOffsets();
        bool ModifyBranches();
        std::list<Instruction *> instructions_;
        Qualifiers flags_;
        DataContainer *parent_;
        bool hasSEH_;
    };
}

#endif // DotNetPELib_CODECONTAINER

