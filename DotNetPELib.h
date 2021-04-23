/* Software License Agreement
 * 
 *     Copyright(C) 1994-2020 David Lindauer, (LADSoft)
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

#ifndef DOTNETPELIB_H
#define DOTNETPELIB_H

#include <PeLib/PELibError.h>
#include <PeLib/CodeContainer.h>
#include <PeLib/DataContainer.h>
#include <PeLib/CustomAttributeContainer.h>
#include <PeLib/AssemblyDef.h>
#include <PeLib/Namespace.h>
#include <PeLib/Property.h>
#include <PeLib/Class.h>
#include <PeLib/Method.h>
#include <PeLib/Field.h>
#include <PeLib/Enum.h>
#include <PeLib/Operand.h>
#include <PeLib/Instruction.h>
#include <PeLib/Value.h>
#include <PeLib/MethodSignature.h>
#include <PeLib/Type.h>
#include <PeLib/Allocator.h>
#include <memory>

// reference changelog.txt to see what the changes are
//
#define DOTNETPELIB_VERSION "3.01"

// this is the main library header
// it allows creation of the methods and data that would be dumped into 
// a .net assembly.  The data can be either directly written to an 
// executable file or DLL, or dumped into a .IL file suitable for 
// viewing or assembling with the ilasm assembler.
//
//
//   The general usage of this library is to first create a PELib object,
//   then populate it with objects related to your program.   If you want you can
//   use members of the Allocator class to instantiate the various objects,
//   if you do this they will be deleted when the PELib object is deleted.   Otherwise
//   if you use operator new you are responsible for your own memory management.
//
//	 Once you have created a PELib object, it will create an AssemblyRef object
//   Which is the container for your program.   If you want to write a free standing program
//   you would put your methods and fields into this object.   However if you want to
//   write a DLL that .net assemblies can access, you need to create a namespace in the assemblyref
//   and at least one class in the namespace.   Methods and fields go into the inner class.
//
//   Another thing you have to do for .net compatibility is set the CIL flag
//   in the PELib object.   IF you have a free-standing program you might not 
//   want to set it, for example you can simplify code generation by putting
//   initialized data in the SDATA segment.   To put data in the SDATA segment
//   you add an initializer to a static field. (it makes the equivalent of a .DATA statement)

//   this library has full support for 32- bit pinvoke
//      you have to help it just a little in the case of pinvokes with varargs
//      by creating a method signature for each unique invocation of the pinvoke
//      that is independent of the main method for the pinvoke.
//      the main method does not have a vararg list but they unique invocations do
//      internally, the exe generator will store the main method in the MethodDef table
//      and the unique invocations in the MemberRef table.   An entry in the ImplMap
//      table will give information about what DLL the pinvoke is related to.
//
//   this library has only been tested with 32 bit programs, however it should
//   work with 64 bit programs as long as pinvoke isn't used.
//
//   Note that if you intend to pass managed methods to unmanaged code or vice
//      versa you may need to create transition thunks somewhere.   OCCIL has
//      a library that does this for you, assuming you only want to marshal
//      simple types.
//   OCCIL also has a library which does simple marshalling between C-style strings
//      and .net style strings.
//
//   This libray does simple peephole optimizations when an instruction
//   can be replaced with a shorter equivalent.   For example:
//			ldc.i4 # can sometimes be shortened into one of several optimized instructions
//			branches can sometimes be shortened
//			parameter and local variable indexes can be optimized
//				for local variables the library will also modify the local variable
//				list and put the most used local variables first (as a further optimization)
//          maximum stack is calculated
//
// there are various things missing:
//   Public and Private are the only scope modifiers currently
//	 no support for character set modifiers
//   
//   Custom Attributes cannot in general be defined however internally
//		we do handle the C# vararg one behind the scenes
//   Parameter handling is missing support for [in][out][opt] and default values
//   you can define static data in the CIL data area, but I haven't vetted it enough
//   to know if you can write to it.
//
namespace DotNetPELib
{

    // definitions for some common types
    typedef long long longlong;
    typedef unsigned char Byte; /* 1 byte */


    ///** this is the main class to instantiate
    // the constructor creates a working assembly, you put all your code and data into that
    class PELib : public Allocator
    {
    public:
        enum eFindType {
            s_notFound=0,
            s_ambiguous=1,
            s_namespace,
            s_class,
            s_enum,
            s_field,
            s_property,
            s_method
        };
        enum CorFlags {
            ///** Set this for compatibility with .net assembly imports,
            // unset it for standalone assemblies if you want to modify your
            // sdata
            ilonly = 1,
            ///** Set this if you want to force 32 bits - you will possibly need it
            // for pinvokes
            bits32 = 2
        };
        enum OutputMode { ilasm, peexe, pedll, object };
        ///** Constructor, creates a working assembly
        PELib(const std::string& AssemblyName, int CoreFlags);
        ///** Get the working assembly
        // This is the one with your code and data, that gets written to the output
        AssemblyDef *WorkingAssembly() const { return assemblyRefs_.front(); }
        ///** replace the working assembly with an empty one.   Data is not deleted and
        /// still remains a part of the PELib instance.
        AssemblyDef *EmptyWorkingAssembly(const std::string& AssemblyName);
        ///** Add a reference to another assembly
        // this is an empty assembly you can put stuff in if you want to
        void AddExternalAssembly(const std::string& assemblyName, Byte *publicKeyToken = nullptr);
        ///** Load data out of an assembly
        int LoadAssembly(const std::string& assemblyName, int major = 0, int minor = 0, int build = 0, int revision = 0);
        ///* Load data out of an unmanaged DLL
        int LoadUnmanaged(const std::string& dllName);
        ///** Load an object file
        bool LoadObject(const std::string& name);
        ///** find an unmanaged dll name
        std::string FindUnmanagedName(const std::string& name);
        ///** Find an assembly
        AssemblyDef *FindAssembly(const std::string& assemblyName) const;
        ///** Find a Class
        Class *LookupClass(PEReader &reader, const std::string& assembly, int major, int minor, int build, int revision,
                           size_t keyIndex, const std::string& nameSpace, const std::string& name);
        ///** Pinvoke references are always added to this object
        void AddPInvokeReference(MethodSignature *methodsig, const std::string& dllname, bool iscdecl);
        void AddPInvokeWithVarargs(MethodSignature *methodsig) { pInvokeReferences_.insert(std::pair<std::string, MethodSignature *>(methodsig->Name(), methodsig)); }
        void RemovePInvokeReference(const std::string& name) {
            pInvokeSignatures_.erase(name);
        }
        Method *FindPInvoke(const std::string& name) const;
        MethodSignature *FindPInvokeWithVarargs(const std::string& name, std::vector<Param *>&vargs) const;
        // get the core flags
        int GetCorFlags() const { return corFlags_; }

        ///** write an output file, possibilities are a .il file, an EXE or a DLL
        // the file can also be tagged as either console or win32
        bool DumpOutputFile(const std::string& fileName, OutputMode mode, bool Gui);
        const std::string &FileName() const { return fileName_; }

        ///** add to the search path, returns true if it finds a namespace at path
        // in any assembly
        bool AddUsing(const std::string& path);

        ///** find something, return value tells what type of object was found
        eFindType Find(std::string path, void **result, std::deque<Type*>* generics = nullptr, AssemblyDef *assembly = nullptr);

        ///** find a method, with overload matching
        eFindType Find(std::string path, Method **result, std::vector<Type *> args, Type* rv = nullptr, std::deque<Type*>* generics = nullptr, AssemblyDef *assembly = nullptr, bool matchArgs = true);

        ///** Traverse the declaration tree
        void Traverse(Callback &callback) const;
                
        ///** internal declarations
        // loads the MSCorLib assembly
        AssemblyDef *MSCorLibAssembly();

        std::string FormatName(const std::string& name);
        std::string UnformatName();
        virtual bool ILSrcDump(PELib &) { return ILSrcDumpHeader() && ILSrcDumpFile(); }
        bool ObjOut();
        bool ObjIn();
        longlong ObjInt();
        int ObjHex2();
        char ObjBegin(bool next = true);
        char ObjEnd(bool next = true);
        char ObjChar();
        void ObjBack() { objInputPos_ -= 3; }
        void ObjReset() { objInputPos_ = objInputCache_; }
        void ObjError(int);
        std::iostream &Out() const { return *outputStream_; }
        void Swap(std::unique_ptr<std::iostream>& stream) { outputStream_.swap(stream); }
        PEWriter &PEOut() const { return *peWriter_; }
        std::map<size_t, size_t> moduleRefs;
        void PushContainer(DataContainer *container) { containerStack_.push_back(container); }
        DataContainer *GetContainer() { if (containerStack_.size()) return containerStack_.back(); else return nullptr; }
        void PopContainer() { containerStack_.pop_back(); }
        void SetCodeContainer(CodeContainer *container) { codeContainer_ = container; }
        CodeContainer *GetCodeContainer() const { return codeContainer_; }
        Class* FindOrCreateGeneric(std::string name, std::deque<Type*>& generics);
    protected:
        void SplitPath(std::vector<std::string> & split, std::string path);
        bool ILSrcDumpHeader();
        bool ILSrcDumpFile();
        bool DumpPEFile(std::string name, bool isexe, bool isgui);
        std::list<AssemblyDef *>assemblyRefs_;
        std::map<std::string, Method *>pInvokeSignatures_;
        std::multimap<std::string, MethodSignature *> pInvokeReferences_;
        std::string assemblyName_;
        std::unique_ptr<std::iostream> outputStream_;
        std::fstream *inputStream_;
        std::string fileName_;
    	std::map<std::string, std::string> unmanagedRoutines_;
        int corFlags_;
        PEWriter *peWriter_;
        std::vector<Namespace *> usingList_;
        std::deque<DataContainer *> containerStack_;
        CodeContainer *codeContainer_;
        const char *objInputBuf_;
        int objInputSize_;
        int objInputPos_;
        int objInputCache_;
    };

} // namespace
#endif // DOTNETPELIB_H
