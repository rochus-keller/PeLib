#ifndef DotNetPELib_CUSTOMATTRIBUTECONTAINER
#define DotNetPELib_CUSTOMATTRIBUTECONTAINER

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
#include <map>
#include <set>
#include <vector>
#include <string>

namespace DotNetPELib
{
    class CustomAttribute;
    typedef unsigned char Byte; /* 1 byte */
    class PELib;
    class AssemblyDef;

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

        const std::vector<CustomAttributeDescriptor *>& Lookup(CustomAttribute *attribute) const;

        bool Has(CustomAttribute &attribute, const std::string& name,  Byte *data = nullptr, size_t sz = 0) const;
    private:
        std::map<CustomAttribute *, std::vector<CustomAttributeDescriptor *>, lt> attributes;
        std::set<CustomAttributeDescriptor *, CustomAttributeDescriptor> descriptors;
    };
}

#endif // DotNetPELib_CUSTOMATTRIBUTECONTAINER

