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
#include "symbolic.hpp"

namespace vtil
{
	// Reads from the register.
	//
	symbolic::expression::reference symbolic_vm::read_register( const register_desc& desc )
	{
		bitcnt_t size = size_register( desc );
		register_desc full = { desc.flags, desc.local_id, size, 0, desc.architecture };

		auto it = register_state.find( full );
		auto exp = it == register_state.end()
			? symbolic::variable{ full }.to_expression( false )
			: it->second;

		if ( lazy_io ) exp.make_lazy();
		if ( desc.bit_offset ) exp = exp >> desc.bit_offset;
		exp.resize( desc.bit_count );
		return lazy_io ? exp : exp.simplify();
	}

	// Writes to the register.
	//
	void symbolic_vm::write_register( const register_desc& desc, symbolic::expression::reference value )
	{
		bitcnt_t size = size_register( desc );
		register_desc full = { desc.flags, desc.local_id, size, 0, desc.architecture };

		if ( desc.bit_count == size && desc.bit_offset == 0 )
		{
			register_state.erase( desc );
			register_state.emplace( desc, std::move( value ) );
		}
		else
		{
			auto& exp = register_state[ full ];
			if ( !exp ) exp = symbolic::make_register_ex( full );
			exp = ( std::move( exp ) & ~desc.get_mask() ) | ( value.resize( desc.bit_count ).resize( size ) << desc.bit_offset );
		}
	}

	// Reads the given number of bytes from the memory.
	//
	symbolic::expression::reference symbolic_vm::read_memory( const symbolic::expression::reference& pointer, size_t byte_count )
	{
		bitcnt_t bcnt = math::narrow_cast<bitcnt_t>( byte_count * 8 );
		symbolic::expression::reference exp = memory_state.read_v(
			pointer, 
			bcnt 
		);
		return lazy_io ? exp.make_lazy() : exp.simplify();
	}

	// Writes the given expression to the memory.
	//
	void symbolic_vm::write_memory( const symbolic::expression::reference& pointer, symbolic::expression::reference value )
	{
		memory_state.write( 
			pointer, 
			value.resize( ( value->size() + 7 ) & ~7 ) 
		);
	}

	// Override execute to enforce lazyness.
	//
	bool symbolic_vm::execute( const instruction& ins )
	{
		bool old = std::exchange( lazy_io, true );
		bool state = vm_interface::execute( ins );
		lazy_io = old;
		return state;
	}
};