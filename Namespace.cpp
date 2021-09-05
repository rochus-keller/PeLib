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

#include "Namespace.h"
#include "PELibError.h"
#include "Stream.h"
#include "PEWriter.h"
namespace DotNetPELib
{

bool Namespace::ILSrcDump(Stream& peLib) const
{

    peLib.Out() << ".namespace '" << name_ << "' {" << std::endl;
    DataContainer::ILSrcDump(peLib);
    peLib.Out() << "}" << std::endl;
    return true;
}

std::string Namespace::ReverseName(DataContainer* child)
{
    std::string rv;
    if (child->Parent())
    {
        if (child->Parent()->Parent())
            rv = ReverseName(child->Parent()) + ".";
        rv += child->Name();
    }
    return rv;
}
bool Namespace::PEDump(Stream& peLib)
{
    if (!InAssemblyRef() || !PEIndex())
    {
        std::string fullName = ReverseName(this);
        peIndex_ = peLib.PEOut().HashString(fullName);
    }
    if (!InAssemblyRef())
        DataContainer::PEDump(peLib);
    return true;
}
}  // namespace DotNetPELib
