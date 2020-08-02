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
#include <string>
#include <optional>
#include <array>
#include <set>
#include <vtil/symex>
#include <vtil/utility>
#include "../arch/register_desc.hpp"

namespace vtil::symbolic
{
	// Declare a symbolic pointer to be used within symbolic execution context.
	//
	struct pointer : reducable<pointer>
	{
		// List of pointer bases we consider to be restricted, can be expanded by user
		// but defaults to image base and stack pointer.
		//
		inline static std::set<register_desc> restricted_bases = { REG_SP, REG_IMGBASE };

		// The symbolic expression that will represent the virtual address 
		// if resolved to an immediate value.
		// 
		expression::reference base;

		// Special flags of the registers the base contains.
		//
		uint64_t flags = 0;

		// X values cached from expression.
		//
		std::array<uint64_t, VTIL_SYMEX_XVAL_KEYS> xvalues;

		// Construct null pointer.
		//
		pointer() {}
		pointer( std::nullptr_t ) : pointer() {}

		// Construct from symbolic expression.
		//
		pointer( const expression::reference& base );
		pointer( const expression& base ) : pointer( ( ( expression::reference&& ) make_local_reference( &base ) ) ) {}

		// Default copy/move.
		//
		pointer( pointer&& ) = default;
		pointer( const pointer& ) = default;
		pointer& operator=( pointer&& ) = default;
		pointer& operator=( const pointer& ) = default;

		// Simple pointer offseting.
		//
		pointer operator+( int64_t dst ) const;
		pointer operator-( int64_t dst ) const { return operator+( -dst ); }

		// Calculates the distance between two pointers as an optional constant.
		//
		std::optional<int64_t> operator-( const pointer& o ) const;

		// Checks whether the two pointers can overlap in terms of real destination, 
		// note that it will consider [rsp+C1] and [rsp+C2] "overlapping" so you will
		// need to check the displacement with the variable sizes considered if you 
		// are checking "is overlapping" instead.
		//
		bool can_overlap( const pointer& o ) const;
		
		// Same as can_overlap but will return false if flags do not overlap.
		//
		bool can_overlap_s( const pointer& o ) const;

		// Conversion to human-readable format.
		//
		std::string to_string() const { return base ? base->to_string() : "null"; }

		// Define reduction.
		//
		REDUCE_TO( flags, base ? ( boxed_expression& ) *base : make_static<boxed_expression>() );
	};
};