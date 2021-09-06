#ifndef DotNetPELib_CALLBACK
#define DotNetPELib_CALLBACK

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

namespace DotNetPELib
{
    class AssemblyDef;
    class Namespace;
    class Class;
    class Enum;
    class Method;
    class Field;
    class Property;

    ///** The callback structure is passed to 'traverse'... it holds callbacks
    // This is a visitor called while traversing the tree
    class Callback
    {
    public:
        virtual ~Callback() { }

        virtual bool EnterAssembly(const AssemblyDef *) { return true; }
        virtual bool ExitAssembly(const AssemblyDef *) { return true; }
        virtual bool EnterNamespace(const Namespace *) { return true; }
        virtual bool ExitNamespace(const Namespace *) { return true; }
        virtual bool EnterClass(const Class *) { return true; }
        virtual bool ExitClass(const Class *) { return true; }
        virtual bool EnterEnum(const Enum *) { return true; }
        virtual bool ExitEnum(const Enum *) { return true; }
        virtual bool EnterMethod(const Method *) { return true; }
        virtual bool EnterField(const Field *) { return true; }
        virtual bool EnterProperty(const Property *) { return true; }
    };
}

#endif // DotNetPELib_CALLBACK

