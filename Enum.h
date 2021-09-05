#ifndef DotNetPELib_ENUM
#define DotNetPELib_ENUM

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

#include <PeLib/Class.h>
#include <PeLib/Field.h>

namespace DotNetPELib
{
    class Stream;

    ///** A special kind of class: enum
    class Enum : public Class
    {
    public:
        Enum(const std::string& Name, Qualifiers Flags, Field::ValueSize Size) :
            size(Size), Class(Name, Flags.Flags() | Qualifiers::Value, -1, -1) {}

        ///** Add an enumeration, give it a name and a value
        // This creates the Field definition for the enumerated value
        Field *AddValue(const std::string& Name, longlong Value);

        // internal functions:
        virtual bool ILSrcDump(Stream &) const override;

        virtual bool PEDump(Stream &) override;

    protected:
        Field::ValueSize size;
    };
}

#endif // DotNetPELib_ENUM

