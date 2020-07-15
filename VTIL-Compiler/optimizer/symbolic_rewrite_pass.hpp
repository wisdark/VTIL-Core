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
#include <vtil/arch>
#include <shared_mutex>
#include "../common/interface.hpp"

namespace vtil::optimizer
{
	// Attempts to execute ranges of the given block in a symbolic virtual machine
	// to automatically simplify expressions created by the instructions where possible.
	//
	struct isymbolic_rewrite_pass : pass_interface<>
	{
		bool force;
		std::set<bitcnt_t> preferred_exp_sizes;
		isymbolic_rewrite_pass( bool force, const std::set<bitcnt_t>& preferred_exp_sizes = { 1, 8, 16, 32, 64 } )
			: force( force ), preferred_exp_sizes( preferred_exp_sizes ) {}

		size_t pass( basic_block* blk, bool xblock ) override;
	};

	template<bool force>
	struct symbolic_rewrite_pass : pass_interface<>
	{
		size_t pass( basic_block* blk, bool xblock = false ) override { return isymbolic_rewrite_pass{ force }.pass( blk, xblock ); }
		size_t xpass( routine* rtn ) override { return isymbolic_rewrite_pass{ force }.xpass( rtn ); }
	};
};