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

#ifndef _MSC_VER
    inline static uint64_t _umul128( uint64_t _Multiplier, uint64_t _Multiplicand, uint64_t* _HighProduct )
    {
        uint64_t LowProduct;
        uint64_t HighProduct;

        __asm__( "mulq  %[b]\n"
                 :"=d"( HighProduct ), "=a"( LowProduct )
                 : "1"( _Multiplier ), [ b ]"rm"( _Multiplicand ) );

        *_HighProduct = HighProduct;
        return LowProduct;
    }

    inline static int64_t _mul128( int64_t _Multiplier, int64_t _Multiplicand, int64_t* _HighProduct )
    {
        int64_t LowProduct;
        int64_t HighProduct;

        __asm__( "imulq  %[b]\n"
                 :"=d"( HighProduct ), "=a"( LowProduct )
                 : "1"( _Multiplier ), [ b ]"rm"( _Multiplicand ) );

        *_HighProduct = HighProduct;
        return LowProduct;
    }

    inline static int64_t __mulh( int64_t _Multiplier, int64_t _Multiplicand )
    {
        int64_t HighProduct;
        _mul128( _Multiplier, _Multiplicand, &HighProduct );
        return HighProduct;
    }

    inline static uint64_t __umulh( uint64_t _Multiplier, uint64_t _Multiplicand )
    {
        uint64_t HighProduct;
        _umul128( _Multiplier, _Multiplicand, &HighProduct );
        return HighProduct;
    }
#else
    #include <intrin.h>
#endif