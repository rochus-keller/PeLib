#ifndef PELIBERROR
#define PELIBERROR

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

#include  <stdexcept>
#include <string>

namespace DotNetPELib
{
    ///** errors can be thrown, generally when the IL code is being 'optimized'
    // but sometimes at other times
    class PELibError : public std::runtime_error
    {
    public:

        enum ErrorList
        {
            ///** More params are being aded after a vararg param has been added
            VarargParamsAlreadyDeclared,
            ///** If calculations show that the stack would underflow somewhere in the method
            StackUnderflow,
            ///** A label can be reached but the paths don't agree on how many
            // things are currently on the stack
            MismatchedStack,
            ///** The stack is unbalanced when the RET statement is hit
            // This can mean either that there are elements on the stack in
            // a function that has no return value, or there is not exactly '
            //one element on the stack when the function has a return value.
            StackNotEmpty,
            ///** A label has been defined twice
            DuplicateLabel,
            ///** A label that has been referenced was not found
            MissingLabel,
            ///** the short version of some branch was requested, but the target
            // is out of range.
            ShortBranchOutOfRange,
            ///** the short version of an instruction which references a
            // local variable or a parameter was referenced, but the index is out
            // of range
            IndexOutOfRange,
            ///** There are multiple entry points
            MultipleEntryPoints,
            ///** There is no entry point (note that if it is a DLL an entry point isn't needed)
            MissingEntryPoint,
            ///** Expected an SEH Try block
            ExpectedSEHTry,
            ///** Expected an SEH handler
            ExpectedSEHHandler,
            ///** Mismatched SEH end tag
            MismatchedSEHTag,
            ///** SEH tag is orphaned
            OrphanedSEHTag,
            ///** Invalid SEH Filter
            InvalidSEHFilter,
            ///** Seh section not correctly ended
            InvalidSEHEpilogue,
            ///** cannot run more than one instance of PELib at a time
            AlreadyRunning,
            ///** symbol resolution
            NotFound,
            Ambiguous,
            Syntax,
            NotSupported,
        };
        PELibError(ErrorList err, const std::string &Name = "") : errnum(err), std::runtime_error(std::string(errorNames[err]) + " " + Name)
        {
        }
        virtual ~PELibError() { }
        ErrorList Errnum() const { return errnum; }
    private:
        ErrorList errnum;
        static const char *errorNames[];
    };

    class ObjectError : public std::runtime_error
    {
    public:
        ObjectError(char *a) : runtime_error(a) { }
    };
    // an internal enumeration for errors associated with invalidly formatted
    // object files.   Usually only used for debugging the object file handlers
    enum {
        oe_syntax,
        oe_nonamespace,
        oe_noclass,
        oe_noenum,
        oe_nofield,
        oe_nomethod,
        oe_typemismatch,
        oe_corFlagsMismatch,
    };
}

#endif // PELIBERROR

