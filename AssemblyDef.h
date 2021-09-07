#ifndef DotNetPELib_ASSEMBLYDEF
#define DotNetPELib_ASSEMBLYDEF

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

#include <PeLib/DataContainer.h>
#include <PeLib/CustomAttributeContainer.h>

namespace DotNetPELib
{
    typedef unsigned char Byte; /* 1 byte */
    class Class;
    class Namespace;

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

        ///** lookup or create a class
        Class *LookupClass(PELib &lib, const std::string& nameSpace, const std::string& name);

        const CustomAttributeContainer &CustomAttributes() const { return customAttributes_;  }

        virtual bool InAssemblyRef() const override { return external_; }
        virtual AssemblyDef* getAssembly() { return this; }

        bool IsLoaded() { return loaded_; }

        void SetLoaded() { loaded_ = true; }

        bool ILHeaderDump(Stream& peLib);

        bool PEHeaderDump(Stream&);
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

