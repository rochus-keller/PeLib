#ifndef DotNetPELib_INSTRUCTION
#define DotNetPELib_INSTRUCTION

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

#include <PeLib/Resource.h>
#include <map>
#include <list>

namespace DotNetPELib
{
    class Operand;
    class PELib;
    class Type;
    class Stream;
    typedef unsigned char Byte; /* 1 byte */

    /* a CIL instruction */
    class Instruction : public Resource
    {
    public:

        // names of opcodes
        enum iop
        {
            ///** should never occur
            i_unknown,
            ///** This instruction is a placeholder for a label
            i_label,
            ///** This instruction is a placeholder for a comment
            i_comment,
            ///** This instruction is an SEH specifier
            i_SEH,
            ///** actual CIL instructions start here
            i_add, i_add_ovf, i_add_ovf_un, i_and, i_arglist, i_beq, i_beq_s, i_bge,
            i_bge_s, i_bge_un, i_bge_un_s, i_bgt, i_bgt_s, i_bgt_un, i_bgt_un_s, i_ble,
            i_ble_s, i_ble_un, i_ble_un_s, i_blt, i_blt_s, i_blt_un, i_blt_un_s, i_bne_un,
            i_bne_un_s, i_box, i_br, i_br_s, i_break, i_brfalse, i_brfalse_s, i_brinst,
            i_brinst_s, i_brnull, i_brnull_s, i_brtrue, i_brtrue_s, i_brzero, i_brzero_s, i_call,
            i_calli, i_callvirt, i_castclass, i_ceq, i_cgt, i_cgt_un, i_ckfinite, i_clt,
            i_clt_un, i_constrained_, i_conv_i, i_conv_i1, i_conv_i2, i_conv_i4, i_conv_i8, i_conv_ovf_i,
            i_conv_ovf_i_un, i_conv_ovf_i1, i_conv_ovf_i1_un, i_conv_ovf_i2, i_conv_ovf_i2_un, i_conv_ovf_i4, i_conv_ovf_i4_un, i_conv_ovf_i8,
            i_conv_ovf_i8_un, i_conv_ovf_u, i_conv_ovf_u_un, i_conv_ovf_u1, i_conv_ovf_u1_un, i_conv_ovf_u2, i_conv_ovf_u2_un, i_conv_ovf_u4,
            i_conv_ovf_u4_un, i_conv_ovf_u8, i_conv_ovf_u8_un, i_conv_r_un, i_conv_r4, i_conv_r8, i_conv_u, i_conv_u1,
            i_conv_u2, i_conv_u4, i_conv_u8, i_cpblk, i_cpobj, i_div, i_div_un, i_dup,
            i_endfault, i_endfilter, i_endfinally, i_initblk, i_initobj, i_isinst, i_jmp, i_ldarg,
            i_ldarg_0, i_ldarg_1, i_ldarg_2, i_ldarg_3, i_ldarg_s, i_ldarga, i_ldarga_s, i_ldc_i4,
            i_ldc_i4_0, i_ldc_i4_1, i_ldc_i4_2, i_ldc_i4_3, i_ldc_i4_4, i_ldc_i4_5, i_ldc_i4_6, i_ldc_i4_7,
            i_ldc_i4_8, i_ldc_i4_m1, i_ldc_i4_M1, i_ldc_i4_s, i_ldc_i8, i_ldc_r4, i_ldc_r8, i_ldelem,
            i_ldelem_i, i_ldelem_i1, i_ldelem_i2, i_ldelem_i4, i_ldelem_i8, i_ldelem_r4, i_ldelem_r8, i_ldelem_ref,
            i_ldelem_u1, i_ldelem_u2, i_ldelem_u4, i_ldelem_u8, i_ldelema, i_ldfld, i_ldflda, i_ldftn,
            i_ldind_i, i_ldind_i1, i_ldind_i2, i_ldind_i4, i_ldind_i8, i_ldind_r4, i_ldind_r8, i_ldind_ref,
            i_ldind_u1, i_ldind_u2, i_ldind_u4, i_ldind_u8, i_ldlen, i_ldloc, i_ldloc_0, i_ldloc_1,
            i_ldloc_2, i_ldloc_3, i_ldloc_s, i_ldloca, i_ldloca_s, i_ldnull, i_ldobj, i_ldsfld,
            i_ldsflda, i_ldstr, i_ldtoken, i_ldvirtftn, i_leave, i_leave_s, i_localloc, i_mkrefany,
            i_mul, i_mul_ovf, i_mul_ovf_un, i_neg, i_newarr, i_newobj, i_no_, i_nop,
            i_not, i_or, i_pop, i_readonly_, i_refanytype, i_refanyval, i_rem, i_rem_un,
            i_ret, i_rethrow, i_shl, i_shr, i_shr_un, i_sizeof, i_starg, i_starg_s,
            i_stelem, i_stelem_i, i_stelem_i1, i_stelem_i2, i_stelem_i4, i_stelem_i8, i_stelem_r4, i_stelem_r8,
            i_stelem_ref, i_stfld, i_stind_i, i_stind_i1, i_stind_i2, i_stind_i4, i_stind_i8, i_stind_r4,
            i_stind_r8, i_stind_ref, i_stloc, i_stloc_0, i_stloc_1, i_stloc_2, i_stloc_3, i_stloc_s,
            i_stobj, i_stsfld, i_sub, i_sub_ovf, i_sub_ovf_un, i_switch, i_tail_, i_throw,
            i_unaligned_, i_unbox, i_unbox_any, i_volatile_, i_xor
        };
        enum iseh { seh_try, seh_catch, seh_finally, seh_fault, seh_filter, seh_filter_handler };
        enum ioperand {
            o_none, o_single, o_rel1, o_rel4, o_index1, o_index2, o_index4,
            o_immed1, o_immed4, o_immed8, o_float4, o_float8, o_switch
        };

        Instruction(iop Op, Operand *Operand = 0);

        // for now only do comments and labels and branches...
        Instruction(iop Op, const std::string& Text) : op_(Op), text_(Text), switches_(nullptr), live_(false), sehType_(seh_try), sehBegin_(false), sehCatchType_(nullptr), offset_(0) { }

        Instruction(iseh type, bool begin, Type *catchType = NULL) : op_(i_SEH), switches_(nullptr), live_(false), sehType_(type), sehBegin_(begin), sehCatchType_(catchType), offset_(0) { }

        virtual ~Instruction() { if (switches_) delete switches_; }

        ///** Get/set the opcode
        iop OpCode() const { return op_; }
        void OpCode(iop Op) { op_ = Op; }

        ///** Get the SEH Type
        int SEHType() const { return sehType_;  }

        ///** return true if it is a begin tag
        bool SEHBegin() const { return sehBegin_;  }

        ///** return the catch type
        Type *SEHCatchType() const { return sehCatchType_; }

        ///** Add a label for a SWITCH instruction
        ///** Labels MUST be added in order
        void AddCaseLabel(const std::string& label);

        ///** Get the set of case labels
        std::list<std::string> * GetSwitches() { return switches_; }

        ///** an 'empty' operand
        void NullOperand();

        ///** Get the operand (CIL instructions have either zero or 1 operands)
        Operand *GetOperand() const { return operand_; }
        void SetOperand(Operand *operand) { operand_ = operand; }

        ///** Get text, e.g. for a comment
        std::string Text() const { return text_; }

        ///** Get the label name associated with the instruction
        std::string Label() const;

        ///** The offset of the instruction within the method
        int Offset() const { return offset_; }
        void Offset(int Offset) { offset_ = Offset; }

        ///** Calculate length of instruction
        int InstructionSize();

        ///** get stack use for this instruction
        // positive means it adds to the stack, negative means it subtracts
        // 0 means it has no effect
        int StackUsage();

        ///** is a branch with a 4 byte relative offset
        int IsRel4() const { return instructions_[op_].operandType == o_rel4; }

        ///** is a branch with a 1 byte relative offset
        int IsRel1() const { return instructions_[op_].operandType == o_rel1; }

        ///** is any kind of branch
        int IsBranch() const { return IsRel1() || IsRel4(); }

        ///** Convert a 4-byte branch to a 1-byte branch
        void Rel4To1() { op_ = (iop)((int)op_ + 1); }

        ///** Is it any kind of call
        int IsCall() const {
            return op_ == i_call || op_ == i_calli || op_ == i_callvirt;
        }

        ///** Set the live flag.   We are checking for live because sometimes
        // dead code sequences can confuse the stack checking routine
        void Live(bool val) { live_ = val; }
        bool IsLive() const { return live_; }

        // internal methods and structures
        virtual bool ILSrcDump(Stream &) const;
        size_t Render(Stream& peLib, Byte *, std::map<std::string, Instruction *> &labels);

    protected:
        std::list<std::string> *switches_;
        iop op_;
        int offset_;
        int sehType_;
        bool sehBegin_;
        union {
            Operand *operand_; // for non-labels
            Type *sehCatchType_;
        };
        std::string text_; // for comments
        bool live_;
        struct InstructionName {
            const char *name;
            Byte op1;
            Byte op2;
            Byte bytes;
            Byte operandType;
            char stackUsage; // positive it adds to stack, negative it consumes stack
        };
        static InstructionName instructions_[];
    };
}

#endif // DotNetPELib_INSTRUCTION

