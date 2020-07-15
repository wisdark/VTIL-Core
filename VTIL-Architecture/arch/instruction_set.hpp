// Copyright (c) 2020 Can Boluk and contributors of the VTIL Project   
// All rights reserved.   
//    
// Redistribution and use in source and binary forms, with or without   
// modification, are permitted provided that the following conditions are met: 
//    
// 1. Redistributions of source code must retain the above copyright notice,   
//    this list of conditions and the following disclaimer.   
// 2. Redistributions in binary form must reproduce the above copyright   
//    notice, this list of conditions and the following disclaimer in the   
//    documentation and/or other materials provided with the distribution.   
// 3. Neither the name of VTIL Project nor the names of its contributors
//    may be used to endorse or promote products derived from this software 
//    without specific prior written permission.   
//    
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE   
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE  
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE   
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR   
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF   
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS   
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN   
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)   
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE  
// POSSIBILITY OF SUCH DAMAGE.        
//
#pragma once
#include <vector>
#include <vtil/math>
#include <array>
#include "instruction_desc.hpp"

namespace vtil
{
    // Instructions have two general restrictions:
    // - They may only perform a single write operation whether that is 
    //   a register or a memory location.
    // - They cannot access more than one memory location.
    //
    namespace ins
    {
        using op = math::operator_id;
        using o = operand_type;

        //  -- Data/Memory instructions
        //
        //    MOV        Reg,    Reg/Imm                                     | OP1 = OP2
        //    MOVSX      Reg,    Reg/Imm                                     | OP1 = SX(OP2)
        //    STR        Reg,    Imm,      Reg/Imm                           | [OP1+OP2] <= OP3
        //    LDD        Reg,    Reg,      Imm                               | OP1 <= [OP2+OP3]
        //
        /*------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
        /*                                          [Name]        [Operands...]                                     [ASizeOp]   [Volatile]  [Operator]                [BranchOps] [MemOps]    */
        inline const instruction_desc mov =         { "mov",      { o::write,    o::read_any                   },   2,          false,      {},                       {},         {}          };
        inline const instruction_desc movsx =       { "movsx",    { o::write,    o::read_any                   },   2,          false,      {},                       {},         {}          };
        inline const instruction_desc str =         { "str",      { o::read_reg, o::read_imm,     o::read_any  },   3,          false,      {},                       {},         { 1, true } };
        inline const instruction_desc ldd =         { "ldd",      { o::write,    o::read_reg,     o::read_imm  },   1,          false,      {},                       {},         { 2, false }};
        /*------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

        //    -- Arithmetic instructions
        //
        //    NEG        Reg                                                 | OP1 = -OP1
        //    ADD        Reg,    Reg/Imm                                     | OP1 = OP1 + OP2
        //    SUB        Reg,    Reg/Imm                                     | OP1 = OP1 - OP2
        //    MUL        Reg,    Reg/Imm                                     | OP1 = OP1 * OP2
        //    MULHI      Reg,    Reg/Imm                                     | OP1 = [OP1 * OP2]>>N
        //    IMUL       Reg,    Reg/Imm                                     | OP1 = OP1 * OP2         (Signed)
        //    IMULHI     Reg,    Reg/Imm                                     | OP1 = [OP1 * OP2]>>N    (Signed)
        //    DIV        Reg,    Reg/Imm    Reg/Imm                          | OP1 = [OP2:OP1] / OP3         
        //    REM        Reg,    Reg/Imm    Reg/Imm                          | OP1 = [OP2:OP1] % OP3         
        //    IDIV       Reg,    Reg/Imm    Reg/Imm                          | OP1 = [OP2:OP1] / OP3   (Signed)
        //    IREM       Reg,    Reg/Imm    Reg/Imm                          | OP1 = [OP2:OP1] % OP3   (Signed)
        //
        /*------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
        /*                                          [Name]        [Operands...]                                     [ASizeOp]   [Volatile]  [Operator]               [BranchOps] [MemOps]     */
        inline const instruction_desc neg =         { "neg",      { o::readwrite                                 }, 1,          false,      op::negate,               {},         {}          };
        inline const instruction_desc add =         { "add",      { o::readwrite,  o::read_any                   }, 1,          false,      op::add,                  {},         {}          };
        inline const instruction_desc sub =         { "sub",      { o::readwrite,  o::read_any                   }, 1,          false,      op::subtract,             {},         {}          };
        inline const instruction_desc mul =         { "mul",      { o::readwrite,  o::read_any                   }, 1,          false,      op::umultiply,            {},         {}          };
        inline const instruction_desc imul =        { "imul",     { o::readwrite,  o::read_any                   }, 1,          false,      op::multiply,             {},         {}          };
        inline const instruction_desc mulhi =       { "mulhi",    { o::readwrite,  o::read_any                   }, 1,          false,      op::umultiply_high,       {},         {}          };
        inline const instruction_desc imulhi =      { "imulhi",   { o::readwrite,  o::read_any                   }, 1,          false,      op::multiply_high,        {},         {}          };
        inline const instruction_desc div =         { "div",      { o::readwrite,  o::read_any,     o::read_any  }, 1,          false,      op::udivide,              {},         {}          };
        inline const instruction_desc idiv =        { "idiv",     { o::readwrite,  o::read_any,     o::read_any  }, 1,          false,      op::divide,               {},         {}          };
        inline const instruction_desc rem =         { "rem",      { o::readwrite,  o::read_any,     o::read_any  }, 1,          false,      op::uremainder,           {},         {}          };
        inline const instruction_desc irem =        { "irem",     { o::readwrite,  o::read_any,     o::read_any  }, 1,          false,      op::remainder,            {},         {}          };
        /*------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
    
        //  -- Bitwise instructions
        //
        //    POPCNT     Reg                                                 | OP1 = popcnt OP1
        //    BSF        Reg                                                 | OP1 = OP1 ? BitScanForward OP1 + 1 : 0
        //    BSR        Reg                                                 | OP1 = OP1 ? BitScanReverse OP1 + 1 : 0
        //    NOT        Reg                                                 | OP1 = ~OP1
        //    SHR        Reg,    Reg/Imm                                     | OP1 >>= OP2
        //    SHL        Reg,    Reg/Imm                                     | OP1 <<= OP2
        //    XOR        Reg,    Reg/Imm                                     | OP1 ^= OP2
        //    OR         Reg,    Reg/Imm                                     | OP1 |= OP2
        //    AND        Reg,    Reg/Imm                                     | OP1 &= OP2
        //    ROR        Reg,    Reg/Imm                                     | OP1 = (OP1>>OP2) | (OP1<<(N-OP2))
        //    ROL        Reg,    Reg/Imm                                     | OP1 = (OP1<<OP2) | (OP1>>(N-OP2))
        //
        /*------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
        /*                                          [Name]        [Operands...]                                     [ASizeOp]   [Volatile]  [Operator]               [BranchOps] [MemOps]     */
        inline const instruction_desc popcnt =      { "popcnt",   { o::readwrite                                 }, 1,          false,      op::popcnt,              {},          {}          };
        inline const instruction_desc bsf =         { "bsf",      { o::readwrite                                 }, 1,          false,      op::bitscan_fwd,         {},          {}          };
        inline const instruction_desc bsr =         { "bsr",      { o::readwrite                                 }, 1,          false,      op::bitscan_rev,         {},          {}          };
        inline const instruction_desc bnot =        { "not",      { o::readwrite                                 }, 1,          false,      op::bitwise_not,         {},          {}          };
        inline const instruction_desc bshr =        { "shr",      { o::readwrite,  o::read_any                   }, 1,          false,      op::shift_right,         {},          {}          };
        inline const instruction_desc bshl =        { "shl",      { o::readwrite,  o::read_any                   }, 1,          false,      op::shift_left,          {},          {}          };
        inline const instruction_desc bxor =        { "xor",      { o::readwrite,  o::read_any                   }, 1,          false,      op::bitwise_xor,         {},          {}          };
        inline const instruction_desc bor =         { "or",       { o::readwrite,  o::read_any                   }, 1,          false,      op::bitwise_or,          {},          {}          };
        inline const instruction_desc band =        { "and",      { o::readwrite,  o::read_any                   }, 1,          false,      op::bitwise_and,         {},          {}          };
        inline const instruction_desc bror =        { "ror",      { o::readwrite,  o::read_any                   }, 1,          false,      op::rotate_right,        {},          {}          };
        inline const instruction_desc brol =        { "rol",      { o::readwrite,  o::read_any                   }, 1,          false,      op::rotate_left,         {},          {}          };
        /*------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
    
        //  -- Conditional instructions
        //
        //    TG         Reg,    Reg/Imm,    Reg/Imm                         | OP1 = OP2   >    OP3
        //    TGE        Reg,    Reg/Imm,    Reg/Imm                         | OP1 = OP2   >=   OP3
        //    TE         Reg,    Reg/Imm,    Reg/Imm                         | OP1 = OP2   ==   OP3
        //    TNE        Reg,    Reg/Imm,    Reg/Imm                         | OP1 = OP2   !=   OP3
        //    TL         Reg,    Reg/Imm,    Reg/Imm                         | OP1 = OP2   <    OP3
        //    TLE        Reg,    Reg/Imm,    Reg/Imm                         | OP1 = OP2   <=   OP3
        //    TUG        Reg,    Reg/Imm,    Reg/Imm                         | OP1 = OP2   u>   OP3
        //    TUGE       Reg,    Reg/Imm,    Reg/Imm                         | OP1 = OP2   u>=  OP3
        //    TUL        Reg,    Reg/Imm,    Reg/Imm                         | OP1 = OP2   u<   OP3
        //    TULE       Reg,    Reg/Imm,    Reg/Imm                         | OP1 = OP2   u<=  OP3
        //    IFS        Reg,    Reg/Imm,    Reg/Imm                         | OP1 = OP2 ? OP3 : 0
        //
        /*------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
        /*                                          [Name]        [Operands...]                                     [ASizeOp]   [Volatile]  [Operator]               [BranchOps] [MemOps]     */
        inline const instruction_desc tg =          { "tg",       { o::write,      o::read_any,     o::read_any  }, 1,          false,      op::greater,             {},          {}          };
        inline const instruction_desc tge =         { "tge",      { o::write,      o::read_any,     o::read_any  }, 1,          false,      op::greater_eq,          {},          {}          };
        inline const instruction_desc te =          { "te",       { o::write,      o::read_any,     o::read_any  }, 1,          false,      op::equal,               {},          {}          };
        inline const instruction_desc tne =         { "tne",      { o::write,      o::read_any,     o::read_any  }, 1,          false,      op::not_equal,           {},          {}          };
        inline const instruction_desc tle =         { "tle",      { o::write,      o::read_any,     o::read_any  }, 1,          false,      op::less_eq,             {},          {}          };
        inline const instruction_desc tl =          { "tl",       { o::write,      o::read_any,     o::read_any  }, 1,          false,      op::less,                {},          {}          };
        inline const instruction_desc tug =         { "tug",      { o::write,      o::read_any,     o::read_any  }, 1,          false,      op::ugreater,            {},          {}          };
        inline const instruction_desc tuge =        { "tuge",     { o::write,      o::read_any,     o::read_any  }, 1,          false,      op::ugreater_eq,         {},          {}          };
        inline const instruction_desc tule =        { "tule",     { o::write,      o::read_any,     o::read_any  }, 1,          false,      op::uless_eq,            {},          {}          };
        inline const instruction_desc tul =         { "tul",      { o::write,      o::read_any,     o::read_any  }, 1,          false,      op::uless,               {},          {}          };
        inline const instruction_desc ifs =         { "ifs",      { o::write,      o::read_any,     o::read_any  }, 3,          false,      op::value_if,            {},          {}          };
        /*------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
    
        //  -- Control flow instructions
        //                                                            
        //    JS         Reg,    Reg/Imm,    Reg/Imm                        | Jumps to OP1 ? OP2 : OP3, continues virtual execution
        //    JMP        Reg/Imm                                            | Jumps to OP1, continues virtual execution
        //    VEXIT      Reg/Imm                                            | Jumps to OP1, continues real execution
        //    VXCALL     Reg/Imm                                            | Calls into OP1, pauses virtual execution until the call returns
        //
        /*------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
        /*                                          [Name]        [Operands...]                                     [ASizeOp]   [Volatile]  [Operator]               [BranchOps] [MemOps]     */
        inline const instruction_desc js =         { "js",        { o::read_reg,   o::read_any,     o::read_any  }, 2,          false,      {},                      { 2, 3 },    {}          };
        inline const instruction_desc jmp =        { "jmp",       { o::read_any                                  }, 1,          false,      {},                      { 1 },       {}          };
        inline const instruction_desc vexit =      { "vexit",     { o::read_any                                  }, 1,          false,      {},                      { -1 },      {}          };
        inline const instruction_desc vxcall =     { "vxcall",    { o::read_any                                  }, 1,          false,      {},                      { -1 },      {}          };
        /*------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

        //    -- Special instructions
        //
        //    NOP                                                           | Placeholder
        //    VEMIT      Imm                                                | Emits the opcode as is to the final instruction stream.
        //    VPINR      Reg                                                | Pins the register for read                 // UD? can be used as a wildcard for all physical registers.
        //    VPINW      Reg                                                | Pins the register for write                // UD? can be used as a wildcard for all physical registers.
        //    VPINRM     Reg,    Imm                                        | Pins the qword @ memory location for read  // UD? can be used as a wildcard for all public memory effectively rendering it an SFENCE.
        //    VPINWM     Reg,    Imm                                        | Pins the qword @ memory location for write // UD? can be used as a wildcard for all public memory effectively rendering it an LFENCE.
        //
        /*------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
        /*                                          [Name]        [Operands...]                                     [ASizeOp]   [Volatile]  [Operator]               [BranchOps] [MemOps]     */
        inline const instruction_desc nop =        { "nop",       {                                             },  0,          false,      {},                      {},         {}           };
        inline const instruction_desc vemit =      { "vemit",     { o::read_imm                                 },  1,          true,       {},                      {},         {}           };
        inline const instruction_desc vpinr =      { "vpinr",     { o::read_reg                                 },  1,          true,       {},                      {},         {}           };
        inline const instruction_desc vpinw =      { "vpinw",     { o::write                                    },  1,          true,       {},                      {},         {}           };
        inline const instruction_desc vpinrm =     { "vpinrm",    { o::read_reg,   o::read_imm,                 },  0,          true,       {},                      {},         { 1, false } };
        inline const instruction_desc vpinwm =     { "vpinwm",    { o::read_reg,   o::read_imm                  },  0,          true,       {},                      {},         { 1, true }  };
        /*------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
    };

    // List of all instructions.
    //
    static const auto& get_instruction_list()
    {
        static std::array instruction_list =
        {
            &ins::mov, &ins::movsx, &ins::str, &ins::ldd, &ins::ifs, &ins::neg, &ins::add, &ins::sub, &ins::mul, &ins::imul,
            &ins::mulhi, &ins::imulhi, &ins::div, &ins::idiv, &ins::rem, &ins::irem, &ins::popcnt, &ins::bsf, &ins::bsr,
            &ins::bnot, &ins::bshr, &ins::bshl,&ins::bxor, &ins::bor, &ins::band, &ins::bror, &ins::brol, &ins::tg,
            &ins::tge, &ins::te, &ins::tne, &ins::tle, &ins::tl, &ins::tug, &ins::tuge, &ins::tule, &ins::tul, &ins::js,
            &ins::jmp, &ins::vexit, &ins::vxcall, &ins::nop, &ins::vemit, &ins::vpinr, &ins::vpinw, &ins::vpinrm,
            &ins::vpinwm
        };
        return instruction_list;
    }
};