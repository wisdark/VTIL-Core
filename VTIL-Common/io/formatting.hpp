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
#include <cstring>
#include <cstdio>
#include <type_traits>
#include <exception>
#include <optional>
#include <filesystem>
#include <numeric>
#include <string_view>
#include "../util/lt_typeid.hpp"
#include "../util/type_helpers.hpp"
#include "../util/time.hpp"
#include "../util/numeric_iterator.hpp"
#include "../util/intrinsics.hpp"
#include "enum_name.hpp"
#include "strong_formatting.hpp"

#ifdef __GNUG__
	#include <cxxabi.h>
#endif

// [Configuration]
// Determine the way we format the instructions.
//
#ifndef VTIL_FMT_DEFINED
	#define VTIL_FMT_INS_MNM	"%-8s"
	#define VTIL_FMT_INS_OPR	"%-12s"
	#define VTIL_FMT_INS_MNM_S	8
	#define VTIL_FMT_INS_OPR_S	12
	#define VTIL_FMT_SUFFIX_1	'b'
	#define VTIL_FMT_SUFFIX_2	'w'
	#define VTIL_FMT_SUFFIX_4	'd'
	#define VTIL_FMT_SUFFIX_8	'q'
	#define VTIL_FMT_DEFINED
#endif

namespace vtil::format
{
	namespace impl
	{
		// Returns a temporary but valid const (w)char* for the given std::(w)string.
		//
		template<typename T>
		static T* buffer_string( std::basic_string<T>&& value )
		{
			static thread_local size_t index = 0;
			static thread_local std::basic_string<T> ring_buffer[ 32 ];
			
			auto& ref = ring_buffer[ index ];
			ref = std::move( value );
			index = ++index % std::size( ring_buffer );
			return ref.data();
		}

		// Fixes the type name to be more friendly.
		//
		static std::string fix_type_name( std::string in )
		{
#ifdef __GNUG__
			int status;
			char* demangled_name = abi::__cxa_demangle( in.data(), nullptr, nullptr, &status );
			// If demangling succeeds, set the name.
			//
			if ( status == 0 )
			{
				in = demangled_name;
			}
			// Free unconditionally.
			//
			free( demangled_name );
#endif
			
			static const std::string remove_list[] = {
				"struct ",
				"class ",
				"enum ",
				"vtil::"
			};
			for ( auto& str : remove_list )
			{
				if ( in.starts_with( str ) )
					return fix_type_name( in.substr( str.length() ) );

				for ( size_t i = 0; i < in.size(); i++ )
					if ( in[ i ] == '<' && in.substr( i + 1 ).starts_with( str ) )
						in = in.substr( 0, i + 1 ) + in.substr( i + 1 + str.length() );
			}
			return in;
		}
	};

	// Suffixes used to indicate registers of N bytes.
	//
	static constexpr char suffix_map[] = { 0, VTIL_FMT_SUFFIX_1, VTIL_FMT_SUFFIX_2, 0, VTIL_FMT_SUFFIX_4, 0, 0, 0, VTIL_FMT_SUFFIX_8 };

	// Returns the type name of the object passed, dynamic type name will
	// redirect to static type name if RTTI is not supported.
	//
	template<typename T>
	static std::string static_type_name()
	{
#if HAS_RTTI
		static const std::string res = impl::fix_type_name( typeid( T ).name() );
		return res;
#else
		char buf[ 32 ];
		sprintf_s( buf, "Type%llx", lt_typeid<T>::value );
		return buf;
#endif
	}
	template<typename T>
	static std::string dynamic_type_name( const T& o )
	{
#if HAS_RTTI
		return impl::fix_type_name( typeid( o ).name() );
#else
		return static_type_name<T>();
#endif
	}

	// VTIL string-convertable types implement [std::string T::to_string() const];
	//
	template<typename T>
	concept CustomStringConvertible = requires( T v ) { v.to_string(); };

	// Checks if std::to_string is specialized to convert type into string.
	//
	template<typename T>
	concept StdStringConvertible = requires( T v ) { std::to_string( v ); };

	// Converts any given object to a string.
	//
	template<typename T>
	static auto as_string( const T& x );
	template<typename T>
	concept StringConvertible = requires( T v ) { !is_specialization_v<type_tag, decltype( as_string( v ) )>; };

	template<typename T>
	__forceinline static auto as_string( const T& x )
	{
		using base_type = std::decay_t<T>;
		
		if constexpr ( CustomStringConvertible<T> )
		{
			return x.to_string();
		}
		else if constexpr ( Enum<T> )
		{
			return enum_name<T>::resolve( x );
		}
		else if constexpr ( Duration<T> )
		{
			return time::to_string( x );
		}
		else if constexpr ( std::is_same_v<base_type, uint64_t> )
		{
			char buffer[ 16 + 3 ];
			return std::string{ buffer, buffer + snprintf( buffer, std::size( buffer ), "0x%llx", x ) };
		}
		else if constexpr ( std::is_same_v<base_type, int64_t> )
		{
			return hexadecimal<base_type>( x ).to_string();
		}
		else if constexpr ( std::is_same_v<base_type, bool> )
		{
			return std::string{ x ? "true" : "false" };
		}
		else if constexpr ( StdStringConvertible<T> )
		{
			return std::to_string( x );
		}
		else if constexpr ( std::is_base_of_v<std::exception, T> )
		{
			return std::string{ x.what() };
		}
		else if constexpr ( CppString<base_type> || CppStringView<base_type> )
		{
			return std::string{ x.begin(), x.end() };
		}
		else if constexpr ( CString<base_type> )
		{
			return std::string{
				x,
				x + std::char_traits<string_unit_t<base_type>>::length( x )
			};
		}
		else if constexpr ( std::is_same_v<base_type, std::filesystem::directory_entry> )
		{
			return x.path().string();
		}
		else if constexpr ( std::is_same_v<base_type, std::filesystem::path> )
		{
			return x.string();
		}
		else if constexpr ( std::is_pointer_v<base_type> )
		{
			char buffer[ 17 ];
			snprintf( buffer, 17, "%p", x );
			return std::string{ buffer };
		}
		else if constexpr ( is_specialization_v<std::pair, base_type> )
		{
			if constexpr ( StringConvertible<decltype( x.first )> && StringConvertible<decltype( x.second )> )
			{
				return "(" + as_string( x.first ) + ", " + as_string( x.second ) + ")";
			}
			else return type_tag<T>{};
		}
		else if constexpr ( is_specialization_v<std::tuple, base_type> )
		{
			constexpr bool is_tuple_str_cvtable = [ ] ()
			{
				bool cvtable = true;
				if constexpr ( std::tuple_size_v<base_type> > 0 )
				{
					make_constant_series<std::tuple_size_v<base_type>>( [ & ] ( auto tag )
					{
						if constexpr ( !StringConvertible<std::tuple_element_t<decltype( tag )::value, base_type>> )
							cvtable = false;
					} );
				}
				return cvtable;
			}();

			if constexpr ( std::tuple_size_v<base_type> == 0 )
				return "{}";
			else if constexpr ( is_tuple_str_cvtable )
			{
				std::string res = std::apply( [ ] ( auto&&... args ) {
					return ( ( as_string( args ) + ", " ) + ... );
				}, x );
				return "{" + res.substr(0, res.length() - 2) + "}";
			}
			else return type_tag<T>{};
		}
		else if constexpr ( is_specialization_v<std::optional, base_type> )
		{
			if constexpr ( StringConvertible<decltype( x.value() )> )
			{
				if ( x.has_value() )
					return as_string( x.value() );
				else
					return std::string{ "nullopt" };
			}
			else return type_tag<T>{};
		}
		else if constexpr ( Iterable<T> )
		{
			if constexpr ( StringConvertible<decltype( *std::begin( x ) )> )
			{
				std::string items = {};
				for ( auto&& entry : x )
					items += as_string( entry ) + ", ";
				if ( !items.empty() ) items.resize( items.size() - 2 );
				return "{" + items + "}";
			}
			else return type_tag<T>{};
		}
		else return type_tag<T>{};
	}

	// Used to fix std::(w)string usage in combination with "%(l)s".
	//
	template<typename T>
	__forceinline static auto fix_parameter( T&& x )
	{
		using base_type = std::remove_cvref_t<T>;

		// If fundamental type, return as is.
		//
		if constexpr ( std::is_fundamental_v<base_type> || std::is_enum_v<base_type> || 
					   std::is_pointer_v<base_type> || std::is_array_v<base_type> )
		{
			return x;
		}
		// If it is a basic string:
		//
		else if constexpr ( std::is_same_v<base_type, std::string> || std::is_same_v<base_type, std::wstring> )
		{
			// If it is a reference, invoke ::data()
			//
			if constexpr ( std::is_reference_v<T> )
				return x.data();
			// Otherwise call buffer helper.
			//
			else
				return impl::buffer_string( std::move( x ) );
		}
		// If string convertible:
		//
		else if constexpr ( StringConvertible<T> )
		{
			return impl::buffer_string( as_string( std::forward<T>( x ) ) );
		}
		// If none matched, forcefully convert into [type @ pointer].
		//
		else
		{
			char buffer[ 32 ];
			snprintf( buffer, 32, "%p", &x );
			return impl::buffer_string( "[" + dynamic_type_name( x ) + "@" + std::string( buffer ) + "]" );
		}
	}

	// Returns formatted string according to <fms>.
	//
	template<typename... params>
	static std::string str( const char* fmt, params&&... ps )
	{
		std::string buffer;
		buffer.resize( snprintf( nullptr, 0, fmt, fix_parameter( ps )... ) );
		snprintf( buffer.data(), buffer.size() + 1, fmt, fix_parameter<params>( std::forward<params>( ps ) )... );
		return buffer;
	}

	// Formats the integer into a signed hexadecimal.
	//
	template<Integral T>
	static std::string hex( T value )
	{
		if constexpr ( !std::is_signed_v<T> )
		{
			return str( "0x%llx", value );
		}
		else
		{
			if ( value >= 0 ) return str( "0x%llx", value );
			else              return str( "-0x%llx", -value );
		}
	}

	// Formats the integer into a signed hexadecimal with explicit + if positive.
	//
	static std::string offset( int64_t value )
	{
		if ( value >= 0 ) return str( "+ 0x%llx", value );
		else              return str( "- 0x%llx", -value );
	}
};
#undef HAS_RTTI

// Export the concepts.
//
namespace vtil
{
	using format::CustomStringConvertible;
	using format::StdStringConvertible;
	using format::StringConvertible;
};
