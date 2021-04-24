#ifndef DotNetPELib_NAMESPACE
#define DotNetPELib_NAMESPACE

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

namespace DotNetPELib
{
    ///** a namespace
    class Namespace : public DataContainer
    {
    public:
        Namespace(const std::string& Name) : DataContainer(Name, Qualifiers(0)){}

        ///** Get the full namespace name including all parents
        std::string ReverseName(DataContainer *child);

        // internal stuff
        virtual bool ILSrcDump(PELib &) const override;
        virtual bool PEDump(PELib &) override;
        virtual void ObjOut(PELib &, int pass) const override;
        static Namespace *ObjIn(PELib &, bool definition = true);
    };

}

#endif // DotNetPELib_NAMESPACE

