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

#include "Operand.h"
#include "Stream.h"
#include "PEWriter.h"
#include "Operand.h"
#include "Value.h"
#include "Instruction.h"
#include "PELibError.h"
#include <iomanip>
#include <cassert>
#ifdef QT_CORE_LIB
#include <QString>
#endif

namespace DotNetPELib
{
std::string Operand::EscapedString() const
{
    bool doit = false;
    for (unsigned i : stringValue_)
        if (i < 32 || i > 126 || i == '\\' || i == '"')
        {
            doit = true;
            break;
        }
    if (doit)
    {
        std::string retVal;
        for (unsigned i : stringValue_)
        {
            i &= 0xff;
            if (i < 32)
            {
                switch (i)
                {

                    case '\a':
                        i = 'a';
                        break;
                    case '\b':
                        i = 'b';
                        break;
                    case '\f':
                        i = 'f';
                        break;
                    case '\n':
                        i = 'n';
                        break;
                    case '\r':
                        i = 'r';
                        break;
                    case '\v':
                        i = 'v';
                        break;
                    case '\t':
                        i = 't';
                        break;
                    default:
                        break;
                }
                if (i < 32)
                {

                    retVal += "\\0";
                    retVal += ((i / 8) + '0');
                    retVal += ((i & 7) + '0');
                }
                else
                {
                    retVal += "\\";
                    retVal += (char)i;
                }
            }
            else if (i == '"' || i == '\\')
            {
                retVal += "\\";
                retVal += (char)i;
            }
            else if (i > 126)
            {
                retVal += "\\";
                retVal += ((i / 64) + '0');
                retVal += (((i / 8) & 7) + '0');
                retVal += ((i & 7) + '0');
            }
            else
                retVal += i;
        }
        return retVal;
    }
    return stringValue_;
}
bool Operand::isnanorinf() const
{
    // little endian architectures only
    longlong check;
    *(double*)&check = floatValue_;
    if (check == 0)
        return false;
    // infinity or nan, or denormal... (exponent all ones or all zeros)
    check >>= 32;
    check &= 0x7ff00000;
    return check == 0x7ff00000 || check == 0;
}
Operand::Operand(Value* V):type_(t_value), refValue_(V), property_(false), sz_(i8), intValue_(0), floatValue_(0)
{
    assert( V != 0 );
}

bool Operand::ILSrcDump(Stream& peLib) const
{
    switch (type_)
    {
        case t_none:  // no operand, nothing to display
            break;
        case t_value:
            refValue_->ILSrcDump(peLib);
            break;
        case t_int:
            peLib.Out() << intValue_;
            break;
        case t_real:
            if (isnanorinf())
            {
                Byte buf[8];
                int sz1, i;
                if (sz_ == r4)
                {
                    sz1 = 4;
                    *(float*)buf = floatValue_;
                }
                else
                {
                    sz1 = 8;
                    *(double*)buf = floatValue_;
                }

                peLib.Out() << "(" << std::hex;
                for (i = 0; i < sz1; i++)
                {
                    peLib.Out() << std::setw(2) << std::setfill('0') << (int)buf[i] << " ";
                }
                peLib.Out() << ")" << std::dec;
            }
            else
            {
                peLib.Out() << floatValue_;
            }
            break;
        case t_string:
            peLib.Out() << "\"" << EscapedString() << "\"";
            break;
        case t_label:
            peLib.Out() << stringValue_;
            break;
    }
    return true;
}

size_t Operand::Render(Stream& peLib, int opcode, int operandType, Byte* result)
{
    int sz = 0;
    switch (type_)
    {
        case t_none:  // no operand, nothing to display
            break;
        case t_label:
            // shouldn't be rendered...
            break;
        case t_value:
            sz = refValue_->Render(peLib, opcode, operandType, result);
            break;
        case t_int:
            switch (operandType)
            {
                case Instruction::o_immed1:
                    result[sz++] = intValue_;
                    break;
                case Instruction::o_immed4:
                    *(int*)(result) = intValue_;
                    sz += 4;
                    break;
                case Instruction::o_immed8:
                    *(longlong*)result = intValue_;
                    sz += 8;
                    break;
            }
            break;
        case t_real:
            switch (operandType)
            {
                case Instruction::o_float4:
                    *(float*)result = floatValue_;
                    sz += 4;
                    break;
                case Instruction::o_float8:
                    *(double*)result = floatValue_;
                    sz += 8;
                    break;
            }
            break;
        case t_string:
        {
#ifdef QT_CORE_LIB
            QString str = QString::fromUtf8(stringValue_.c_str());
            str.replace("\\0",'\0');
            int size = str.size()+1;
            wchar_t* buf = new wchar_t[size];
            str.toWCharArray(buf);
            buf[size-1] = 0;
#else
            std::string str = stringValue_;
            int size = str.size();
            if( str[size-1] == '0' && str[size-2] == '\\' )
            {
                str[size-2] = 0;
                // size includes "\0"
                size--;
            }else
                size++;
            wchar_t* buf = new wchar_t[size];
            for (int i = 0; i < size; i++)
                buf[i] = str.c_str()[i];

#endif
            size_t usIndex = peLib.PEOut().HashUS(buf,size); // original was std::wstring which choped the explicit \0
            *(int*)(result) = usIndex | (0x70 << 24);
            sz += 4;
            delete[] buf;
            break;
        }
    }
    return sz;
}
}  // namespace DotNetPELib
