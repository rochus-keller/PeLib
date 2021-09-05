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

#include "CustomAttributeContainer.h"
#include "PEMetaTables.h"
#include "AssemblyDef.h"

namespace DotNetPELib
{
CustomAttributeContainer::~CustomAttributeContainer()
{
    for (auto a : attributes)
    {
        delete a.first;
    }
    for (auto s : descriptors)
    {
        delete s;
    }
}
bool CustomAttributeContainer::lt::operator()(const CustomAttribute* left, const CustomAttribute* right) const
{
    if (left->tag_ < right->tag_)
        return true;
    else if (left->tag_ == right->tag_ && left->index_ < right->index_)
        return true;
    return false;
}
bool CustomAttributeContainer::CustomAttributeDescriptor::operator()(const CustomAttributeDescriptor* left,
                                                                     const CustomAttributeDescriptor* right) const
{
    if (left->name < right->name)
        return true;
    if (left->name == right->name)
    {
        if (!left->data && right->data)
            return true;
        else if (left->data && right->data)
        {
            if (left->sz < right->sz)
                return true;
            else
            {
                if (left->sz == right->sz)
                    return memcmp(left->data, right->data, right->sz) < 0;
            }
        }
    }
    return false;
}
const std::vector<CustomAttributeContainer::CustomAttributeDescriptor*>&
CustomAttributeContainer::Lookup(CustomAttribute* attribute) const
{
    static std::vector<CustomAttributeDescriptor*> empty;

    auto it = attributes.find(attribute);
    if (it != attributes.end())
        return it->second;
    else
        return empty;
}
bool CustomAttributeContainer::Has(CustomAttribute& attribute, const std::string& name, Byte* data, size_t sz) const
{
    auto it = attributes.find(&attribute);
    if (it != attributes.end())
    {
        for (auto a : it->second)
        {
            if (a->name == name)
                if (!data || (sz == a->sz && !memcmp(data, a->data, sz)))
                    return true;
        }
    }
    return false;
}
}  // namespace DotNetPELib
