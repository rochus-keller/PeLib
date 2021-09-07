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

#include "DataContainer.h"
#include "Callback.h"
#include "PEMetaTables.h"
#include "Stream.h"
#include "PEWriter.h"
#include "Field.h"
#include "Namespace.h"
#include "Class.h"
#include "AssemblyDef.h"
#include "PELibError.h"
#include "Enum.h"
#include "Method.h"
#include <typeinfo>
namespace DotNetPELib
{
void DataContainer::Add(DataContainer* item)
{
    if (item)
    {
        item->parent_ = this;
        children_.push_back(item);
        sortedChildren_[item->name_].push_back(item);
    }
}

void DataContainer::Add(CodeContainer* item)
{
    if (item)
    {
        item->SetContainer(this);
        methods_.push_back(item);
    }
}

void DataContainer::Add(Field* field)
{
    if (field)
    {
        field->SetContainer(this);
        fields_.push_back(field);
    }
}
size_t DataContainer::ParentNamespace(Stream& peLib) const
{
    DataContainer* current = this->Parent();
    while (current && typeid(*current) != typeid(Namespace))
        current = current->Parent();
    if (current)
    {
        if (current && current->InAssemblyRef())
            static_cast<Namespace*>(current)->PEDump(peLib);
        return current->PEIndex();
    }
    return 0;
}
size_t DataContainer::ParentClass(Stream& peLib) const
{
    DataContainer* current = Parent();
    if (current && typeid(*current) == typeid(Class))
    {
        if (current && current->InAssemblyRef())
            static_cast<Class*>(current)->PEDump(peLib);
        return current->PEIndex();
    }
    return 0;
}
size_t DataContainer::ParentAssembly(Stream& peLib) const
{
    // the parent assembly is always at top of the datacontainer tree
    DataContainer* current = Parent();
    while (current->Parent() && typeid(*current) != typeid(AssemblyDef))
    {
        current = current->Parent();
    }
    if (current->InAssemblyRef())
        ; // TODO original leads to infinite loop: static_cast<AssemblyDef*>(current)->PEDump(peLib);
    return current->PEIndex();
}
DataContainer *DataContainer::FindContainer(const std::string& name, std::deque<Type*>* generics)
{
    if (!generics)
    {
        if (sortedChildren_[name].size() > 0)
            return sortedChildren_[name].front();
    }
    else
    {
        for (auto f : sortedChildren_[name])
        {
            if (typeid(*f) == typeid(Class))
            {
                if (static_cast<Class*>(f)->MatchesGeneric(generics))
                    return f;
            }
        }
    }
    return nullptr;
}
DataContainer* DataContainer::FindContainer(std::vector<std::string>& split, size_t& n, std::deque<Type*>* generics, bool method)
{
    n = 0;
    int count = 1;
    if (method)
        count++;
    DataContainer *current = this, *rv = current;
    for (int i = 0; i < split.size(); i++)
    {
        current = current->FindContainer(split[i],  i == split.size() - count ? generics : nullptr);
        if (!current)
            break;
        rv = current;
        n++;
    }
    return rv;
}
bool DataContainer::ILSrcDump(Stream& peLib) const
{
    for (std::list<Field*>::const_iterator it = fields_.begin(); it != fields_.end(); ++it)
        (*it)->ILSrcDump(peLib);
    for (std::list<CodeContainer*>::const_iterator it = methods_.begin(); it != methods_.end(); ++it)
        (*it)->ILSrcDump(peLib);
    for (std::list<DataContainer*>::const_iterator it = children_.begin(); it != children_.end(); ++it)
        (*it)->ILSrcDump(peLib);
    return true;
}

bool DataContainer::PEDump(Stream& peLib)
{
    for (auto field : fields_)
        field->PEDump(peLib);
    for (auto method : methods_)
        method->PEDump(peLib);
    for (auto child : children_)
        child->PEDump(peLib);
    return true;
}
void DataContainer::Number(int& n)
{
    if (typeid(*this) != typeid(Namespace))
        peIndex_ = n++;
    for (auto child : children_)
        child->Number(n);
}
void DataContainer::Compile(Stream& peLib)
{
    for (auto method : methods_)
        method->Compile(peLib);
    for (auto child : children_)
        child->Compile(peLib);
}
void DataContainer::BaseTypes(int& types) const
{
    for (auto method : methods_)
        method->BaseTypes(types);
    for (auto child : children_)
        child->BaseTypes(types);

    if (typeid(*this) == typeid(Enum))
    {
        types |= basetypeEnum;
    }
    else
    {
        if (typeid(*this) != typeid(Namespace))
        {
            if (flags_.Flags() & Qualifiers::Value)
                types |= basetypeValue;
            else
                types |= basetypeObject;
        }
    }
}
bool DataContainer::Traverse(Callback& callback) const
{
    for (auto child : children_)
        if (typeid(*child) == typeid(Class))
        {
            if (!callback.EnterClass(static_cast<const Class*>(child)))
                continue;
            if (!child->Traverse(callback))
                return true;
            if (!callback.ExitClass(static_cast<const Class*>(child)))
                return false;
        }
        else if (typeid(*child) == typeid(Enum))
        {
            if (!callback.EnterEnum(static_cast<const Enum*>(child)))
                continue;
            if (!child->Traverse(callback))
                return true;
            if (!callback.ExitEnum(static_cast<const Enum*>(child)))
                return false;
        }
        else if (typeid(*child) == typeid(Namespace))
        {
            if (!callback.EnterNamespace(static_cast<const Namespace*>(child)))
                continue;
            if (!child->Traverse(callback))
                return true;
            if (!callback.ExitNamespace(static_cast<const Namespace*>(child)))
                return false;
        }
    for (auto field : fields_)
        if (!callback.EnterField(field))
            return false;
    for (auto method : methods_)
        if (!callback.EnterMethod(static_cast<Method*>(method)))
            return false;
    return true;
}
}  // namespace DotNetPELib
