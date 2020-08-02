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

// Furthermore, the following pieces of software have additional copyrights
// licenses, and/or restrictions:
//
// |--------------------------------------------------------------------------|
// | File name               | Link for further information                   |
// |-------------------------|------------------------------------------------|
// | amd64/*                 | https://github.com/aquynh/capstone/            |
// |                         | https://github.com/keystone-engine/keystone/   |
// |--------------------------------------------------------------------------|
//
#pragma once
#include <map>
#include <tuple>
#include <string>
#include "../../io/asserts.hpp"
#include "amd64_disassembler.hpp"
#include "../register_mapping.hpp"

namespace vtil::amd64
{
    // List of all physical registers and the base registers they map to <0> at offset <1> of size <2>.
	//
	static constexpr register_map<x86_reg, X86_REG_ENDING> registers =
	{
		{
            /* [Instance]           [Base]       [Offset] [Size]  */
            { X86_REG_RAX,		{ X86_REG_RAX,		0,		8	} },
            { X86_REG_EAX,		{ X86_REG_RAX,		0,		4	} },
            { X86_REG_AX,		{ X86_REG_RAX,		0,		2	} },
            { X86_REG_AH,		{ X86_REG_RAX,		1,		1	} },
            { X86_REG_AL,		{ X86_REG_RAX,		0,		1	} },
                    
            { X86_REG_RBX,		{ X86_REG_RBX,		0,		8	} },
            { X86_REG_EBX,		{ X86_REG_RBX,		0,		4	} },
            { X86_REG_BX,		{ X86_REG_RBX,		0,		2	} },
            { X86_REG_BH,		{ X86_REG_RBX,		1,		1	} },
            { X86_REG_BL,		{ X86_REG_RBX,		0,		1	} },
                    
            { X86_REG_RCX,		{ X86_REG_RCX,		0,		8	} },
            { X86_REG_ECX,		{ X86_REG_RCX,		0,		4	} },
            { X86_REG_CX,		{ X86_REG_RCX,		0,		2	} },
            { X86_REG_CH,		{ X86_REG_RCX,		1,		1	} },
            { X86_REG_CL,		{ X86_REG_RCX,		0,		1	} },
                    
            { X86_REG_RDX,		{ X86_REG_RDX,		0,		8	} },
            { X86_REG_EDX,		{ X86_REG_RDX,		0,		4	} },
            { X86_REG_DX,		{ X86_REG_RDX,		0,		2	} },
            { X86_REG_DH,		{ X86_REG_RDX,		1,		1	} },
            { X86_REG_DL,		{ X86_REG_RDX,		0,		1	} },
                    
            { X86_REG_RDI,		{ X86_REG_RDI,		0,		8	} },
            { X86_REG_EDI,		{ X86_REG_RDI,		0,		4	} },
            { X86_REG_DI,		{ X86_REG_RDI,		0,		2	} },
            { X86_REG_DIL,		{ X86_REG_RDI,		0,		1	} },
                    
            { X86_REG_RSI,		{ X86_REG_RSI,		0,		8	} },
            { X86_REG_ESI,		{ X86_REG_RSI,		0,		4	} },
            { X86_REG_SI,		{ X86_REG_RSI,		0,		2	} },
            { X86_REG_SIL,		{ X86_REG_RSI,		0,		1	} },
                    
            { X86_REG_RBP,		{ X86_REG_RBP,		0,		8	} },
            { X86_REG_EBP,		{ X86_REG_RBP,		0,		4	} },
            { X86_REG_BP,		{ X86_REG_RBP,		0,		2	} },
            { X86_REG_BPL,		{ X86_REG_RBP,		0,		1	} },
                    
            { X86_REG_RSP,		{ X86_REG_RSP,		0,		8	} },
            { X86_REG_ESP,		{ X86_REG_RSP,		0,		4	} },
            { X86_REG_SP,		{ X86_REG_RSP,		0,		2	} },
            { X86_REG_SPL,		{ X86_REG_RSP,		0,		1	} },
                    
            { X86_REG_R8,		{ X86_REG_R8,		0,		8	} },
            { X86_REG_R8D,		{ X86_REG_R8,		0,		4	} },
            { X86_REG_R8W,		{ X86_REG_R8,		0,		2	} },
            { X86_REG_R8B,		{ X86_REG_R8,		0,		1	} },
                    
            { X86_REG_R9,		{ X86_REG_R9,		0,		8	} },
            { X86_REG_R9D,		{ X86_REG_R9,		0,		4	} },
            { X86_REG_R9W,		{ X86_REG_R9,		0,		2	} },
            { X86_REG_R9B,		{ X86_REG_R9,		0,		1	} },

            { X86_REG_R10,		{ X86_REG_R10,		0,		8	} },
            { X86_REG_R10D,		{ X86_REG_R10,		0,		4	} },
            { X86_REG_R10W,		{ X86_REG_R10,		0,		2	} },
            { X86_REG_R10B,		{ X86_REG_R10,		0,		1	} },

            { X86_REG_R11,		{ X86_REG_R11,		0,		8	} },
            { X86_REG_R11D,		{ X86_REG_R11,		0,		4	} },
            { X86_REG_R11W,		{ X86_REG_R11,		0,		2	} },
            { X86_REG_R11B,		{ X86_REG_R11,		0,		1	} },

            { X86_REG_R12,		{ X86_REG_R12,		0,		8	} },
            { X86_REG_R12D,		{ X86_REG_R12,		0,		4	} },
            { X86_REG_R12W,		{ X86_REG_R12,		0,		2	} },
            { X86_REG_R12B,		{ X86_REG_R12,		0,		1	} },

            { X86_REG_R13,		{ X86_REG_R13,		0,		8	} },
            { X86_REG_R13D,		{ X86_REG_R13,		0,		4	} },
            { X86_REG_R13W,		{ X86_REG_R13,		0,		2	} },
            { X86_REG_R13B,		{ X86_REG_R13,		0,		1	} },

            { X86_REG_R14,		{ X86_REG_R14,		0,		8	} },
            { X86_REG_R14D,		{ X86_REG_R14,		0,		4	} },
            { X86_REG_R14W,		{ X86_REG_R14,		0,		2	} },
            { X86_REG_R14B,		{ X86_REG_R14,		0,		1	} },

            { X86_REG_R15,		{ X86_REG_R15,		0,		8	} },
            { X86_REG_R15D,		{ X86_REG_R15,		0,		4	} },
            { X86_REG_R15W,		{ X86_REG_R15,		0,		2	} },
            { X86_REG_R15B,		{ X86_REG_R15,		0,		1	} },

            { X86_REG_EFLAGS,	{ X86_REG_EFLAGS,	0,		8	} },
		}
	};

	// Converts the enum into human-readable format.
	//
	static const char* name( uint32_t _reg ) { return cs_reg_name( get_cs_handle(), _reg ); }
};