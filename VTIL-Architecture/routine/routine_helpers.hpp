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
#include <type_traits>
#include <vtil/utility>
#include "basic_block.hpp"
#include "routine.hpp"

namespace vtil
{
	namespace impl
	{
		template<typename callback, typename iterator_type, bool fwd>
		static void enumerate_instructions( callback&& fn, iterator_type it, const iterator_type& dst, path_set& set )
		{
			// If already in path-set, skip.
			//
			if ( !set.emplace( it.container ).second )
				return;

			// Until we reach the destination:
			//
			std::vector<basic_block*>* links = nullptr;
			while ( it != dst )
			{
				// If we reached the end of the block, set links and break.
				// - Forward.
				//
				if ( fwd && it.is_end() )
				{
					links = &it.container->next;
					break;
				}

				// Invoke callback.
				//
				fn( it );

				// If we reached the end of the block, set links and break.
				// - Backwards
				//
				if ( !fwd && it.is_end() )
				{
					links = &it.container->prev;
					break;
				}

				// Skip to next iterator.
				//
				std::advance( it, fwd ? +1 : -1 );
			}

			// Recurse.
			//
			if ( links && !links->empty() )
			{
				constexpr auto make_it = [ ] ( basic_block* blk ) -> iterator_type { return fwd ? blk->begin() : blk->end(); };

				for ( auto i = links->end() - 1; i != links->begin(); i-- )
					enumerate_instructions<callback, iterator_type, fwd>( make_copy<callback>( fn ), make_it( *i ), dst, set );
				enumerate_instructions<callback, iterator_type, fwd>( std::forward<callback>( fn ), make_it( links->front() ), dst, set );
			}
		}
	};

	// Enumerates every instruction in the routine forward/backward, within the boundaries if specified.
	//
	template<typename callback, typename iterator_type>
	void routine::enumerate( callback fn, const iterator_type& src, const iterator_type& dst ) const
	{
		path_set set = {};
		impl::enumerate_instructions<callback, iterator_type, true>
		(
			std::move( fn ), 
			src,
			dst, 
			set 
		);
	}
	template<typename callback, typename iterator_type>
	void routine::enumerate_bwd( callback fn, const iterator_type& src, const iterator_type& dst ) const
	{
		path_set set = {};
		impl::enumerate_instructions<callback, iterator_type, false>
		(
			std::move( fn ), 
			src, 
			dst, 
			set 
		);
	}
};