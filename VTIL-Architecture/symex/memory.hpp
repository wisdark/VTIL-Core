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
#include <vtil/utility>
#include <vtil/math>
#include "pointer.hpp"
#include "variable.hpp"
#include "../arch/register_desc.hpp"

namespace vtil::symbolic
{
	// Strictness of pointers describing what happens when the pointer could 
	// not be resolved based on the previous write operations.
	//
	enum class memory_type
	{
		// Will generate a variable of the result of dereferencing 
		// upon default construction.
		//
		free,

		// Will generate an undefined variable upon default construction.
		//
		relaxed,

		// Will throw an exception upon default construction.
		//
		strict
	};

	// Declaration of symbolic memory type using sinkhole.
	//
	using memory = sinkhole<pointer, expression::reference, pointer::make_weak>;
	
	// Creates a symbolic memory of the given type.
	//
	static memory create_memory( memory_type type )
	{
		switch ( type )
		{
			case memory_type::free:
				return { [ ] ( const pointer& ptr, bitcnt_t size ) -> expression::reference { return make_memory_ex( ptr, size ); } };
			case memory_type::relaxed:
				return { [ ] ( const pointer& ptr, bitcnt_t size ) -> expression::reference { return make_undefined_ex( size ); } };
			default:
				return { [ ] ( const pointer& ptr, bitcnt_t size ) -> expression::reference { unreachable(); return nullptr; } };
		}
	}
};