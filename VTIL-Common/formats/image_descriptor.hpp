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
#include <stdint.h>
#include <string>
#include <string_view>
#include <optional>
#include "../util/zip.hpp"
#include "../util/function_view.hpp"
#include "../util/range.hpp"

namespace vtil
{
	// Generic section information.
	//
	struct section_descriptor
	{
		// Name of the section.
		//
		std::string_view name = "";

		// Characteristics of the section.
		//
		bool valid = false;
		bool read = false;
		bool write = false;
		bool execute = false;

		// Relative virtual address of the section and the lenght of the data at runtime.
		//
		uint64_t virtual_address = 0;
		size_t virtual_size = 0;

		// Physical address of the section and the length of the data on disk.
		//
		uint64_t physical_address = 0;
		size_t physical_size = 0;
	
		// Cast to bool redirects to ::valid.
		//
		explicit operator bool() const { return valid; }

		// Checks if RVA is within this section.
		//
		bool contains( uint64_t rva, size_t n = 1 ) const
		{
			return virtual_address < ( rva + n ) && rva < ( virtual_address + virtual_size );
		}

		// Translates relative virtual address to physical address.
		//
		std::optional<uint64_t> translate( uint64_t rva ) const
		{
			if ( virtual_address <= rva && rva < ( virtual_address + virtual_size ) )
			{
				uint64_t offset = rva - virtual_address;
				if ( offset < physical_size )
					return offset + physical_address;
			}
			return std::nullopt;
		}

		// Basic comparison operators, only checks the mapping.
		//
		bool operator==( const section_descriptor& o ) const
		{
			return virtual_address == o.virtual_address &&
				   physical_address == o.physical_address;
		}
		bool operator!=( const section_descriptor& o ) const { return !operator==( o ); }
	};

	// Generic relocation information.
	//
	struct relocation_descriptor
	{
		uint64_t rva;
		size_t length;
		void( *relocator )( void* data, int64_t delta );
	};

	// Generic image interface.
	//
	struct image_descriptor
	{
		// Declare the iterator type.
		//
		struct section_iterator
		{
			// Generic iterator typedefs.
			//
			using iterator_category = std::bidirectional_iterator_tag;
			using difference_type =   int64_t;
			using reference =         section_descriptor;
			using value_type =        section_descriptor;
			using pointer =           void*;

			// Contains an index and a reference to the original image.
			//
			const image_descriptor* image;
			size_t at;

			// Support bidirectional iteration.
			//
			section_iterator& operator++() { at++; return *this; }
			section_iterator& operator--() { at--; return *this; }
			section_iterator operator++( int ) { auto s = *this; operator++(); return s; }
			section_iterator operator--( int ) { auto s = *this; operator--(); return s; }

			// Equality check against another iterator.
			//
			bool operator==( const section_iterator& other ) const { return at == other.at && image == other.image; }
			bool operator!=( const section_iterator& other ) const { return at != other.at || image != other.image; }

			// Redirect dereferencing to the image itself.
			//
			value_type operator*() const { return image->get_section( at ); }
		};

		// Returns the number of sections in the binary.
		//
		virtual size_t get_section_count() const = 0;

		// Returns the details of the Nth section in the binary.
		//
		virtual section_descriptor get_section( size_t index ) const = 0;

		// Modifies the characteristics of the Nth section according to the information passed.
		//
		virtual void modify_section( size_t index, const section_descriptor& desc ) = 0;

		// Returns the next relative virtual address that'd the next section added using ::add_section would be assigned.
		//
		virtual uint64_t next_free_rva() const = 0;

		// Appends a new section to the binary. ::name/read/write/execute will be used, rest will be overwritten.
		//
		virtual void add_section( section_descriptor& in_out, const void* data, size_t size ) = 0;

		// Invokes the enumerator for each relocation entry in the binary, breaks if enumerator returns true.
		//
		virtual void enum_relocations( const function_view<bool( const relocation_descriptor& )>& fn ) const = 0;

		// Returns the image base.
		//
		virtual uint64_t get_image_base() const = 0;

		// Returns the image size.
		//
		virtual size_t get_image_size() const = 0;

		// Returns the entry point's RVA if relevant, else nullopt.
		//
		virtual std::optional<uint64_t> get_entry_point() const = 0;

		// Returns whether the image has any relocations.
		//
		virtual bool has_relocations() const = 0;

		// Returns the image size and the raw byte array.
		//
		virtual size_t size() const = 0;
		virtual void* data() = 0;
		virtual const void* cdata() const = 0;

		// Retuns whether or not the image is valid.
		//
		virtual bool is_valid() const = 0;

		// Returns the data associated with the given relative virtual address.
		//
		template<typename T = void>
		T* rva_to_ptr( uint64_t rva )
		{
			auto scn = rva_to_section( rva );
			if ( !scn ) return nullptr;
			auto offset = scn.translate( rva );
			if ( !offset ) return nullptr;
			return ( T* ) ( ( uint8_t* ) data() + *offset );
		}
		template<typename T = void>
		const T* rva_to_ptr( uint64_t rva ) const
		{
			auto scn = rva_to_section( rva );
			if ( !scn ) return nullptr;
			auto offset = scn.translate( rva );
			if ( !offset ) return nullptr;
			return ( const T* ) ( ( const uint8_t* ) cdata() + *offset );
		}

		// Returns an enumeratable section list.
		//
		auto sections() const { return make_range<section_iterator>( { this, 0 }, { this, get_section_count() } ); }

		// Returns the section associated with the given relative virtual address.
		//
		section_descriptor rva_to_section( uint64_t rva ) const
		{
			for ( auto scn : sections() )
				if ( scn.virtual_address <= rva && rva < ( scn.virtual_address + scn.virtual_size ) )
					return scn;
			return {};
		}

		// Returns whether the address provided will be relocated or not.
		//
		bool is_relocated( uint64_t rva, size_t n = 1 ) const
		{
			bool found = false;
			enum_relocations( [ & ] ( const relocation_descriptor& e )
			{
				return ( found = ( e.rva < ( rva + n ) && rva < ( e.rva + e.length ) ) );
			} );
			return found;
		}

		// Returns a list of all relocation entries.
		//
		std::vector<relocation_descriptor> get_relocations() const
		{
			std::vector<relocation_descriptor> entries;
			enum_relocations( [ & ] ( const relocation_descriptor& e ) { entries.emplace_back( e ); return false; } );
			return entries;
		}

		// Enumerates all non-empty and executable sections, breaks if enumerator returns true.
		//
		void enum_executable( const function_view<bool( const section_descriptor& )>& fn ) const
		{
			for ( auto scn : sections() )
				if ( scn.execute && scn.physical_size && scn.virtual_size )
					if ( fn( scn ) )
						return;
		}

		// Cast to bool redirects to ::is_valid.
		//
		explicit operator bool() const { return is_valid(); }
	};
};