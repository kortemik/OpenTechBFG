/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.

This file is part of the Doom 3 BFG Edition GPL Source Code ("Doom 3 BFG Edition Source Code").

Doom 3 BFG Edition Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Doom 3 BFG Edition Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Doom 3 BFG Edition Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Doom 3 BFG Edition Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Doom 3 BFG Edition Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/
#pragma hdrstop
#ifndef GCC_BUILTINS_H
#define GCC_BUILTINS_H

#ifdef __cplusplus
extern "C" {
#endif

//---------------
// Allows GCC's built-in intrinsics for x86 to be used "as" VC's intrinsics
//---------------

#ifdef ID_QNX_X86_SSE_INTRIN

typedef float __v4sf__ __attribute__ ((__vector_size__ (16)));;
#ifndef v4sf
#define v4sf __v4sf__
#endif
#ifndef __m128
#define __m128 __v4sf__
#endif

typedef int __v4si__ __attribute__ ((__vector_size__ (16)));
#ifndef v4si
#define v4si __v4si__
#endif
#ifndef __m128i
#define __m128i __v4si__
#endif

#ifndef __builtin_ia32_loadss
static ID_FORCE_INLINE __v4sf__ __attribute__((__always_inline__, __nodebug__)) __builtin_ia32_loadss(float const *__fp) {
	float _f = *__fp;
	return (__v4sf__){ _f, _f, _f, _f };
}
#endif

#define _mm_load_ps __builtin_ia32_loadups
#define _mm_load1_ps __builtin_ia32_loadss
#define _mm_load_si128(p) (*(__m128i*)(p))
#define _mm_store_ps __builtin_ia32_storeups
#define _mm_stream_si128 __builtin_ia32_movntq

static ID_FORCE_INLINE __m128 _mm_setzero_ps() {
	static float a = 0.0f;
	static v4sf va = __builtin_ia32_loadss( & a );
	return va;
}

#define _mm_add_ps __builtin_ia32_addps
#define _mm_sub_ps __builtin_ia32_subps
#define _mm_mul_ps __builtin_ia32_mulps
#define _mm_xor_ps __builtin_ia32_xorps

#endif

#ifdef __cplusplus
}
#endif

#endif // GCC_BUILTINS_H
