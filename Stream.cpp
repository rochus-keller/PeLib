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

#include "Stream.h"
#include "PELib.h"
#ifdef QT_CORE_LIB
#include <QtDebug>
#endif
using namespace DotNetPELib;

Stream::Stream(std::iostream* s):outputStream_(s),peWriter_(0),peLib_(0)
{

}

Stream::Stream(PEWriter* w, PELib* l):peWriter_(w),peLib_(l)
{

}

void Stream::Find(const std::string& name, Resource** result)
{
    peLib_->Find(name, result, nullptr, peLib_->MSCorLibAssembly() );
}

void Stream::addMethod(Method* m)
{
    peLib_->allMethods.push_back(m);
}

