#ifndef STREAM_H
#define STREAM_H

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

#include <iostream>
#include <memory>
#include <map>

namespace DotNetPELib
{
class PEWriter;
class PELib;
class Resource;

class Stream
{
public:
    Stream(std::iostream*);
    Stream(PEWriter* w,PELib* l);

    std::iostream &Out() const { return *outputStream_; }
    void Swap(std::unique_ptr<std::iostream>& stream) { outputStream_.swap(stream); }

    PEWriter &PEOut() const { return *peWriter_; }
    void Find( const std::string& name, Resource** result );

    std::map<size_t, size_t> moduleRefs;
private:
    std::unique_ptr<std::iostream> outputStream_;
    PEWriter* peWriter_;
    PELib* peLib_;
};
}

#endif // STREAM_H
