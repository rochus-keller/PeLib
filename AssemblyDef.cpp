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

#include "AssemblyDef.h"
#include "PEWriter.h"
#include "Namespace.h"
#include "Class.h"
#include "Enum.h"
#include "PELibError.h"
#include "PELib.h"
#include "sha1.h"
#include <climits>
namespace DotNetPELib
{
bool AssemblyDef::ILHeaderDump(PELib& peLib)
{
    peLib.Out() << ".assembly ";
    if (external_)
        peLib.Out() << "extern ";
    peLib.Out() << "'" << name_ << "' {" << std::endl;
    if (major_ || minor_ || build_ || revision_)
        peLib.Out() << "\t.ver " << major_ << ":" << minor_ << ":" << build_ << ":" << revision_ << std::endl;
    for (int i = 0; i < 8; i++)
    {
        if (publicKeyToken_[i])
        {
            peLib.Out() << "\t.publickeytoken = (";
            for (int i = 0; i < 8; i++)
            {
                peLib.Out() << std::hex << (int)(unsigned char)publicKeyToken_[i] << " ";
            }
            peLib.Out() << ")" << std::endl << std::dec;
            break;
        }
    }
    peLib.Out() << "}" << std::endl;
    return true;
}
bool AssemblyDef::PEHeaderDump(PELib& peLib)
{
    size_t nameIndex = peLib.PEOut().HashString(name_);
    TableEntryBase* table;
    if (external_)
    {
        size_t blobIndex = 0;
        for (int i = 0; i < 8; i++)
        {
            if (publicKeyToken_[i])
            {
                blobIndex = peLib.PEOut().HashBlob(publicKeyToken_, 8);
                break;
            }
        }
        table = new AssemblyRefTableEntry(PA_None, major_, minor_, build_, revision_, nameIndex, blobIndex);
    }
    else
    {
        table = new AssemblyDefTableEntry(PA_None, major_, minor_, build_, revision_, nameIndex);
    }
    peIndex_ = peLib.PEOut().AddTableEntry(table);
    return true;
}
Namespace* AssemblyDef::InsertNameSpaces(PELib& lib, std::map<std::string, Namespace*>& nameSpaces, const std::string& name)
{
    if (nameSpaces.find(name) == nameSpaces.end())
    {
        Namespace* parent = nullptr;
        int n = name.find_last_of(".");
        std::string end = name;
        if (n != std::string::npos)
        {
            parent = InsertNameSpaces(lib, nameSpaces, name.substr(0, n));
            end = name.substr(n + 1);
        }
        Namespace* rv = nullptr;
        if (parent)
        {
            DataContainer* dc = parent->FindContainer(end);
            if (dc && typeid(*dc) == typeid(Namespace))
            {
                rv = parent = static_cast<Namespace*>(dc);
            }
        }
        else
        {
            DataContainer* dc = FindContainer(end);
            if (dc && typeid(*dc) == typeid(Namespace))
            {
                rv = parent = static_cast<Namespace*>(dc);
            }
        }
        if (!rv)
        {
            nameSpaces[name] = new Namespace(end);
            if (parent)
                parent->Add(nameSpaces[name]);
        }
        else
        {
            nameSpaces[name] = rv;
        }
    }
    return nameSpaces[name];
}
Namespace* AssemblyDef::InsertNameSpaces(PELib& lib, Namespace* nameSpace, std::string name)
{
    auto it = namespaceCache.find(name);
    if (it != namespaceCache.end())
    {
        return it->second;
    }
    std::string in = name;
    size_t n = name.find_last_of(".");
    if (n != std::string::npos)
    {
        nameSpace = InsertNameSpaces(lib, nullptr, name.substr(0, n));
        name = name.substr(n + 1);
    }
    Namespace* rv = nullptr;
    if (nameSpace)
    {
        DataContainer* dc = nameSpace->FindContainer(name);
        if (dc && typeid(*dc) == typeid(Namespace))
        {
            rv = nameSpace = static_cast<Namespace*>(dc);
        }
    }
    else
    {
        DataContainer* dc = FindContainer(name);
        if (dc && typeid(*dc) == typeid(Namespace))
        {
            rv = nameSpace = static_cast<Namespace*>(dc);
        }
    }
    if (!rv)
    {
        rv = new Namespace(name);
        if (nameSpace)
        {
            nameSpace->Add(rv);
        }
        else
        {
            Add(rv);
        }
        nameSpace = rv;
        namespaceCache[in] = nameSpace;
    }
    return nameSpace;
}
Class* AssemblyDef::InsertClasses(PELib& lib, Namespace* nameSpace, Class* cls, std::string name)
{
    size_t n = name.find_last_of(".");
    if (n != std::string::npos)
    {
        cls = InsertClasses(lib, nameSpace, nullptr, name.substr(0, n));
        name = name.substr(n + 1);
    }
    Class* rv = nullptr;
    if (cls)
    {
        rv = static_cast<Class*>(cls->FindContainer(name));
    }
    else
    {
        DataContainer* dc = nameSpace->FindContainer(name);
        if (dc && (typeid(*dc) == typeid(Class) || typeid(*dc) == typeid(Enum)))
        {
            rv = static_cast<Class*>(dc);
        }
    }
    if (!rv)
    {
        rv = new Class(name, 0, -1, -1);
        if (cls)
            cls->Add(rv);
        else
            nameSpace->Add(rv);
    }
    return rv;
}
Class* AssemblyDef::LookupClass(PELib& lib, const std::string& nameSpaceName, const std::string& name)
{
    auto in = nameSpaceName + "::" + name;
    auto it = classCache.find(in);
    if (it != classCache.end())
    {
        return it->second;
    }

    Namespace* nameSpace = InsertNameSpaces(lib, nullptr, nameSpaceName);
    auto rv = InsertClasses(lib, nameSpace, nullptr, name);
    classCache[in] = rv;
    return rv;
}


void AssemblyDef::ObjOut(PELib& peLib, int pass) const
{
    if (loaded_)
    {
        if (pass == 1)
        {
            peLib.Out() << std::endl << "$abr" << peLib.FormatName(name_);
            peLib.Out() << std::endl << "$ae";
        }
        return;
    }
    else
    {
        peLib.Out() << std::endl << "$abb" << peLib.FormatName(name_) << external_;
        DataContainer::ObjOut(peLib, pass);
        peLib.Out() << std::endl << "$ae";
    }
}

AssemblyDef::AssemblyDef(const std::string& Name, bool External, Byte* KeyToken): DataContainer(Name, 0), external_(External),
    major_(0), minor_(0), build_(0), revision_(0), loaded_(false)
{
    if (KeyToken)
        memcpy(publicKeyToken_, KeyToken, 8);
    else
        memset(publicKeyToken_, 0, 8);
}

}  // namespace DotNetPELib
