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
#include <mutex>
#include <cstdlib>
#include <atomic>
#include <algorithm>
#include <cstring>
#include <thread>
#include "detached_queue.hpp"
#include "relaxed_atomics.hpp"
#include "type_helpers.hpp"
#include "task.hpp"

// [Configuration]
// Determine the number of buckets, initial size, growth settings and the local buffer length.
//  - If local buffer length is zero, will dispatch to bucket directly.
//
#ifndef VTIL_OBJECT_POOL_BUCKETS
	#define	VTIL_OBJECT_POOL_BUCKETS           std::thread::hardware_concurrency()
#endif
#ifndef VTIL_OBJECT_POOL_INITIAL_SIZE
	#define VTIL_OBJECT_POOL_INITIAL_SIZE      ( 8ull  * 1024 * 1024 )
#endif
#ifndef VTIL_OBJECT_POOL_GROWTH_CAP
	#define VTIL_OBJECT_POOL_GROWTH_CAP        ( 64ull * 1024 * 1024 )
#endif
#ifndef VTIL_OBJECT_POOL_GROWTH_FACTOR
	#define VTIL_OBJECT_POOL_GROWTH_FACTOR     2
#endif
#ifndef VTIL_OBJECT_POOL_LOCAL_BUFFER_LEN
	#define VTIL_OBJECT_POOL_LOCAL_BUFFER_LEN  256
#endif

namespace vtil
{
	// Object pools allow for fast singular type allocation.
	//
	template <typename T>
	struct object_pool
	{
		// Forward declares and type exports.
		//
		using value_type = T;
		struct pool_instance;
		struct bucket_entry;

		// A single entry in the pool.
		//
		struct alignas( T ) object_entry
		{
			// Stores raw data.
			//
			uint8_t raw_data[ sizeof( T ) ] = { 0 };

			// Pointer to owning pool.
			//
			pool_instance* pool = nullptr;

			// Whether object has to be destructed to be used again or not.
			//
			bool deferred_destruction = false;

			// Key for free queue.
			//
			detached_queue_key<object_entry> free_queue_key;
			
			// Decay into object pointer.
			//
			T* decay() { return ( T* ) &raw_data[ 0 ]; }
			const T* decay() const { return ( const T* ) make_mutable( this )->decay(); }

			// Resolve from object pointer.
			//
			static object_entry* resolve( const void* obj ) { return ptr_at<object_entry>( ( T* ) obj, -make_offset( &object_entry::raw_data ) ); }
		};

		// Base pool type.
		//
		struct pool_instance
		{
			// Key for the pool queue.
			//
			detached_queue_key<pool_instance> pool_queue_key;

			// Number of objects we store and the objects themselves.
			//
			size_t object_count;
			object_entry objects[ 1 ];
		};

		// Declare the pool allocator.
		//
		__forceinline static pool_instance* allocate_pool( size_t n )
		{
			static_assert( alignof( object_entry ) <= 8, "Object aligned over max alignment." );
			pool_instance* pool = ( pool_instance* ) malloc( sizeof( pool_instance ) + sizeof( object_entry ) * ( n - 1 ) );
			pool->object_count = n;
			return pool;
		}
		__forceinline static void deallocate_pool( pool_instance* pool )
		{
			free( pool );
		}

		// Bucket entry dedicating a pool list to each thread.
		//
		struct bucket_entry
		{
			// Atomic queue of free memory regions.
			//
			atomic_detached_queue<object_entry> free_queue;

			// Mutex protecting the pool list.
			//
			std::mutex pool_list_mutex;

			// Last of last pool in bytes, approximated.
			//
			size_t last_pool_size_raw = 0;

			// List of pools.
			//
			detached_queue<pool_instance> pools;

			// Allocation and deallocation.
			//
			T* allocate()
			{
				static_assert( sizeof( object_entry ) < VTIL_OBJECT_POOL_INITIAL_SIZE, "Objects cannot be larger than initial size." );

				// Enter pool allocation loop:
				//
				while ( true )
				{
					// Pop entry from free queue, if non null:
					//
					if ( object_entry* entry = free_queue.pop_front( &object_entry::free_queue_key ) )
					{
						// If it's destruction was deferred, do so now.
						//
						if ( entry->deferred_destruction )
							std::destroy_at<T>( entry->decay() );

						// Return the entry.
						//
						return entry->decay();
					}

					// Acquire pool list mutex.
					//
					std::lock_guard _gp{ pool_list_mutex };

					// If free queue has any entries, try again.
					//
					if ( free_queue.size() )
						continue;

					// Determine new pool's size (raw size is merely an approximation).
					//
					size_t new_pool_size_raw = last_pool_size_raw
						? std::min<size_t>( last_pool_size_raw * VTIL_OBJECT_POOL_GROWTH_FACTOR, VTIL_OBJECT_POOL_GROWTH_CAP )
						: VTIL_OBJECT_POOL_INITIAL_SIZE;
					last_pool_size_raw = new_pool_size_raw;
					size_t object_count = new_pool_size_raw / sizeof( object_entry );

					// Allocate the pool, keep the first object to ourselves.
					//
					pool_instance* new_pool = allocate_pool( object_count );

					object_entry* return_value = &new_pool->objects[ 0 ];
					return_value->pool = new_pool;

					// Initialize every other object, linking them internally so that 
					// we don't have to hold the free-list lock too long.
					//
					object_entry* tmp_prev = nullptr;
					for ( size_t i = 1; i < object_count; i++ )
					{
						new ( new_pool->objects + i ) object_entry{
							.pool = new_pool,
							.free_queue_key = {
								.prev = &new_pool->objects[ i - 1 ].free_queue_key,
								.next = &new_pool->objects[ i + 1 ].free_queue_key
							}
						};
					}

					// Form the partial list.
					//
					detached_queue<object_entry> tmp;
					tmp.head = &new_pool->objects[ 1 ].free_queue_key;
					tmp.tail = &new_pool->objects[ object_count - 1 ].free_queue_key;
					tmp.tail->next = nullptr;
					tmp.head->prev = nullptr;
					tmp.list_size = object_count - 1;
				
					// Insert into pools list.
					//
					pools.emplace_back( &new_pool->pool_queue_key );

					// Merge into free queue.
					//
					free_queue.emplace_back( tmp );

					// Return the allocated address.
					//
					return return_value->decay();
				}
			}

			void deallocate( T* pointer )
			{
				// Resolve object entry, and emplace it into the free queue.
				//
				object_entry* entry = object_entry::resolve( pointer );
				free_queue.emplace_back( &entry->free_queue_key );
			}
		};

		// Atomic counter responsible of the grouping of threads => buckets.
		//
		inline static std::atomic<size_t> counter = { 0 };
		
		// Global list of buckets.
		//
		inline static const size_t bucket_count = VTIL_OBJECT_POOL_BUCKETS;
		static bucket_entry* get_bucket( size_t idx )
		{
			static const size_t length = VTIL_OBJECT_POOL_BUCKETS;
			static bucket_entry* entries =  new bucket_entry[ length ];
			return entries + ( idx % length );
		}

		// Local proxy that buffers all commands to avoid spinning. 
		//
		struct local_proxy
		{
			// Secondary queue that proxies bucket::free_queue.
			//
			detached_queue<object_entry> secondary_free_queue;

			// Smart bucket swapping / balancing.
			//
			size_t bucket_index = counter++;
			bucket_entry* _bucketa = get_bucket( bucket_index );
			bucket_entry* _bucketd = get_bucket( bucket_index );

			bucket_entry* get_bucket_for_alloc()
			{
				if ( _bucketa->free_queue.empty() )
					_bucketa = get_bucket( --bucket_index );
				return _bucketa;
			}
			bucket_entry* get_bucket_for_dealloc()
			{
				if ( _bucketd->free_queue.size() > ( VTIL_OBJECT_POOL_INITIAL_SIZE / sizeof( object_entry ) ) )
					_bucketd = get_bucket( ++bucket_index );
				return _bucketd;
			}

			// Allocate / deallocate proxies.
			//
			T* allocate()
			{
				// Handle no-buffering case.
				//
				if constexpr( VTIL_OBJECT_POOL_LOCAL_BUFFER_LEN == 0 )
					return get_bucket_for_alloc()->allocate();

				// If we've buffered any freed memory regions:
				//
				if ( object_entry* entry = secondary_free_queue.pop_back( &object_entry::free_queue_key ) )
				{
					// If it's destruction was deferred, do so now.
					//
					if ( entry->deferred_destruction )
						std::destroy_at<T>( entry->decay() );

					// Return the entry.
					//
					return entry->decay();
				}

				// Dispatch to bucket.
				//
				return get_bucket_for_alloc()->allocate();
			}
			void deallocate( T* pointer )
			{
				// Handle no-buffering case.
				//
				if constexpr ( VTIL_OBJECT_POOL_LOCAL_BUFFER_LEN == 0 )
					return get_bucket_for_dealloc()->deallocate( pointer );

				// Insert into free queue.
				//
				secondary_free_queue.emplace_back( &object_entry::resolve( pointer )->free_queue_key );
				
				// If queue size is over the buffer length:
				//
				if ( secondary_free_queue.size() >= VTIL_OBJECT_POOL_LOCAL_BUFFER_LEN )
					get_bucket_for_dealloc()->free_queue.emplace_back( secondary_free_queue );
			}
			void flush()
			{
				if ( secondary_free_queue.size() )
					get_bucket_for_dealloc()->free_queue.emplace_back( secondary_free_queue );
			}

			// Flush buffer on destruction.
			//
			~local_proxy() { flush(); }
		};
		inline static task_local( local_proxy ) bucket_proxy;

		// Allocate / deallocate wrappers.
		//
		__forceinline static T* allocate() { return bucket_proxy->allocate(); }
		__forceinline static void deallocate( T* pointer ) { bucket_proxy->deallocate( pointer ); }

		// Construct / deconsturct wrappers.
		//
		template<typename... Tx>
		__forceinline static T* construct( Tx&&... args ) { return new ( allocate() ) T( std::forward<Tx>( args )... ); }
		__forceinline static void destruct( T* pointer, bool deferred = true ) 
		{
			if ( !( object_entry::resolve( pointer )->deferred_destruction = deferred ) )
				std::destroy_at<T>( pointer );
			return deallocate( pointer ); 
		}
	};
};
