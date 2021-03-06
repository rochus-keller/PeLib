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

#include "PELib.h"
#include "Callback.h"
#include "PEWriter.h"
#include "AssemblyDef.h"
#include "Method.h"
#include "MethodSignature.h"
#include "Value.h"
#include "Type.h"
#include "Namespace.h"
#include "Class.h"
#include "Enum.h"
#include "Field.h"
#include "Property.h"
#include "PELibError.h"
#include <cassert>
#include <fstream>

#define OBJECT_FILE_VERSION "100"

namespace DotNetPELib
{

static PELib* s_this = 0;

extern std::string DIR_SEP;
PELib::PELib(const std::string& AssemblyName, int CoreFlags) :
    corFlags_(CoreFlags),
    codeContainer_(nullptr),
    objInputBuf_(nullptr),
    objInputSize_(0),
    objInputPos_(0),
    objInputCache_(0)
{
    if( s_this != 0 )
        throw PELibError(PELibError::AlreadyRunning);
    s_this = this;
    // create the working assembly.   Note that this will ALWAYS be the first
    // assembly in the list
    AssemblyDef* assemblyRef = new AssemblyDef(AssemblyName, false);
    assemblyRefs_.push_back(assemblyRef);
}

PELib::~PELib()
{
    std::deque<Resource*> all;
    all.swap(Resource::s_all);
    assert( Resource::s_all.empty() );
    std::deque<Resource*>::const_iterator j;
    for( j = all.begin(); j != all.end(); ++j )
    {
        if( *j )
            delete *j;
    }
    s_this = 0;
}
AssemblyDef* PELib::EmptyWorkingAssembly(const std::string& AssemblyName)
{
    AssemblyDef* assemblyRef = new AssemblyDef(AssemblyName, false);
    assemblyRefs_.pop_front();
    assemblyRefs_.push_front(assemblyRef);
    return assemblyRef;
}
bool PELib::DumpOutputFile(const std::string& file, OutputMode mode, bool gui)
{
    bool rv;
    switch (mode)
    {
        case ilasm:
            rv = ILSrcDump(file);
            break;
        case peexe:
            rv = DumpPEFile(file, true, gui);
            break;
        case pedll:
            rv = DumpPEFile(file, false, gui);
            break;
        default:
            rv = false;
            break;
    }
    return rv;
}
AssemblyDef* PELib::AddExternalAssembly(const std::string& assemblyName, Byte* publicKeyToken)
{
    AssemblyDef* assemblyRef = new AssemblyDef(assemblyName, true, publicKeyToken);
    assemblyRefs_.push_back(assemblyRef);
    return assemblyRef;
}

void PELib::SetLibPath(const std::string& paths)
{
    libPath_ = paths;
}
void PELib::AddPInvokeReference(MethodSignature* methodsig, const std::string& dllname, bool iscdecl)
{
    Method* m = new Method(methodsig, Qualifiers::PInvokeFunc | Qualifiers::Public);
    m->SetPInvoke(dllname, iscdecl ? Method::Cdecl : Method::Stdcall);
    pInvokeSignatures_[methodsig->Name()] = m;
}

void PELib::AddPInvokeWithVarargs(MethodSignature* methodsig)
{
    pInvokeReferences_.insert(std::pair<std::string, MethodSignature *>(methodsig->Name(), methodsig));
}
Method* PELib::FindPInvoke(const std::string& name) const
{
    auto it = pInvokeSignatures_.find(name);
    if (it != pInvokeSignatures_.end())
        return it->second;
    return nullptr;
}
MethodSignature* PELib::FindPInvokeWithVarargs(const std::string& name, std::vector<Param*>& vargs) const
{
    auto range = pInvokeReferences_.equal_range(name);
    for (auto it = range.first; it != range.second; ++it)
    {
        if (vargs.size() == (*it).second->VarargParamCount())
        {
            auto it1 = (*it).second->vbegin();
            auto it2 = vargs.begin();
            while (it2 != vargs.end())
            {
                if (!(*it2)->GetType()->Matches((*it1)->GetType()))
                    break;
                ++it1;
                ++it2;
            }
            if (it2 == vargs.end())
                return (*it).second;
        }
    }
    return nullptr;
}
bool PELib::AddUsing(const std::string& path)
{
    std::vector<std::string> split;
    SplitPath(split, path);
    bool found = false;
    for (auto a : assemblyRefs_)
    {
        size_t n = 0;
        DataContainer* container = a->FindContainer(split, n);
        if (n == split.size() && container && typeid(*container) == typeid(Namespace))
        {
            usingList_.push_back(static_cast<Namespace*>(container));
            found = true;
        }
    }
    return found;
}
void PELib::SplitPath(std::vector<std::string>& split, std::string path)
{
    std::string last;
    int n = path.find(":");
    if (n != std::string::npos && n < path.size() - 2 && path[n + 1] == ':')
    {
        last = path.substr(n + 2);
        path = path.substr(0, n);
    }
    n = path.find(".");
    while (n != std::string::npos)
    {
        split.push_back(path.substr(0, n));
        if (path.size() > n + 1)
            path = path.substr(n + 1);
        else
            path = "";
        n = path.find(".");
    }
    if (path.size())
    {
        split.push_back(path);
    }
    if (last.size())
    {
        split.push_back(last);
    }
    if (split.size() > 2)
    {
        if (split[split.size() - 1] == "ctor" || split[split.size() - 1] == "cctor")
            if (split[split.size() - 2] == "")
            {
                split[split.size() - 2] = "." + split[split.size() - 1];
                split.resize(split.size() - 1);
            }
    }
}
PELib::eFindType PELib::Find(std::string path, Resource** result, std::deque<Type*>* generics, AssemblyDef *assembly)
{
    for( int i = 0; i < path.size(); i++ )
    {
        if( path[i] == '/' )
            path[i] = '.';
    }
    if (path.size() && path[0] == '[')
    {
        size_t npos = path.find(']');
        if (npos != std::string::npos)
        {
            std::string assemblyName = path.substr(1, npos - 1);
            path = path.substr(npos + 1);
            assembly = FindAssembly(assemblyName);
        }
    }
    std::vector<std::string> split;
    SplitPath(split, path);
    std::vector<DataContainer*> found;
    std::vector<Field*> foundField;
    std::vector<Method*> foundMethod;
    std::vector<Property*> foundProperty;

    for (AssemblyDef* a : assemblyRefs_)
    {
        //            if (a->InAssemblyRef())
        {
            if (!assembly || assembly == a)
            {
                size_t n = 0;
                DataContainer* dc = a->FindContainer(split, n, generics);
                if (dc)
                {
                    if (n == split.size())
                    {
                        found.push_back(dc);
                    }
                    else if (n == split.size() - 1 &&
                             (typeid(*dc) == typeid(Class) || typeid(*dc) == typeid(Enum) ||
                              typeid(*dc) == typeid(AssemblyDef)))
                    {
                        for (auto field : dc->Fields())
                        {
                            if (field->Name() == split[n])
                                foundField.push_back(field);
                        }
                        for (auto cc : dc->Methods())
                        {
                            Method* method = static_cast<Method*>(cc);
                            if (method->Signature()->Name() == split[n])
                                foundMethod.push_back(method);
                        }
                        if (typeid(*dc) == typeid(Class))
                        {
                            for (auto cc : static_cast<Class*>(dc)->Properties())
                            {
                                if (cc->Name() == split[n])
                                    foundProperty.push_back(cc);
                            }
                        }
                    }
                }
            }
        }
    }
    for (auto u : usingList_)
    {
        size_t n = 0;
        DataContainer* dc = u->FindContainer(split, n, generics);
        if (dc)
        {
            if (n == split.size())
            {
                found.push_back(dc);
            }
            else if (n == split.size() - 1 && (typeid(*dc) == typeid(Class) || typeid(*dc) == typeid(Enum)))
            {
                for (auto field : dc->Fields())
                {
                    if (field->Name() == split[n])
                        foundField.push_back(field);
                }
                for (auto cc : dc->Methods())
                {
                    Method* method = static_cast<Method*>(cc);
                    if (method->Signature()->Name() == split[n])
                        foundMethod.push_back(method);
                }
                if (typeid(*dc) == typeid(Class))
                {
                    for (auto cc : static_cast<Class*>(dc)->Properties())
                    {
                        if (cc->Name() == split[n])
                            foundProperty.push_back(cc);
                    }
                }
            }
        }
    }
    int n = found.size() + foundField.size() + foundMethod.size() + foundProperty.size();
    if (!n)
        return s_notFound;
    else if (n > 1)
    {
        return s_ambiguous;
    }
    else if (found.size())
    {
        *result = found[0];
        if (typeid(*found[0]) == typeid(Namespace))
            return s_namespace;
        else if (typeid(*found[0]) == typeid(Class))
            return s_class;
        else if (typeid(*found[0]) == typeid(Enum))
            return s_enum;
    }
    else if (foundMethod.size())
    {
        *result = foundMethod[0];
        return s_method;
    }
    else if (foundField.size())
    {
        *result = foundField[0];
        return s_field;
    }
    else if (foundProperty.size())
    {
        *result = foundProperty[0];
        return s_property;
    }
    return s_notFound;
}
Class* PELib::FindOrCreateGeneric(std::string name, std::deque<Type*>& generics)
{
    Resource *result = nullptr;
    if (Find(name, &result, &generics) == s_class)
    {
        return static_cast<Class *>(result);
    }
    if (Find(name, &result) == s_class)
    {
        Class* rv = new Class(*static_cast<Class*>(result));
        rv->Generic() = generics;
        rv->GenericParent(static_cast<Class*>(result));
        static_cast<Class *>(result)->Parent()->Add(rv);
        rv->Clear();
        for (auto m : static_cast<Class*>(result)->Methods())
        {
            // only doing methods right now...
            Method *old = static_cast<Method*>(m);
            MethodSignature *m1 = new MethodSignature(*old->Signature());
            m1->SetContainer(rv);
            Method* nm = new Method(m1, old->Flags());
            rv->Add(nm);
        }
        return rv;
    }
    return nullptr;
}

PELib::eFindType PELib::Find(std::string path, Method **result, const std::vector<Type*>& args, Type* rv, std::deque<Type*>* generics, AssemblyDef *assembly, bool matchArgs)
{
    if (path.size() && path[0] == '[')
    {
        size_t npos = path.find(']');
        if (npos != std::string::npos)
        {
            std::string assemblyName = path.substr(1, npos - 1);
            path = path.substr(npos + 1);
            assembly = FindAssembly(assemblyName);
        }
    }
    std::vector<std::string> split;
    SplitPath(split, path);
    std::vector<Method*> foundMethod;

    for (auto a : assemblyRefs_)
    {
        //            if (a->InAssemblyRef())
        {
            if (!assembly || assembly == a)
            {
                size_t n = 0;
                DataContainer* dc = a->FindContainer(split, n, generics, true);
                if (dc)
                {
                    if (n == split.size() - 1 &&
                        (typeid(*dc) == typeid(Class) || typeid(*dc) == typeid(Enum) || typeid(*dc) == typeid(AssemblyDef)))
                    {
                        for (auto cc : dc->Methods())
                        {
                            Method* method = static_cast<Method*>(cc);
                            if (method->Signature()->Name() == split[n])
                                foundMethod.push_back(method);
                        }
                    }
                }
            }
        }
    }
    for (auto u : usingList_)
    {
        size_t n = 0;
        DataContainer* dc = u->FindContainer(split, n, generics);
        if (dc)
        {
            if ((n == split.size() - 1 && typeid(*dc) == typeid(Class)) || typeid(*dc) == typeid(Enum))
            {
                for (auto cc : dc->Methods())
                {
                    Method* method = static_cast<Method*>(cc);
                    if (method->Signature()->Name() == split[n])
                        foundMethod.push_back(method);
                }
            }
        }
    }
    if (matchArgs)
    {
        for (auto it = foundMethod.begin(); it != foundMethod.end();)
        {
            if (!(*it)->Signature()->Matches(args) || (rv && !(*it)->Signature()->MatchesType((*it)->Signature()->ReturnType(), rv)))
                it = foundMethod.erase(it);
            else
                ++it;
        }
    }
    if (foundMethod.size() > 1)
    {
        for (auto it = foundMethod.begin(); it != foundMethod.end();)
        {
            if ((*it)->Signature()->Flags() & MethodSignature::Vararg)
                it = foundMethod.erase(it);
            else
                ++it;
        }
    }
    if (foundMethod.size() == 0)
    {
        Method* method = FindPInvoke(path);
        if (method)
        {
            *result = method;
            return s_method;
        }
        return s_notFound;
    }
    else if (foundMethod.size() > 1)
    {
        return s_ambiguous;
    }
    else
    {
        *result = foundMethod[0];
        return s_method;
    }
}
bool PELib::ILSrcDumpHeader(Stream& s)
{
    s.Out() << ".corflags " << corFlags_ << std::endl << std::endl;
    for (std::list<AssemblyDef*>::const_iterator it = assemblyRefs_.begin(); it != assemblyRefs_.end(); ++it)
    {
        (*it)->ILHeaderDump(s);
    }
    s.Out() << std::endl;
    return true;
}
bool PELib::ILSrcDumpFile(Stream& s)
{
    WorkingAssembly()->ILSrcDump(s);
    for (auto sig : pInvokeSignatures_)
    {
        sig.second->ILSrcDump(s);
    }
    return true;
}

bool PELib::ILSrcDump(const std::string& file)
{
    Stream s(new std::fstream(file.c_str(), std::ios::in | std::ios::out | std::ios::trunc | std::ios::in) );
    const bool res = ILSrcDumpHeader(s) && ILSrcDumpFile(s);
    static_cast<std::fstream&>( s.Out() ).close();
    return res;
}


AssemblyDef* PELib::FindAssembly(const std::string& assemblyName) const
{
    for (std::list<AssemblyDef*>::const_iterator it = assemblyRefs_.begin(); it != assemblyRefs_.end(); ++it)
    {
        if ((*it)->Name() == assemblyName)
            return *it;
    }
    return nullptr;
}


bool PELib::DumpPEFile(std::string file, bool isexe, bool isgui)
{
    int n = 1;
    WorkingAssembly()->Number(n);  // give initial PE Indexes for field resolution..

    PEWriter peWriter(isexe, isgui, WorkingAssembly()->SNKFile());

    // RK: Unhandled Exception on Mono 3 and 5:
    // System.TypeLoadException: Could not load type 'Module' from assembly 'test6, Version=0.0.0.0, Culture=neutral,
    // PublicKeyToken=null'.
    // [ERROR] FATAL UNHANDLED EXCEPTION: System.TypeLoadException: Could not load type 'Module' from assembly 'test6,
    // Version=0.0.0.0, Culture=neutral, PublicKeyToken=null'.
    size_t moduleIndex = peWriter.HashString("<Module>"); // RK fix: "<Module>" instead of "Module" fixes the issue

    TypeDefOrRef typeDef(TypeDefOrRef::TypeDef, 0);
    TableEntryBase* table = new TypeDefTableEntry(0, moduleIndex, 0, typeDef, 1, 1);
    peWriter.AddTableEntry(table);

    int baseTypes = 0;
    WorkingAssembly()->BaseTypes(baseTypes);
    if (baseTypes)
    {
        MSCorLibAssembly();
    }
    size_t systemIndex = 0;
    size_t objectIndex = 0;
    size_t valueIndex = 0;
    size_t enumIndex = 0;
    if (baseTypes)
    {
        systemIndex = peWriter.HashString("System");
        if (baseTypes & DataContainer::basetypeObject)
        {
            objectIndex = peWriter.HashString("Object");
        }
        if (baseTypes & DataContainer::basetypeValue)
        {
            valueIndex = peWriter.HashString("ValueType");
        }
        if (baseTypes & DataContainer::basetypeEnum)
        {
            enumIndex = peWriter.HashString("Enum");
        }
    }
    Stream s(&peWriter,this);

    for (AssemblyDef * assemblyRef : assemblyRefs_)
    {
        assemblyRef->PEHeaderDump(s);
    }

    if (baseTypes)
    {
        AssemblyDef* mscorlibAssembly = MSCorLibAssembly();
        int assemblyIndex = mscorlibAssembly->PEIndex();
        ResolutionScope rs(ResolutionScope::AssemblyRef, assemblyIndex);
        if (baseTypes & DataContainer::basetypeObject)
        {
            Resource* result = nullptr;
            table = new TypeRefTableEntry(rs, objectIndex, systemIndex);
            objectIndex = peWriter.AddTableEntry(table);
            Find("[mscorlib]System::Object", &result);
            if (result)
                static_cast<Class*>(result)->PEIndex(objectIndex);
        }
        if (baseTypes & DataContainer::basetypeValue)
        {
            Resource* result = nullptr;
            table = new TypeRefTableEntry(rs, valueIndex, systemIndex);
            valueIndex = peWriter.AddTableEntry(table);
            Find("[mscorlib]System::ValueType", &result);
            if (result)
                static_cast<Class*>(result)->PEIndex(valueIndex);
        }
        if (baseTypes & DataContainer::basetypeEnum)
        {
            Resource* result = nullptr;
            table = new TypeRefTableEntry(rs, enumIndex, systemIndex);
            enumIndex = peWriter.AddTableEntry(table);
            Find("[mscorlib]System::Enum", &result);
            if (result)
                static_cast<Class*>(result)->PEIndex(enumIndex);
        }
        peWriter.SetBaseClasses(objectIndex, valueIndex, enumIndex, systemIndex);
    }
    size_t npos = file.find_last_of("\\");
    if (npos != std::string::npos && npos != file.size() - 1)
        file = file.substr(npos + 1);
    size_t nameIndex = peWriter.HashString(file);
    peWriter.CreateGuid(moduleGuid);
    size_t guidIndex = peWriter.HashGUID(moduleGuid);
    table = new ModuleTableEntry(nameIndex, guidIndex);
    peWriter.AddTableEntry(table);

    for (auto signature : pInvokeSignatures_)
    {
        signature.second->PEDump(s);
    }
    bool rv = WorkingAssembly()->PEDump(s);
    WorkingAssembly()->Compile(s);

    std::fstream out(file.c_str(), std::ios::in | std::ios::out | std::ios::trunc | std::ios::binary);
    peWriter.WriteFile(GetCorFlags(), out);
    return rv;
}

AssemblyDef* PELib::MSCorLibAssembly()
{
    // [mscorlib]System.ParamArrayAttribute
    // System. + typeNames_[tp_]
    AssemblyDef* mscorlibAssembly = FindAssembly("mscorlib");
    if (mscorlibAssembly == nullptr)
    {
        mscorlibAssembly = AddExternalAssembly("mscorlib");
        // create mscorlib and add most importand things
        Namespace* system = new Namespace("System");
        mscorlibAssembly->Add(system);
        Class* object = new Class("Object",Qualifiers::Public,-1,-1);
        system->Add(object);
        Class* value = new Class("ValueType",Qualifiers::Public,-1,-1);
        value->Extends(object);
        system->Add(value);
        Class* enum_ = new Class("Enum",Qualifiers::Public,-1,-1);
        enum_->Extends(value);
        system->Add(enum_);
    }
    return mscorlibAssembly;
}


}  // namespace DotNetPELib
