/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.
Copyright (C) 2014 Vincent Simonetti

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
/*
================================================================================================
Contains the DxtEncoder implementation for SSE2.
================================================================================================
*/
#pragma hdrstop
#include "DXTCodec_local.h"
#include "DXTCodec.h"

//TODO: QNX ASM implementation should be made portable. Name mangling variables to access them inline isn't portable.

#if defined( ID_WIN_X86_SSE2_INTRIN ) || ( ( defined( ID_WIN_X86_ASM ) || defined( ID_MAC_X86_ASM ) ) ) || defined( ID_QNX_X86_SSE2_INTRIN ) || defined( ID_QNX_X86_SSE2_ASM )

//#define TEST_COMPRESSION
#ifdef TEST_COMPRESSION
#include <malloc.h>
#endif

#define INSET_COLOR_SHIFT		4		// inset the bounding box with ( range >> shift )
#define INSET_ALPHA_SHIFT		5		// inset alpha channel

#define C565_5_MASK				0xF8	// 0xFF minus last three bits
#define C565_6_MASK				0xFC	// 0xFF minus last two bits

#define NVIDIA_7X_HARDWARE_BUG_FIX		// keep the DXT5 colors sorted as: max, min

#if !defined( R_SHUFFLE_D )
#define R_SHUFFLE_D( x, y, z, w )	(( (w) & 3 ) << 6 | ( (z) & 3 ) << 4 | ( (y) & 3 ) << 2 | ( (x) & 3 ))
#endif

typedef uint16	word;
typedef uint32	dword;

#ifdef ID_QNX_X86_SSE2_ASM
ALIGN16( static __m128i SIMD_SSE2_zero ) = { 0LL, 0LL };
#else
ALIGN16( static __m128i SIMD_SSE2_zero ) = { 0, 0, 0, 0 };
#endif
ALIGN16( static dword SIMD_SSE2_dword_byte_mask[4] ) = { 0x000000FF, 0x000000FF, 0x000000FF, 0x000000FF };
ALIGN16( static dword SIMD_SSE2_dword_word_mask[4] ) = { 0x0000FFFF, 0x0000FFFF, 0x0000FFFF, 0x0000FFFF };
ALIGN16( static dword SIMD_SSE2_dword_red_mask[4] )   = { 0x000000FF, 0x000000FF, 0x000000FF, 0x000000FF };
ALIGN16( static dword SIMD_SSE2_dword_green_mask[4] ) = { 0x0000FF00, 0x0000FF00, 0x0000FF00, 0x0000FF00 };
ALIGN16( static dword SIMD_SSE2_dword_blue_mask[4] )  = { 0x00FF0000, 0x00FF0000, 0x00FF0000, 0x00FF0000 };
ALIGN16( static dword SIMD_SSE2_dword_colorMask_1010[4] ) = { 0xFFFFFFFF, 0x00000000, 0xFFFFFFFF, 0x00000000 };
ALIGN16( static dword SIMD_SSE2_dword_colorMask_0100[4] ) = { 0x00000000, 0xFFFFFFFF, 0x00000000, 0x00000000 };
ALIGN16( static dword SIMD_SSE2_dword_alpha_bit_mask0[4] ) = { 7<<0, 0, 7<<0, 0 };
ALIGN16( static dword SIMD_SSE2_dword_alpha_bit_mask1[4] ) = { 7<<3, 0, 7<<3, 0 };
ALIGN16( static dword SIMD_SSE2_dword_alpha_bit_mask2[4] ) = { 7<<6, 0, 7<<6, 0 };
ALIGN16( static dword SIMD_SSE2_dword_alpha_bit_mask3[4] ) = { 7<<9, 0, 7<<9, 0 };
ALIGN16( static dword SIMD_SSE2_dword_alpha_bit_mask4[4] ) = { 7<<12, 0, 7<<12, 0 };
ALIGN16( static dword SIMD_SSE2_dword_alpha_bit_mask5[4] ) = { 7<<15, 0, 7<<15, 0 };
ALIGN16( static dword SIMD_SSE2_dword_alpha_bit_mask6[4] ) = { 7<<18, 0, 7<<18, 0 };
ALIGN16( static dword SIMD_SSE2_dword_alpha_bit_mask7[4] ) = { 7<<21, 0, 7<<21, 0 };
ALIGN16( static dword SIMD_SSE2_dword_color_bit_mask0[4] ) = { 3<<0, 0, 3<<0, 0 };
ALIGN16( static dword SIMD_SSE2_dword_color_bit_mask1[4] ) = { 3<<2, 0, 3<<2, 0 };
ALIGN16( static dword SIMD_SSE2_dword_color_bit_mask2[4] ) = { 3<<4, 0, 3<<4, 0 };
ALIGN16( static dword SIMD_SSE2_dword_color_bit_mask3[4] ) = { 3<<6, 0, 3<<6, 0 };
ALIGN16( static dword SIMD_SSE2_dword_color_bit_mask4[4] ) = { 3<<8, 0, 3<<8, 0 };
ALIGN16( static dword SIMD_SSE2_dword_color_bit_mask5[4] ) = { 3<<10, 0, 3<<10, 0 };
ALIGN16( static dword SIMD_SSE2_dword_color_bit_mask6[4] ) = { 3<<12, 0, 3<<12, 0 };
ALIGN16( static dword SIMD_SSE2_dword_color_bit_mask7[4] ) = { 3<<14, 0, 3<<14, 0 };
ALIGN16( static word SIMD_SSE2_word_0[8] ) = { 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 };
ALIGN16( static word SIMD_SSE2_word_1[8] ) = { 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001 };
ALIGN16( static word SIMD_SSE2_word_2[8] ) = { 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002 };
ALIGN16( static word SIMD_SSE2_word_3[8] ) = { 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003 };
ALIGN16( static word SIMD_SSE2_word_7[8] ) = { 0x0007, 0x0007, 0x0007, 0x0007, 0x0007, 0x0007, 0x0007, 0x0007 };
ALIGN16( static word SIMD_SSE2_word_8[8] ) = { 0x0008, 0x0008, 0x0008, 0x0008, 0x0008, 0x0008, 0x0008, 0x0008 };
ALIGN16( static word SIMD_SSE2_word_31[8] ) = { 31, 31, 31, 31, 31, 31, 31, 31 };
ALIGN16( static word SIMD_SSE2_word_63[8] ) = { 63, 63, 63, 63, 63, 63, 63, 63 };
ALIGN16( static word SIMD_SSE2_word_127[8] ) = { 127, 127, 127, 127, 127, 127, 127, 127 };
ALIGN16( static word SIMD_SSE2_word_255[8] ) = { 255, 255, 255, 255, 255, 255, 255, 255 };
ALIGN16( static word SIMD_SSE2_word_center_128[8] ) = { 128, 128, 0, 0, 0, 0, 0, 0 };
ALIGN16( static word SIMD_SSE2_word_div_by_3[8] ) = { (1<<16)/3+1, (1<<16)/3+1, (1<<16)/3+1, (1<<16)/3+1, (1<<16)/3+1, (1<<16)/3+1, (1<<16)/3+1, (1<<16)/3+1 };
ALIGN16( static word SIMD_SSE2_word_div_by_6[8] ) = { (1<<16)/6+1, (1<<16)/6+1, (1<<16)/6+1, (1<<16)/6+1, (1<<16)/6+1, (1<<16)/6+1, (1<<16)/6+1, (1<<16)/6+1 };
ALIGN16( static word SIMD_SSE2_word_div_by_14[8] ) = { (1<<16)/14+1, (1<<16)/14+1, (1<<16)/14+1, (1<<16)/14+1, (1<<16)/14+1, (1<<16)/14+1, (1<<16)/14+1, (1<<16)/14+1 };
ALIGN16( static word SIMD_SSE2_word_scale_7_9_11_13[8] ) = { 7, 7, 9, 9, 11, 11, 13, 13 };
ALIGN16( static word SIMD_SSE2_word_scale_7_5_3_1[8] ) = { 7, 7, 5, 5, 3, 3, 1, 1 };
ALIGN16( static word SIMD_SSE2_word_scale_5_3_1[8] ) = { 5, 3, 1, 0, 5, 3, 1, 0 };
ALIGN16( static word SIMD_SSE2_word_scale_1_3_5[8] ) = { 1, 3, 5, 0, 1, 3, 5, 0 };
ALIGN16( static word SIMD_SSE2_word_insetShift[8] ) = { 1 << ( 16 - INSET_COLOR_SHIFT ), 1 << ( 16 - INSET_COLOR_SHIFT ), 1 << ( 16 - INSET_COLOR_SHIFT ), 1 << ( 16 - INSET_ALPHA_SHIFT ), 0, 0, 0, 0 };
ALIGN16( static word SIMD_SSE2_word_insetYCoCgRound[8] ) = { ((1<<(INSET_COLOR_SHIFT-1))-1), ((1<<(INSET_COLOR_SHIFT-1))-1), ((1<<(INSET_COLOR_SHIFT-1))-1), ((1<<(INSET_ALPHA_SHIFT-1))-1), 0, 0, 0, 0 };
ALIGN16( static word SIMD_SSE2_word_insetYCoCgMask[8] ) = { 0xFFFF, 0xFFFF, 0x0000, 0xFFFF, 0xFFFF, 0xFFFF, 0x0000, 0xFFFF };
ALIGN16( static word SIMD_SSE2_word_insetYCoCgShiftUp[8] ) = { 1 << INSET_COLOR_SHIFT, 1 << INSET_COLOR_SHIFT, 1 << INSET_COLOR_SHIFT, 1 << INSET_ALPHA_SHIFT, 0, 0, 0, 0 };
ALIGN16( static word SIMD_SSE2_word_insetYCoCgShiftDown[8] ) = { 1 << ( 16 - INSET_COLOR_SHIFT ), 1 << ( 16 - INSET_COLOR_SHIFT ), 1 << ( 16 - INSET_COLOR_SHIFT ), 1 << ( 16 - INSET_ALPHA_SHIFT ), 0, 0, 0, 0 };
ALIGN16( static word SIMD_SSE2_word_insetYCoCgQuantMask[8] ) = { C565_5_MASK, C565_6_MASK, C565_5_MASK, 0xFF, C565_5_MASK, C565_6_MASK, C565_5_MASK, 0xFF };
ALIGN16( static word SIMD_SSE2_word_insetYCoCgRep[8] ) = { 1 << ( 16 - 5 ), 1 << ( 16 - 6 ), 1 << ( 16 - 5 ), 0, 1 << ( 16 - 5 ), 1 << ( 16 - 6 ), 1 << ( 16 - 5 ), 0 };
ALIGN16( static word SIMD_SSE2_word_insetNormalDXT5Round[8] ) = { 0, ((1<<(INSET_COLOR_SHIFT-1))-1), 0, ((1<<(INSET_ALPHA_SHIFT-1))-1), 0, 0, 0, 0 };
ALIGN16( static word SIMD_SSE2_word_insetNormalDXT5Mask[8] ) = { 0x0000, 0xFFFF, 0x0000, 0xFFFF, 0x0000, 0x0000, 0x0000, 0x0000 };
ALIGN16( static word SIMD_SSE2_word_insetNormalDXT5ShiftUp[8] ) = { 1, 1 << INSET_COLOR_SHIFT, 1, 1 << INSET_ALPHA_SHIFT, 1, 1, 1, 1 };
ALIGN16( static word SIMD_SSE2_word_insetNormalDXT5ShiftDown[8] ) = { 0, 1 << ( 16 - INSET_COLOR_SHIFT ), 0, 1 << ( 16 - INSET_ALPHA_SHIFT ), 0, 0, 0, 0 };
ALIGN16( static word SIMD_SSE2_word_insetNormalDXT5QuantMask[8] ) = { 0xFF, C565_6_MASK, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
ALIGN16( static word SIMD_SSE2_word_insetNormalDXT5Rep[8] ) = { 0, 1 << ( 16 - 6 ), 0, 0, 0, 0, 0, 0 };
ALIGN16( static word SIMD_SSE2_word_insetNormal3DcRound[8] ) = { ((1<<(INSET_ALPHA_SHIFT-1))-1), ((1<<(INSET_ALPHA_SHIFT-1))-1), 0, 0, 0, 0, 0, 0 };
ALIGN16( static word SIMD_SSE2_word_insetNormal3DcMask[8] ) = { 0xFFFF, 0xFFFF, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 };
ALIGN16( static word SIMD_SSE2_word_insetNormal3DcShiftUp[8] ) = { 1 << INSET_ALPHA_SHIFT, 1 << INSET_ALPHA_SHIFT, 1, 1, 1, 1, 1, 1 };
ALIGN16( static word SIMD_SSE2_word_insetNormal3DcShiftDown[8] ) = { 1 << ( 16 - INSET_ALPHA_SHIFT ), 1 << ( 16 - INSET_ALPHA_SHIFT ), 0, 0, 0, 0, 0, 0 };
ALIGN16( static byte SIMD_SSE2_byte_0[16] ) = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
ALIGN16( static byte SIMD_SSE2_byte_1[16] ) = { 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01 };
ALIGN16( static byte SIMD_SSE2_byte_2[16] ) = { 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02 };
ALIGN16( static byte SIMD_SSE2_byte_3[16] ) = { 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03 };
ALIGN16( static byte SIMD_SSE2_byte_4[16] ) = { 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04 };
ALIGN16( static byte SIMD_SSE2_byte_7[16] ) = { 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07 };
ALIGN16( static byte SIMD_SSE2_byte_8[16] ) = { 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08 };
ALIGN16( static byte SIMD_SSE2_byte_not[16] ) = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
ALIGN16( static byte SIMD_SSE2_byte_colorMask[16] ) = { C565_5_MASK, C565_6_MASK, C565_5_MASK, 0x00, 0x00, 0x00, 0x00, 0x00, C565_5_MASK, C565_6_MASK, C565_5_MASK, 0x00, 0x00, 0x00, 0x00, 0x00 };
ALIGN16( static byte SIMD_SSE2_byte_colorMask2[16] ) = { 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00 };
ALIGN16( static byte SIMD_SSE2_byte_ctx1Mask[16] ) = { 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
ALIGN16( static byte SIMD_SSE2_byte_diagonalMask[16] ) = { 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
ALIGN16( static byte SIMD_SSE2_byte_scale_mask0[16] ) = { 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF };
ALIGN16( static byte SIMD_SSE2_byte_scale_mask1[16] ) = { 0xFF, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00 };
ALIGN16( static byte SIMD_SSE2_byte_scale_mask2[16] ) = { 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00 };
ALIGN16( static byte SIMD_SSE2_byte_scale_mask3[16] ) = { 0xFF, 0xFF, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00 };
ALIGN16( static byte SIMD_SSE2_byte_scale_mask4[16] ) = { 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00 };
ALIGN16( static byte SIMD_SSE2_byte_minus_128_0[16] ) = { (byte)-128, (byte)-128, 0, 0, (byte)-128, (byte)-128, 0, 0, (byte)-128, (byte)-128, 0, 0, (byte)-128, (byte)-128, 0, 0 };

/*
========================
idDxtEncoder::ExtractBlock_SSE2

params:	inPtr		- input image, 4 bytes per pixel
paramO:	colorBlock	- 4*4 output tile, 4 bytes per pixel
========================
*/
ID_INLINE void idDxtEncoder::ExtractBlock_SSE2( const byte * inPtr, int width, byte * colorBlock ) const {
#if ( defined( ID_WIN_X86_ASM ) || defined( ID_MAC_X86_ASM ) )
	__asm {
		mov			esi, inPtr
		mov			edi, colorBlock
		mov			eax, width
		shl			eax, 2
		movdqa		xmm0, xmmword ptr [esi]
		movdqa		xmmword ptr [edi+ 0], xmm0
		movdqa		xmm1, xmmword ptr [esi+eax]			// + 4 * width
		movdqa		xmmword ptr [edi+16], xmm1
		movdqa		xmm2, xmmword ptr [esi+eax*2]		// + 8 * width
		add			esi, eax
		movdqa		xmmword ptr [edi+32], xmm2
		movdqa		xmm3, xmmword ptr [esi+eax*2]		// + 12 * width
		movdqa		xmmword ptr [edi+48], xmm3
	}
#elif defined ( ID_QNX_X86_SSE2_ASM )
	__asm__ __volatile__(
			"shl	$2, %[width]\n"
			"movdqa	(%[in]), %%xmm0\n"
			"movdqa	%%xmm0,  0(%[color])\n"
			"movdqa	(%[in],%[width]), %%xmm1\n"			// + 4 * width
			"movdqa	%%xmm1, 16(%[color])\n"
			"movdqa	(%[in],%[width],2), %%xmm2\n"		// + 8 * width
			"mov	%[width], %[in]\n"
			"movdqa	%%xmm2, 32(%[color])\n"
			"movdqa	(%[in],%[width],2), %%xmm3\n"		// + 12 * width
			"movdqa	%%xmm3, 48(%[color])\n"
			:: [in] "r" (inPtr), [color] "r" (colorBlock), [width] "r" (width) : "xmm0", "xmm1", "xmm2", "xmm3", "memory");
#elif defined ( ID_WIN_X86_SSE2_INTRIN ) || defined ( ID_QNX_X86_SSE2_INTRIN )
	*((__m128i *)(&colorBlock[ 0])) = _mm_load_si128( (__m128i *)( inPtr + width * 4 * 0 ) );
	*((__m128i *)(&colorBlock[16])) = _mm_load_si128( (__m128i *)( inPtr + width * 4 * 1 ) );
	*((__m128i *)(&colorBlock[32])) = _mm_load_si128( (__m128i *)( inPtr + width * 4 * 2 ) );
	*((__m128i *)(&colorBlock[48])) = _mm_load_si128( (__m128i *)( inPtr + width * 4 * 3 ) );
#else
	assert( false );
#endif
}

/*
========================
idDxtEncoder::GetMinMaxBBox_SSE2

Takes the extents of the bounding box of the colors in the 4x4 block.

params:	colorBlock	- 4*4 input tile, 4 bytes per pixel
paramO:	minColor	- Min 4 byte output color
paramO:	maxColor	- Max 4 byte output color
========================
*/
ID_INLINE void idDxtEncoder::GetMinMaxBBox_SSE2( const byte * colorBlock, byte * minColor, byte * maxColor ) const {
#if ( defined( ID_WIN_X86_ASM ) || defined( ID_MAC_X86_ASM ) )
	__asm {
		mov			eax, colorBlock
		mov			esi, minColor
		mov			edi, maxColor
		movdqa		xmm0, xmmword ptr [eax+ 0]
		movdqa		xmm1, xmmword ptr [eax+ 0]
		pminub		xmm0, xmmword ptr [eax+16]
		pmaxub		xmm1, xmmword ptr [eax+16]
		pminub		xmm0, xmmword ptr [eax+32]
		pmaxub		xmm1, xmmword ptr [eax+32]
		pminub		xmm0, xmmword ptr [eax+48]
		pmaxub		xmm1, xmmword ptr [eax+48]
		pshufd		xmm3, xmm0, R_SHUFFLE_D( 2, 3, 2, 3 )
		pshufd		xmm4, xmm1, R_SHUFFLE_D( 2, 3, 2, 3 )
		pminub		xmm0, xmm3
		pmaxub		xmm1, xmm4
		pshuflw		xmm6, xmm0, R_SHUFFLE_D( 2, 3, 2, 3 )
		pshuflw		xmm7, xmm1, R_SHUFFLE_D( 2, 3, 2, 3 )
		pminub		xmm0, xmm6
		pmaxub		xmm1, xmm7
		movd		dword ptr [esi], xmm0
		movd		dword ptr [edi], xmm1
	}
#elif defined ( ID_QNX_X86_SSE2_ASM )
	__asm__ __volatile__(
			"movdqa		 0(%[color]), %%xmm0\n"
			"movdqa		 0(%[color]), %%xmm1\n"
			"pminub		16(%[color]), %%xmm0\n"
			"pmaxub		16(%[color]), %%xmm1\n"
			"pminub		32(%[color]), %%xmm0\n"
			"pmaxub		32(%[color]), %%xmm1\n"
			"pminub		48(%[color]), %%xmm0\n"
			"pmaxub		48(%[color]), %%xmm1\n"
			"pshufd		%[shuf2323], %%xmm0, %%xmm3\n"
			"pshufd		%[shuf2323], %%xmm1, %%xmm4\n"
			"pminub		%%xmm3, %%xmm0\n"
			"pmaxub		%%xmm4, %%xmm1\n"
			"pshuflw	%[shuf2323], %%xmm0, %%xmm6\n"
			"pshuflw	%[shuf2323], %%xmm1, %%xmm7\n"
			"pminub		%%xmm6, %%xmm0\n"
			"pmaxub		%%xmm7, %%xmm1\n"
			"movd		%%xmm0, (%[min])\n"
			"movd		%%xmm1, (%[max])\n"
			:: [color] "r" (colorBlock), [min] "r" (minColor), [max] "r" (maxColor), [shuf2323] "i" R_SHUFFLE_D( 2, 3, 2, 3 )
			: "xmm0", "xmm1", "xmm3", "xmm4", "xmm6", "xmm7", "memory");
#elif defined ( ID_WIN_X86_SSE2_INTRIN ) || defined ( ID_QNX_X86_SSE2_INTRIN )
	__m128i block0 = *((__m128i *)(&colorBlock[ 0]));
	__m128i block1 = *((__m128i *)(&colorBlock[16]));
	__m128i block2 = *((__m128i *)(&colorBlock[32]));
	__m128i block3 = *((__m128i *)(&colorBlock[48]));

	__m128i max1 = _mm_max_epu8( block0, block1 );
	__m128i min1 = _mm_min_epu8( block0, block1 );
	__m128i max2 = _mm_max_epu8( block2, block3 );
	__m128i min2 = _mm_min_epu8( block2, block3 );

	__m128i max3 = _mm_max_epu8( max1, max2 );
	__m128i min3 = _mm_min_epu8( min1, min2 );

	__m128i max4 = _mm_shuffle_epi32( max3, R_SHUFFLE_D( 2, 3, 2, 3 ) );
	__m128i min4 = _mm_shuffle_epi32( min3, R_SHUFFLE_D( 2, 3, 2, 3 ) );

	__m128i max5 = _mm_max_epu8( max3, max4 );
	__m128i min5 = _mm_min_epu8( min3, min4 );

	__m128i max6 = _mm_shufflelo_epi16( max5, R_SHUFFLE_D( 2, 3, 2, 3 ) );
	__m128i min6 = _mm_shufflelo_epi16( min5, R_SHUFFLE_D( 2, 3, 2, 3 ) );

	max6 = _mm_max_epu8( max5, max6 );
	min6 = _mm_min_epu8( min5, min6 );

	*((int *)maxColor) = _mm_cvtsi128_si32( max6 );
	*((int *)minColor) = _mm_cvtsi128_si32( min6 );
#else
	assert( false );
#endif
}

/*
========================
idDxtEncoder::InsetColorsBBox_SSE2
========================
*/
ID_INLINE void idDxtEncoder::InsetColorsBBox_SSE2( byte * minColor, byte * maxColor ) const {
#if ( defined( ID_WIN_X86_ASM ) || defined( ID_MAC_X86_ASM ) )
	__asm {
		mov			esi, minColor
		mov			edi, maxColor
		movd		xmm0, dword ptr [esi]
		movd		xmm1, dword ptr [edi]
		punpcklbw	xmm0, SIMD_SSE2_byte_0
		punpcklbw	xmm1, SIMD_SSE2_byte_0
		movdqa		xmm2, xmm1
		psubw		xmm2, xmm0
		pmulhw		xmm2, SIMD_SSE2_word_insetShift
		paddw		xmm0, xmm2
		psubw		xmm1, xmm2
		packuswb	xmm0, xmm0
		packuswb	xmm1, xmm1
		movd		dword ptr [esi], xmm0
		movd		dword ptr [edi], xmm1
	}
#elif defined ( ID_QNX_X86_SSE2_ASM )
	__asm__ __volatile__(
			"movd		(%[min]), %%xmm0\n"
			"movd		(%[max]), %%xmm1\n"
			"punpcklbw	_ZL16SIMD_SSE2_byte_0, %%xmm0\n"
			"punpcklbw	_ZL16SIMD_SSE2_byte_0, %%xmm1\n"
			"movdqa		%%xmm1, %%xmm2\n"
			"psubw		%%xmm0, %%xmm2\n"
			"pmulhw		_ZL25SIMD_SSE2_word_insetShift, %%xmm2\n"
			"paddw		%%xmm2, %%xmm0\n"
			"psubw		%%xmm2, %%xmm1\n"
			"packuswb	%%xmm0, %%xmm0\n"
			"packuswb	%%xmm1, %%xmm1\n"
			"movd		%%xmm0, (%[min])\n"
			"movd		%%xmm1, (%[max])\n"
			:: [min] "r" (minColor), [max] "r" (maxColor) : "xmm0", "xmm1", "xmm2", "memory");
#elif defined ( ID_WIN_X86_SSE2_INTRIN ) || defined ( ID_QNX_X86_SSE2_INTRIN )
	__m128i min = _mm_cvtsi32_si128( *(int *)minColor );
	__m128i max = _mm_cvtsi32_si128( *(int *)maxColor );

	__m128i xmm0 = _mm_unpacklo_epi8( min, *(__m128i *)SIMD_SSE2_byte_0 );
	__m128i xmm1 = _mm_unpacklo_epi8( max, *(__m128i *)SIMD_SSE2_byte_0 );

	__m128i xmm2 = _mm_sub_epi16( xmm1, xmm0 );

	xmm2 = _mm_mulhi_epi16( xmm2, *(__m128i *)SIMD_SSE2_word_insetShift );

	xmm0 = _mm_add_epi16( xmm0, xmm2 );
	xmm1 = _mm_sub_epi16( xmm1, xmm2 );

	xmm0 = _mm_packus_epi16( xmm0, xmm0 );
	xmm1 = _mm_packus_epi16( xmm1, xmm1 );

	*((int *)minColor) = _mm_cvtsi128_si32( xmm0 );
	*((int *)maxColor) = _mm_cvtsi128_si32( xmm1 );
#else
	assert( false );
#endif
}

/*
========================
idDxtEncoder::EmitColorIndices_SSE2

params:	colorBlock	- 16 pixel block for which to find color indices
paramO:	minColor	- Min alpha found
paramO:	maxColor	- Max alpha found
return: 4 byte color index block
========================
*/
void idDxtEncoder::EmitColorIndices_SSE2( const byte * colorBlock, const byte * minColor_, const byte * maxColor_ ) {
#if ( defined( ID_WIN_X86_ASM ) || defined( ID_MAC_X86_ASM ) ) || defined ( ID_QNX_X86_SSE2_ASM )
	byte *outPtr = outData;

#if ( defined( ID_WIN_X86_ASM ) || defined( ID_MAC_X86_ASM ) )
	ALIGN16( byte color0[16] );
	ALIGN16( byte color1[16] );
	ALIGN16( byte color2[16] );
	ALIGN16( byte color3[16] );
	ALIGN16( byte result[16] );

	__asm {
		mov			esi, maxColor_
		mov			edi, minColor_
		pxor		xmm7, xmm7
		movdqa		result, xmm7

		movd		xmm0, dword ptr [esi]
		pand		xmm0, SIMD_SSE2_byte_colorMask
		punpcklbw	xmm0, xmm7
		pshuflw		xmm4, xmm0, R_SHUFFLE_D( 0, 3, 2, 3 )
		pshuflw		xmm5, xmm0, R_SHUFFLE_D( 3, 1, 3, 3 )
		psrlw		xmm4, 5
		psrlw		xmm5, 6
		por			xmm0, xmm4
		por			xmm0, xmm5

		movd		xmm1, dword ptr [edi]
		pand		xmm1, SIMD_SSE2_byte_colorMask
		punpcklbw	xmm1, xmm7
		pshuflw		xmm4, xmm1, R_SHUFFLE_D( 0, 3, 2, 3 )
		pshuflw		xmm5, xmm1, R_SHUFFLE_D( 3, 1, 3, 3 )
		psrlw		xmm4, 5
		psrlw		xmm5, 6
		por			xmm1, xmm4
		por			xmm1, xmm5

		movdqa		xmm2, xmm0
		packuswb	xmm2, xmm7
		pshufd		xmm2, xmm2, R_SHUFFLE_D( 0, 1, 0, 1 )
		movdqa		color0, xmm2

		movdqa		xmm6, xmm0
		paddw		xmm6, xmm0
		paddw		xmm6, xmm1
		pmulhw		xmm6, SIMD_SSE2_word_div_by_3	// * ( ( 1 << 16 ) / 3 + 1 ) ) >> 16
		packuswb	xmm6, xmm7
		pshufd		xmm6, xmm6, R_SHUFFLE_D( 0, 1, 0, 1 )
		movdqa		color2, xmm6

		movdqa		xmm3, xmm1
		packuswb	xmm3, xmm7
		pshufd		xmm3, xmm3, R_SHUFFLE_D( 0, 1, 0, 1 )
		movdqa		color1, xmm3

		paddw		xmm1, xmm1
		paddw		xmm0, xmm1
		pmulhw		xmm0, SIMD_SSE2_word_div_by_3	// * ( ( 1 << 16 ) / 3 + 1 ) ) >> 16
		packuswb	xmm0, xmm7
		pshufd		xmm0, xmm0, R_SHUFFLE_D( 0, 1, 0, 1 )
		movdqa		color3, xmm0

		mov			eax, 32
		mov			esi, colorBlock

	loop1:			// iterates 2 times
		movq		xmm3, qword ptr [esi+eax+0]
		pshufd		xmm3, xmm3, R_SHUFFLE_D( 0, 2, 1, 3 )		// punpckldq	xmm4, SIMD_SSE2_dword_0
		movq		xmm5, qword ptr [esi+eax+8]
		pshufd		xmm5, xmm5, R_SHUFFLE_D( 0, 2, 1, 3 )		// punpckldq	xmm5, SIMD_SSE2_dword_0

		movdqa		xmm0, xmm3
		movdqa		xmm6, xmm5
		psadbw		xmm0, color0
		psadbw		xmm6, color0
		packssdw	xmm0, xmm6
		movdqa		xmm1, xmm3
		movdqa		xmm6, xmm5
		psadbw		xmm1, color1
		psadbw		xmm6, color1
		packssdw	xmm1, xmm6
		movdqa		xmm2, xmm3
		movdqa		xmm6, xmm5
		psadbw		xmm2, color2
		psadbw		xmm6, color2
		packssdw	xmm2, xmm6
		psadbw		xmm3, color3
		psadbw		xmm5, color3
		packssdw	xmm3, xmm5

		movq		xmm4, qword ptr [esi+eax+16]
		pshufd		xmm4, xmm4, R_SHUFFLE_D( 0, 2, 1, 3 )
		movq		xmm5, qword ptr [esi+eax+24]
		pshufd		xmm5, xmm5, R_SHUFFLE_D( 0, 2, 1, 3 )

		movdqa		xmm6, xmm4
		movdqa		xmm7, xmm5
		psadbw		xmm6, color0
		psadbw		xmm7, color0
		packssdw	xmm6, xmm7
		packssdw	xmm0, xmm6				// d1
		movdqa		xmm6, xmm4
		movdqa		xmm7, xmm5
		psadbw		xmm6, color1
		psadbw		xmm7, color1
		packssdw	xmm6, xmm7
		packssdw	xmm1, xmm6				// d1
		movdqa		xmm6, xmm4
		movdqa		xmm7, xmm5
		psadbw		xmm6, color2
		psadbw		xmm7, color2
		packssdw	xmm6, xmm7
		packssdw	xmm2, xmm6				// d2
		psadbw		xmm4, color3
		psadbw		xmm5, color3
		packssdw	xmm4, xmm5
		packssdw	xmm3, xmm4				// d3

		movdqa		xmm7, result
		pslld		xmm7, 16

		movdqa		xmm4, xmm0
		movdqa		xmm5, xmm1
		pcmpgtw		xmm0, xmm3				// b0
		pcmpgtw		xmm1, xmm2				// b1
		pcmpgtw		xmm4, xmm2				// b2
		pcmpgtw		xmm5, xmm3				// b3
		pcmpgtw		xmm2, xmm3				// b4
		pand		xmm4, xmm1				// x0
		pand		xmm5, xmm0				// x1
		pand		xmm2, xmm0				// x2
		por			xmm4, xmm5
		pand		xmm2, SIMD_SSE2_word_1
		pand		xmm4, SIMD_SSE2_word_2
		por			xmm2, xmm4

		pshufd		xmm5, xmm2, R_SHUFFLE_D( 2, 3, 0, 1 )
		punpcklwd	xmm2, SIMD_SSE2_word_0
		punpcklwd	xmm5, SIMD_SSE2_word_0
		pslld		xmm5, 8
		por			xmm7, xmm5
		por			xmm7, xmm2
		movdqa		result, xmm7

		sub			eax, 32
		jge			loop1

		mov			esi, outPtr
		pshufd		xmm4, xmm7, R_SHUFFLE_D( 1, 2, 3, 0 )
		pshufd		xmm5, xmm7, R_SHUFFLE_D( 2, 3, 0, 1 )
		pshufd		xmm6, xmm7, R_SHUFFLE_D( 3, 0, 1, 2 )
		pslld		xmm4, 2
		pslld		xmm5, 4
		pslld		xmm6, 6
		por			xmm7, xmm4
		por			xmm7, xmm5
		por			xmm7, xmm6
		movd		dword ptr [esi], xmm7
	}
#else
	ALIGN16( byte data[16 * 5] ); // 5 separate arrays end up using too many registers for inline ASM, so consolidate them.

	__asm__ __volatile__(
			"pxor		%%xmm7, %%xmm7\n"
			"movdqa		%%xmm7, 64(%[data])\n"

			"movd		(%[maxColor]), %%xmm0\n"
			"pand		_ZL24SIMD_SSE2_byte_colorMask, %%xmm0\n"
			"punpcklbw	%%xmm7, %%xmm0\n"
			"pshuflw	%[shuf0323], %%xmm0, %%xmm4\n"
			"pshuflw	%[shuf3133], %%xmm0, %%xmm5\n"
			"psrlw		$5, %%xmm4\n"
			"psrlw		$6, %%xmm5\n"
			"por		%%xmm4, %%xmm0\n"
			"por		%%xmm5, %%xmm0\n"

			"movd		(%[minColor]), %%xmm1\n"
			"pand		_ZL24SIMD_SSE2_byte_colorMask, %%xmm1\n"
			"punpcklbw	%%xmm7, %%xmm1\n"
			"pshuflw	%[shuf0323], %%xmm1, %%xmm4\n"
			"pshuflw	%[shuf3133], %%xmm1, %%xmm5\n"
			"psrlw		$5, %%xmm4\n"
			"psrlw		$6, %%xmm5\n"
			"por		%%xmm4, %%xmm1\n"
			"por		%%xmm5, %%xmm1\n"

			"movdqa		%%xmm0, %%xmm2\n"
			"packuswb	%%xmm7, %%xmm2\n"
			"pshufd		%[shuf0101], %%xmm2, %%xmm2\n"
			"movdqa		%%xmm2, (%[data])\n"

			"movdqa		%%xmm0, %%xmm6\n"
			"paddw		%%xmm0, %%xmm6\n"
			"paddw		%%xmm1, %%xmm6\n"
			"pmulhw		_ZL23SIMD_SSE2_word_div_by_3, %%xmm6\n"	// * ( ( 1 << 16 ) / 3 + 1 ) ) >> 16
			"packuswb	%%xmm7, %%xmm6\n"
			"pshufd		%[shuf0101], %%xmm6, %%xmm6\n"
			"movdqa		%%xmm6, 32(%[data])\n"

			"movdqa		%%xmm1, %%xmm3\n"
			"packuswb	%%xmm7, %%xmm3\n"
			"pshufd		%[shuf0101], %%xmm3, %%xmm3\n"
			"movdqa		%%xmm3, 16(%[data])\n"

			"paddw		%%xmm1, %%xmm1\n"
			"paddw		%%xmm1, %%xmm0\n"
			"pmulhw		_ZL23SIMD_SSE2_word_div_by_3, %%xmm0\n"	// * ( ( 1 << 16 ) / 3 + 1 ) ) >> 16
			"packuswb	%%xmm7, %%xmm0\n"
			"pshufd		%[shuf0101], %%xmm0, %%xmm0\n"
			"movdqa		%%xmm0, 48(%[data])\n"

			"mov		$32, %%eax\n"

		"loop1_EmitColorIndices_SSE2:\n"	// iterates 2 times
			"movq		0(%[colorBlock],%%eax), %%xmm3\n"
			"pshufd		%[shuf0213], %%xmm3, %%xmm3\n"			// punpckldq	xmm4, SIMD_SSE2_dword_0
			"movq		8(%[colorBlock],%%eax), %%xmm5\n"
			"pshufd		%[shuf0213], %%xmm5, %%xmm5\n"			// punpckldq	xmm5, SIMD_SSE2_dword_0

			"movdqa		%%xmm3, %%xmm0\n"
			"movdqa		%%xmm5, %%xmm6\n"
			"psadbw		(%[data]), %%xmm0\n"
			"psadbw		(%[data]), %%xmm6\n"
			"packssdw	%%xmm6, %%xmm0\n"
			"movdqa		%%xmm3, %%xmm1\n"
			"movdqa		%%xmm5, %%xmm6\n"
			"psadbw		16(%[data]), %%xmm1\n"
			"psadbw		16(%[data]), %%xmm6\n"
			"packssdw	%%xmm6, %%xmm1\n"
			"movdqa		%%xmm3, %%xmm2\n"
			"movdqa		%%xmm5, %%xmm6\n"
			"psadbw		32(%[data]), %%xmm2\n"
			"psadbw		32(%[data]), %%xmm6\n"
			"packssdw	%%xmm6, %%xmm2\n"
			"psadbw		48(%[data]), %%xmm3\n"
			"psadbw		48(%[data]), %%xmm5\n"
			"packssdw	%%xmm5, %%xmm3\n"

			"movq		16(%[colorBlock],%%eax), %%xmm4\n"
			"pshufd		%[shuf0213], %%xmm4, %%xmm4\n"
			"movq		24(%[colorBlock],%%eax), %%xmm5\n"
			"pshufd		%[shuf0213], %%xmm5, %%xmm5\n"

			"movdqa		%%xmm4, %%xmm6\n"
			"movdqa		%%xmm5, %%xmm7\n"
			"psadbw		(%[data]), %%xmm6\n"
			"psadbw		(%[data]), %%xmm7\n"
			"packssdw	%%xmm7, %%xmm6\n"
			"packssdw	%%xmm6, %%xmm0\n"						// d1
			"movdqa		%%xmm4, %%xmm6\n"
			"movdqa		%%xmm5, %%xmm7\n"
			"psadbw		16(%[data]), %%xmm6\n"
			"psadbw		16(%[data]), %%xmm7\n"
			"packssdw	%%xmm7, %%xmm6\n"
			"packssdw	%%xmm6, %%xmm1\n"						// d1
			"movdqa		%%xmm4, %%xmm6\n"
			"movdqa		%%xmm5, %%xmm7\n"
			"psadbw		32(%[data]), %%xmm6\n"
			"psadbw		32(%[data]), %%xmm7\n"
			"packssdw	%%xmm7, %%xmm6\n"
			"packssdw	%%xmm6, %%xmm2\n"						// d2
			"psadbw		48(%[data]), %%xmm4\n"
			"psadbw		48(%[data]), %%xmm5\n"
			"packssdw	%%xmm5, %%xmm4\n"
			"packssdw	%%xmm4, %%xmm3\n"						// d3

			"movdqa		64(%[data]), %%xmm7\n"
			"pslld		$16, %%xmm7\n"

			"movdqa		%%xmm0, %%xmm4\n"
			"movdqa		%%xmm1, %%xmm5\n"
			"pcmpgtw	%%xmm3, %%xmm0\n"						// b0
			"pcmpgtw	%%xmm2, %%xmm1\n"						// b1
			"pcmpgtw	%%xmm2, %%xmm4\n"						// b2
			"pcmpgtw	%%xmm3, %%xmm5\n"						// b3
			"pcmpgtw	%%xmm3, %%xmm2\n"						// b4
			"pand		%%xmm1, %%xmm4\n"						// x0
			"pand		%%xmm0, %%xmm5\n"						// x1
			"pand		%%xmm0, %%xmm2\n"						// x2
			"por		%%xmm5, %%xmm4\n"
			"pand		_ZL16SIMD_SSE2_word_1, %%xmm2\n"
			"pand		_ZL16SIMD_SSE2_word_2, %%xmm4\n"
			"por		%%xmm4, %%xmm2\n"

			"pshufd		%[shuf2301], %%xmm2, %%xmm5\n"
			"punpcklwd	_ZL16SIMD_SSE2_word_0, %%xmm2\n"
			"punpcklwd	_ZL16SIMD_SSE2_word_0, %%xmm5\n"
			"pslld		$8, %%xmm5\n"
			"por		%%xmm5, %%xmm7\n"
			"por		%%xmm2, %%xmm7\n"
			"movdqa		%%xmm7, 64(%[data])\n"

			"sub		$32, %%eax\n"
			"jge		loop1_EmitColorIndices_SSE2\n"

			"pshufd		%[shuf1230], %%xmm7, %%xmm4\n"
			"pshufd		%[shuf2301], %%xmm7, %%xmm5\n"
			"pshufd		%[shuf3012], %%xmm7, %%xmm6\n"
			"pslld		$2, %%xmm4\n"
			"pslld		$4, %%xmm5\n"
			"pslld		$6, %%xmm6\n"
			"por		%%xmm4, %%xmm7\n"
			"por		%%xmm5, %%xmm7\n"
			"por		%%xmm6, %%xmm7\n"
			"movd		%%xmm7, (%[outPtr])\n"
			:: [colorBlock] "r" (colorBlock), [minColor] "r" (minColor_), [maxColor] "r" (maxColor_), [outPtr] "r" (outPtr), [data] "r" (data),
			[shuf0101] "i" R_SHUFFLE_D( 0, 1, 0, 1 ), [shuf0213] "i" R_SHUFFLE_D( 0, 2, 1, 3 ), [shuf0323] "i" R_SHUFFLE_D( 0, 3, 2, 3 ), [shuf3133] "i" R_SHUFFLE_D( 3, 1, 3, 3 ),
			[shuf2301] "i" R_SHUFFLE_D( 2, 3, 0, 1 ), [shuf1230] "i" R_SHUFFLE_D( 1, 2, 3, 0 ), [shuf3012] "i" R_SHUFFLE_D( 3, 0, 1, 2 )
			: "eax", "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "memory", "cc");
#endif

	outData += 4;
#elif defined ( ID_WIN_X86_SSE2_INTRIN ) || defined ( ID_QNX_X86_SSE2_INTRIN )
	__m128c zero = SIMD_SSE2_zero;
	__m128c result = SIMD_SSE2_zero;
	__m128c color0, color1, color2, color3;
	__m128c temp0, temp1, temp2, temp3, temp4, temp5, temp6, temp7;
	__m128c minColor = _mm_cvtsi32_si128( *(int *)minColor_ );
	__m128c maxColor = _mm_cvtsi32_si128( *(int *)maxColor_ );
	__m128c blocka[2], blockb[2];
	blocka[0] = *((__m128i *)(&colorBlock[ 0]));
	blocka[1] = *((__m128i *)(&colorBlock[32]));
	blockb[0] = *((__m128i *)(&colorBlock[16]));
	blockb[1] = *((__m128i *)(&colorBlock[48]));

	temp0 = _mm_and_si128( maxColor, (const __m128i &)SIMD_SSE2_byte_colorMask );
	temp0 = _mm_unpacklo_epi8( temp0, zero );
	temp4 = _mm_shufflelo_epi16( temp0, R_SHUFFLE_D( 0, 3, 2, 3 ) );
	temp5 = _mm_shufflelo_epi16( temp0, R_SHUFFLE_D( 3, 1, 3, 3 ) );
	temp4 = _mm_srli_epi16( temp4, 5 );
	temp5 = _mm_srli_epi16( temp5, 6 );
	temp0 = _mm_or_si128( temp0, temp4 );
	temp0 = _mm_or_si128( temp0, temp5 );


	temp1 = _mm_and_si128( minColor, (const __m128i &)SIMD_SSE2_byte_colorMask );
	temp1 = _mm_unpacklo_epi8( temp1, zero );
	temp4 = _mm_shufflelo_epi16( temp1, R_SHUFFLE_D( 0, 3, 2, 3 ) );
	temp5 = _mm_shufflelo_epi16( temp1, R_SHUFFLE_D( 3, 1, 3, 3 ) );
	temp4 = _mm_srli_epi16( temp4, 5 );
	temp5 = _mm_srli_epi16( temp5, 6 );
	temp1 = _mm_or_si128( temp1, temp4 );
	temp1 = _mm_or_si128( temp1, temp5 );


	temp2 = _mm_packus_epi16( temp0, zero );
	color0 = _mm_shuffle_epi32( temp2, R_SHUFFLE_D( 0, 1, 0, 1 ) );

	temp6 = _mm_add_epi16( temp0, temp0 );
	temp6 = _mm_add_epi16( temp6, temp1 );
	temp6 = _mm_mulhi_epi16( temp6, (const __m128i &)SIMD_SSE2_word_div_by_3 );		// * ( ( 1 << 16 ) / 3 + 1 ) ) >> 16
	temp6 = _mm_packus_epi16( temp6, zero );
	color2 = _mm_shuffle_epi32( temp6, R_SHUFFLE_D( 0, 1, 0, 1 ) );

	temp3 = _mm_packus_epi16( temp1, zero );
	color1 = _mm_shuffle_epi32( temp3, R_SHUFFLE_D( 0, 1, 0, 1 ) );

	temp1 = _mm_add_epi16( temp1, temp1 );
	temp0 = _mm_add_epi16( temp0, temp1 );
	temp0 = _mm_mulhi_epi16( temp0, (const __m128i &)SIMD_SSE2_word_div_by_3 );		// * ( ( 1 << 16 ) / 3 + 1 ) ) >> 16
	temp0 = _mm_packus_epi16( temp0, zero );
	color3 = _mm_shuffle_epi32( temp0, R_SHUFFLE_D( 0, 1, 0, 1 ) );

	for ( int i = 1; i >= 0; i-- ) {
		// Load block
		temp3 = _mm_shuffle_epi32( blocka[i], R_SHUFFLE_D( 0, 2, 1, 3 ) );
		temp5 = _mm_shuffle_ps( blocka[i], zero, R_SHUFFLE_D( 2, 3, 0, 1 ) );
		temp5 = _mm_shuffle_epi32( temp5, R_SHUFFLE_D( 0, 2, 1, 3 ) );

		temp0 = _mm_sad_epu8( temp3, color0 );
		temp6 = _mm_sad_epu8( temp5, color0 );
		temp0 = _mm_packs_epi32( temp0, temp6 );

		temp1 = _mm_sad_epu8( temp3, color1 );
		temp6 = _mm_sad_epu8( temp5, color1 );
		temp1 = _mm_packs_epi32( temp1, temp6 );

		temp2 = _mm_sad_epu8( temp3, color2 );
		temp6 = _mm_sad_epu8( temp5, color2 );
		temp2 = _mm_packs_epi32( temp2, temp6 );

		temp3 = _mm_sad_epu8( temp3, color3 );
		temp5 = _mm_sad_epu8( temp5, color3 );
		temp3 = _mm_packs_epi32( temp3, temp5 );

		// Load block
		temp4 = _mm_shuffle_epi32( blockb[i], R_SHUFFLE_D( 0, 2, 1, 3 ) );
		temp5 = _mm_shuffle_ps( blockb[i], zero, R_SHUFFLE_D( 2, 3, 0, 1 ) );
		temp5 = _mm_shuffle_epi32( temp5, R_SHUFFLE_D( 0, 2, 1, 3 ) );

		temp6 = _mm_sad_epu8( temp4, color0 );
		temp7 = _mm_sad_epu8( temp5, color0 );
		temp6 = _mm_packs_epi32( temp6, temp7 );
		temp0 = _mm_packs_epi32( temp0, temp6 );	// d0

		temp6 = _mm_sad_epu8( temp4, color1 );
		temp7 = _mm_sad_epu8( temp5, color1 );
		temp6 = _mm_packs_epi32( temp6, temp7 );
		temp1 = _mm_packs_epi32( temp1, temp6 );	// d1

		temp6 = _mm_sad_epu8( temp4, color2 );
		temp7 = _mm_sad_epu8( temp5, color2 );
		temp6 = _mm_packs_epi32( temp6, temp7 );
		temp2 = _mm_packs_epi32( temp2, temp6 );	// d2

		temp4 = _mm_sad_epu8( temp4, color3 );
		temp5 = _mm_sad_epu8( temp5, color3 );
		temp4 = _mm_packs_epi32( temp4, temp5 );
		temp3 = _mm_packs_epi32( temp3, temp4 );	// d3

		temp7 = _mm_slli_epi32( result, 16 );

		temp4 = _mm_cmpgt_epi16( temp0, temp2 );	// b2
		temp5 = _mm_cmpgt_epi16( temp1, temp3 );	// b3
		temp0 = _mm_cmpgt_epi16( temp0, temp3 );	// b0
		temp1 = _mm_cmpgt_epi16( temp1, temp2 );	// b1
		temp2 = _mm_cmpgt_epi16( temp2, temp3 );	// b4

		temp4 = _mm_and_si128( temp4, temp1 );		// x0
		temp5 = _mm_and_si128( temp5, temp0 );		// x1
		temp2 = _mm_and_si128( temp2, temp0 );		// x2
		temp4 = _mm_or_si128( temp4, temp5 );
		temp2 = _mm_and_si128( temp2, (const __m128i &)SIMD_SSE2_word_1 );
		temp4 = _mm_and_si128( temp4, (const __m128i &)SIMD_SSE2_word_2 );
		temp2 = _mm_or_si128( temp2, temp4 );

		temp5 = _mm_shuffle_epi32( temp2, R_SHUFFLE_D( 2, 3, 0, 1 ) );
		temp2 = _mm_unpacklo_epi16( temp2, (const __m128i &)SIMD_SSE2_word_0 );
		temp5 = _mm_unpacklo_epi16( temp5, (const __m128i &)SIMD_SSE2_word_0 );
		temp5 = _mm_slli_epi32( temp5, 8 );
		temp7 = _mm_or_si128( temp7, temp5 );
		result = _mm_or_si128( temp7, temp2 );
	}

	temp4 = _mm_shuffle_epi32( result, R_SHUFFLE_D( 1, 2, 3, 0 ) );
	temp5 = _mm_shuffle_epi32( result, R_SHUFFLE_D( 2, 3, 0, 1 ) );
	temp6 = _mm_shuffle_epi32( result, R_SHUFFLE_D( 3, 0, 1, 2 ) );
	temp4 = _mm_slli_epi32( temp4, 2 );
	temp5 = _mm_slli_epi32( temp5, 4 );
	temp6 = _mm_slli_epi32( temp6, 6 );
	temp7 = _mm_or_si128( result, temp4 );
	temp7 = _mm_or_si128( temp7, temp5 );
	temp7 = _mm_or_si128( temp7, temp6 );

	unsigned int out = _mm_cvtsi128_si32( temp7 );
	EmitUInt( out );
#else
	assert( false );
#endif
}

/*
========================
idDxtEncoder::EmitColorAlphaIndices_SSE2

params:	colorBlock	- 16 pixel block for which find color indexes
paramO:	minColor	- Min color found
paramO:	maxColor	- Max color found
return: 4 byte color index block
========================
*/
void idDxtEncoder::EmitColorAlphaIndices_SSE2( const byte *colorBlock, const byte *minColor_, const byte *maxColor_ ) {
#if ( defined( ID_WIN_X86_ASM ) || defined( ID_MAC_X86_ASM ) ) || defined ( ID_QNX_X86_SSE2_ASM )
	byte *outPtr = outData;

#if ( defined( ID_WIN_X86_ASM ) || defined( ID_MAC_X86_ASM ) )
	ALIGN16( byte color0[16] );
	ALIGN16( byte color1[16] );
	ALIGN16( byte color2[16] );
	ALIGN16( byte color3[16] );
	ALIGN16( byte result[16] );

	__asm {
		mov			esi, maxColor_
		mov			edi, minColor_
		pxor		xmm7, xmm7
		movdqa		result, xmm7

		movd		xmm0, dword ptr [esi]
		pand		xmm0, SIMD_SSE2_byte_colorMask
		punpcklbw	xmm0, xmm7
		pshuflw		xmm4, xmm0, R_SHUFFLE_D( 0, 3, 2, 3 )
		pshuflw		xmm5, xmm0, R_SHUFFLE_D( 3, 1, 3, 3 )
		psrlw		xmm4, 5
		psrlw		xmm5, 6
		por			xmm0, xmm4
		por			xmm0, xmm5

		movd		xmm1, dword ptr [edi]
		pand		xmm1, SIMD_SSE2_byte_colorMask
		punpcklbw	xmm1, xmm7
		pshuflw		xmm4, xmm1, R_SHUFFLE_D( 0, 3, 2, 3 )
		pshuflw		xmm5, xmm1, R_SHUFFLE_D( 3, 1, 3, 3 )
		psrlw		xmm4, 5
		psrlw		xmm5, 6
		por			xmm1, xmm4
		por			xmm1, xmm5

		movdqa		xmm2, xmm0
		packuswb	xmm2, xmm7
		pshufd		xmm2, xmm2, R_SHUFFLE_D( 0, 1, 0, 1 )
		movdqa		color0, xmm2

		movdqa		xmm6, xmm0
		paddw		xmm6, xmm1
		psrlw		xmm6, 1
		packuswb	xmm6, xmm7
		pshufd		xmm6, xmm6, R_SHUFFLE_D( 0, 1, 0, 1 )
		movdqa		color2, xmm6

		movdqa		xmm3, xmm1
		packuswb	xmm3, xmm7
		pshufd		xmm3, xmm3, R_SHUFFLE_D( 0, 1, 0, 1 )
		movdqa		color1, xmm3

		movdqa		color3, xmm7

		mov			eax, 32
		mov			esi, colorBlock

	loop1:			// iterates 2 times
		movq		xmm3, qword ptr [esi+eax+0]
		pshufd		xmm3, xmm3, R_SHUFFLE_D( 0, 2, 1, 3 )
		movq		xmm5, qword ptr [esi+eax+8]
		pshufd		xmm5, xmm5, R_SHUFFLE_D( 0, 2, 1, 3 )

		movdqa		xmm0, xmm3
		movdqa		xmm6, xmm5
		psadbw		xmm0, color0
		psadbw		xmm6, color0
		packssdw	xmm0, xmm6
		movdqa		xmm1, xmm3
		movdqa		xmm6, xmm5
		psadbw		xmm1, color1
		psadbw		xmm6, color1
		packssdw	xmm1, xmm6
		movdqa		xmm2, xmm3
		movdqa		xmm6, xmm5
		psadbw		xmm2, color2
		psadbw		xmm6, color2
		packssdw	xmm2, xmm6

		shufps		xmm3, xmm5, R_SHUFFLE_D( 0, 2, 0, 2 )
		psrld		xmm3, 24
		packssdw	xmm3, xmm3

		movq		xmm4, qword ptr [esi+eax+16]
		pshufd		xmm4, xmm4, R_SHUFFLE_D( 0, 2, 1, 3 )
		movq		xmm5, qword ptr [esi+eax+24]
		pshufd		xmm5, xmm5, R_SHUFFLE_D( 0, 2, 1, 3 )

		movdqa		xmm6, xmm4
		movdqa		xmm7, xmm5
		psadbw		xmm6, color0
		psadbw		xmm7, color0
		packssdw	xmm6, xmm7
		packssdw	xmm0, xmm6					// d1
		movdqa		xmm6, xmm4
		movdqa		xmm7, xmm5
		psadbw		xmm6, color1
		psadbw		xmm7, color1
		packssdw	xmm6, xmm7
		packssdw	xmm1, xmm6					// d1
		movdqa		xmm6, xmm4
		movdqa		xmm7, xmm5
		psadbw		xmm6, color2
		psadbw		xmm7, color2
		packssdw	xmm6, xmm7
		packssdw	xmm2, xmm6					// d2

		shufps		xmm4, xmm5, R_SHUFFLE_D( 0, 2, 0, 2 )
		psrld		xmm4, 24
		packssdw	xmm4, xmm4

		punpcklqdq	xmm3, xmm4					// c3

		movdqa		xmm7, result
		pslld		xmm7, 16

		movdqa		xmm4, xmm2
		pcmpgtw		xmm2, xmm0					// b0
		pcmpgtw		xmm4, xmm1					// b1
		pcmpgtw		xmm1, xmm0					// b2
		pmaxsw		xmm3, SIMD_SSE2_word_127	// b3
		pcmpeqw		xmm3, SIMD_SSE2_word_127

		pand		xmm2, xmm4
		por			xmm2, xmm3					// b0 & b1 | b3
		pxor		xmm1, xmm4
		por			xmm1, xmm3					// b2 ^ b1 | b3
		pand		xmm2, SIMD_SSE2_word_2
		pand		xmm1, SIMD_SSE2_word_1
		por			xmm2, xmm1

		pshufd		xmm5, xmm2, R_SHUFFLE_D( 2, 3, 0, 1 )
		punpcklwd	xmm2, SIMD_SSE2_word_0
		punpcklwd	xmm5, SIMD_SSE2_word_0
		pslld		xmm5, 8
		por			xmm7, xmm5
		por			xmm7, xmm2
		movdqa		result, xmm7

		sub			eax, 32
		jge			loop1

		mov			esi, outPtr
		pshufd		xmm4, xmm7, R_SHUFFLE_D( 1, 2, 3, 0 )
		pshufd		xmm5, xmm7, R_SHUFFLE_D( 2, 3, 0, 1 )
		pshufd		xmm6, xmm7, R_SHUFFLE_D( 3, 0, 1, 2 )
		pslld		xmm4, 2
		pslld		xmm5, 4
		pslld		xmm6, 6
		por			xmm7, xmm4
		por			xmm7, xmm5
		por			xmm7, xmm6
		movd		dword ptr [esi], xmm7
	}
#else
	ALIGN16( byte data[16 * 5] ); // 5 separate arrays end up using too many registers for inline ASM, so consolidate them.

	__asm__ __volatile__(
			"pxor		%%xmm7, %%xmm7\n"
			"movdqa		%%xmm7, 64(%[data])\n"

			"movd		(%[maxColor]), %%xmm0\n"
			"pand		_ZL24SIMD_SSE2_byte_colorMask, %%xmm0\n"
			"punpcklbw	%%xmm7, %%xmm0\n"
			"pshuflw	%[shuf0323], %%xmm0, %%xmm4\n"
			"pshuflw	%[shuf3133], %%xmm0, %%xmm5\n"
			"psrlw		$5, %%xmm4\n"
			"psrlw		$6, %%xmm5\n"
			"por		%%xmm4, %%xmm0\n"
			"por		%%xmm5, %%xmm0\n"

			"movd		(%[minColor]), %%xmm1\n"
			"pand		_ZL24SIMD_SSE2_byte_colorMask, %%xmm1\n"
			"punpcklbw	%%xmm7, %%xmm1\n"
			"pshuflw	%[shuf0323], %%xmm1, %%xmm4\n"
			"pshuflw	%[shuf3133], %%xmm1, %%xmm5\n"
			"psrlw		$5, %%xmm4\n"
			"psrlw		$6, %%xmm5\n"
			"por		%%xmm4, %%xmm1\n"
			"por		%%xmm5, %%xmm1\n"

			"movdqa		%%xmm0, %%xmm2\n"
			"packuswb	%%xmm7, %%xmm2\n"
			"pshufd		%[shuf0101], %%xmm2, %%xmm2\n"
			"movdqa		%%xmm2, (%[data])\n"

			"movdqa		%%xmm0, %%xmm6\n"
			"paddw		%%xmm1, %%xmm6\n"
			"psrlw		$1, %%xmm6\n"
			"packuswb	%%xmm7, %%xmm6\n"
			"pshufd		%[shuf0101], %%xmm6, %%xmm6\n"
			"movdqa		%%xmm6, 32(%[data])\n"

			"movdqa		%%xmm1, %%xmm3\n"
			"packuswb	%%xmm7, %%xmm3\n"
			"pshufd		%[shuf0101], %%xmm3, %%xmm3\n"
			"movdqa		%%xmm3, 16(%[data])\n"

			"movdqa		%%xmm7, 48(%[data])\n"

			"mov		$32, %%eax\n"

		"loop1_EmitColorAlphaIndices_SSE2:\n"	// iterates 2 times
			"movq		0(%[colorBlock],%%eax), %%xmm3\n"
			"pshufd		%[shuf0213], %%xmm3, %%xmm3\n"
			"movq		8(%[colorBlock],%%eax), %%xmm5\n"
			"pshufd		%[shuf0213], %%xmm5, %%xmm5\n"

			"movdqa		%%xmm3, %%xmm0\n"
			"movdqa		%%xmm5, %%xmm6\n"
			"psadbw		(%[data]), %%xmm0\n"
			"psadbw		(%[data]), %%xmm6\n"
			"packssdw	%%xmm6, %%xmm0\n"
			"movdqa		%%xmm3, %%xmm1\n"
			"movdqa		%%xmm5, %%xmm6\n"
			"psadbw		16(%[data]), %%xmm1\n"
			"psadbw		16(%[data]), %%xmm6\n"
			"packssdw	%%xmm6, %%xmm1\n"
			"movdqa		%%xmm3, %%xmm2\n"
			"movdqa		%%xmm5, %%xmm6\n"
			"psadbw		32(%[data]), %%xmm2\n"
			"psadbw		32(%[data]), %%xmm6\n"
			"packssdw	%%xmm6, %%xmm2\n"

			"shufps		%[shuf0202], %%xmm5, %%xmm3\n"
			"psrld		$24, %%xmm3\n"
			"packssdw	%%xmm3, %%xmm3\n"

			"movq		16(%[colorBlock],%%eax), %%xmm4\n"
			"pshufd		%[shuf0213], %%xmm4, %%xmm4\n"
			"movq		24(%[colorBlock],%%eax), %%xmm5\n"
			"pshufd		%[shuf0213], %%xmm5, %%xmm5\n"

			"movdqa		%%xmm4, %%xmm6\n"
			"movdqa		%%xmm5, %%xmm7\n"
			"psadbw		(%[data]), %%xmm6\n"
			"psadbw		(%[data]), %%xmm7\n"
			"packssdw	%%xmm7, %%xmm6\n"
			"packssdw	%%xmm6, %%xmm0\n"					// d1
			"movdqa		%%xmm4, %%xmm6\n"
			"movdqa		%%xmm5, %%xmm7\n"
			"psadbw		16(%[data]), %%xmm6\n"
			"psadbw		16(%[data]), %%xmm7\n"
			"packssdw	%%xmm7, %%xmm6\n"
			"packssdw	%%xmm6, %%xmm1\n"					// d1
			"movdqa		%%xmm4, %%xmm6\n"
			"movdqa		%%xmm5, %%xmm7\n"
			"psadbw		32(%[data]), %%xmm6\n"
			"psadbw		32(%[data]), %%xmm7\n"
			"packssdw	%%xmm7, %%xmm6\n"
			"packssdw	%%xmm6, %%xmm2\n"					// d2

			"shufps		%[shuf0202], %%xmm5, %%xmm4\n"
			"psrld		$24, %%xmm4\n"
			"packssdw	%%xmm4, %%xmm4\n"

			"punpcklqdq	%%xmm4, %%xmm3\n"					// c3

			"movdqa		64(%[data]), %%xmm7\n"
			"pslld		$16, %%xmm7\n"

			"movdqa		%%xmm2, %%xmm4\n"
			"pcmpgtw	%%xmm0, %%xmm2\n"					// b0
			"pcmpgtw	%%xmm1, %%xmm4\n"					// b1
			"pcmpgtw	%%xmm0, %%xmm1\n"					// b2
			"pmaxsw		_ZL18SIMD_SSE2_word_127, %%xmm3\n"	// b3
			"pcmpeqw	_ZL18SIMD_SSE2_word_127, %%xmm3\n"

			"pand		%%xmm4, %%xmm2\n"
			"por		%%xmm3, %%xmm2\n"					// b0 & b1 | b3
			"pxor		%%xmm4, %%xmm1\n"
			"por		%%xmm3, %%xmm1\n"					// b2 ^ b1 | b3
			"pand		_ZL16SIMD_SSE2_word_2, %%xmm2\n"
			"pand		_ZL16SIMD_SSE2_word_1, %%xmm1\n"
			"por		%%xmm1, %%xmm2\n"

			"pshufd		%[shuf2301], %%xmm2, %%xmm5\n"
			"punpcklwd	_ZL16SIMD_SSE2_word_0, %%xmm2\n"
			"punpcklwd	_ZL16SIMD_SSE2_word_0, %%xmm5\n"
			"pslld		$8, %%xmm5\n"
			"por		%%xmm5, %%xmm7\n"
			"por		%%xmm2, %%xmm7\n"
			"movdqa		%%xmm7, 64(%[data])\n"

			"sub		$32, %%eax\n"
			"jge		loop1_EmitColorAlphaIndices_SSE2\n"

			"pshufd		%[shuf1230], %%xmm7, %%xmm4\n"
			"pshufd		%[shuf2301], %%xmm7, %%xmm5\n"
			"pshufd		%[shuf3012], %%xmm7, %%xmm6\n"
			"pslld		$2, %%xmm4\n"
			"pslld		$4, %%xmm5\n"
			"pslld		$6, %%xmm6\n"
			"por		%%xmm4, %%xmm7\n"
			"por		%%xmm5, %%xmm7\n"
			"por		%%xmm6, %%xmm7\n"
			"movd		%%xmm7, (%[outPtr])\n"
			:: [colorBlock] "r" (colorBlock), [minColor] "r" (minColor_), [maxColor] "r" (maxColor_), [outPtr] "r" (outPtr), [data] "r" (data),
			[shuf0101] "i" R_SHUFFLE_D( 0, 1, 0, 1 ), [shuf0213] "i" R_SHUFFLE_D( 0, 2, 1, 3 ), [shuf0323] "i" R_SHUFFLE_D( 0, 3, 2, 3 ), [shuf3133] "i" R_SHUFFLE_D( 3, 1, 3, 3 ),
			[shuf0202] "i" R_SHUFFLE_D( 0, 2, 0, 2 ), [shuf2301] "i" R_SHUFFLE_D( 2, 3, 0, 1 ), [shuf1230] "i" R_SHUFFLE_D( 1, 2, 3, 0 ), [shuf3012] "i" R_SHUFFLE_D( 3, 0, 1, 2 )
			: "eax", "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "memory", "cc");
#endif

	outData += 4;
#elif defined ( ID_WIN_X86_SSE2_INTRIN ) || defined ( ID_QNX_X86_SSE2_INTRIN )
	__m128c zero = SIMD_SSE2_zero;
	__m128c result = SIMD_SSE2_zero;
	__m128c color0, color1, color2;
	__m128c temp0, temp1, temp2, temp3, temp4, temp5, temp6, temp7;
	__m128c minColor = _mm_cvtsi32_si128( *(int *)minColor_ );
	__m128c maxColor = _mm_cvtsi32_si128( *(int *)maxColor_ );
	__m128c blocka[2], blockb[2];
	blocka[0] = *((__m128i *)(&colorBlock[ 0]));
	blocka[1] = *((__m128i *)(&colorBlock[32]));
	blockb[0] = *((__m128i *)(&colorBlock[16]));
	blockb[1] = *((__m128i *)(&colorBlock[48]));

	temp0 = _mm_and_si128( maxColor, *(__m128c*)SIMD_SSE2_byte_colorMask );
	temp0 = _mm_unpacklo_epi8( temp0, zero );
	temp4 = _mm_shufflelo_epi16( temp0, R_SHUFFLE_D( 0, 3, 2, 3 ) );
	temp5 = _mm_shufflelo_epi16( temp0, R_SHUFFLE_D( 3, 1, 3, 3 ) );
	temp4 = _mm_srli_epi16( temp4, 5 );
	temp5 = _mm_srli_epi16( temp5, 6 );
	temp0 = _mm_or_si128( temp0, temp4 );
	temp0 = _mm_or_si128( temp0, temp5 );

	temp1 = _mm_and_si128( minColor, *(__m128c*)SIMD_SSE2_byte_colorMask );
	temp1 = _mm_unpacklo_epi8( temp1, zero );
	temp4 = _mm_shufflelo_epi16( temp1, R_SHUFFLE_D( 0, 3, 2, 3 ) );
	temp5 = _mm_shufflelo_epi16( temp1, R_SHUFFLE_D( 3, 1, 3, 3 ) );
	temp4 = _mm_srli_epi16( temp4, 5 );
	temp5 = _mm_srli_epi16( temp5, 6 );
	temp1 = _mm_or_si128( temp1, temp4 );
	temp1 = _mm_or_si128( temp1, temp5 );

	temp2 = _mm_packus_epi16( temp0, zero );
	color0 = _mm_shuffle_epi32( temp2, R_SHUFFLE_D( 0, 1, 0, 1 ) );

	temp6 = _mm_add_epi16( temp0, temp0 );
	temp6 = _mm_srli_epi16( temp6, 1 );			// diff from color
	temp6 = _mm_packus_epi16( temp6, zero );
	color2 = _mm_shuffle_epi32( temp6, R_SHUFFLE_D( 0, 1, 0, 1 ) );

	temp3 = _mm_packus_epi16( temp1, zero );
	color1 = _mm_shuffle_epi32( temp3, R_SHUFFLE_D( 0, 1, 0, 1 ) );

	// not used
	//color3 = zero;

	for ( int i = 1; i >= 0; i-- ) {
		// Load block
		temp3 = _mm_shuffle_epi32( blocka[i], R_SHUFFLE_D( 0, 2, 1, 3 ) );
		temp5 = _mm_shuffle_ps( blocka[i], zero, R_SHUFFLE_D( 2, 3, 0, 1 ) );
		temp5 = _mm_shuffle_epi32( temp5, R_SHUFFLE_D( 0, 2, 1, 3 ) );

		temp0 = _mm_sad_epu8( temp3, color0 );
		temp6 = _mm_sad_epu8( temp5, color0 );
		temp0 = _mm_packs_epi32( temp0, temp6 );

		temp1 = _mm_sad_epu8( temp3, color1 );
		temp6 = _mm_sad_epu8( temp5, color1 );
		temp1 = _mm_packs_epi32( temp1, temp6 );

		temp2 = _mm_sad_epu8( temp3, color2 );
		temp6 = _mm_sad_epu8( temp5, color2 );
		temp2 = _mm_packs_epi32( temp2, temp6 );


		// diff from color
		temp3 = _mm_shuffle_ps( temp3, temp5, R_SHUFFLE_D( 0, 2, 0, 2 ) );
		temp3 = _mm_srli_epi32( temp3, 24 );
		temp3 = _mm_packs_epi32( temp3, temp3 );


		// Load block
		temp4 = _mm_shuffle_epi32( blockb[i], R_SHUFFLE_D( 0, 2, 1, 3 ) );
		temp5 = _mm_shuffle_ps( blockb[i], zero, R_SHUFFLE_D( 2, 3, 0, 1 ) );
		temp5 = _mm_shuffle_epi32( temp5, R_SHUFFLE_D( 0, 2, 1, 3 ) );

		temp6 = _mm_sad_epu8( temp4, color0 );
		temp7 = _mm_sad_epu8( temp5, color0 );
		temp6 = _mm_packs_epi32( temp6, temp7 );
		temp0 = _mm_packs_epi32( temp0, temp6 );	// d0

		temp6 = _mm_sad_epu8( temp4, color1 );
		temp7 = _mm_sad_epu8( temp5, color1 );
		temp6 = _mm_packs_epi32( temp6, temp7 );
		temp1 = _mm_packs_epi32( temp1, temp6 );	// d1

		temp6 = _mm_sad_epu8( temp4, color2 );
		temp7 = _mm_sad_epu8( temp5, color2 );
		temp6 = _mm_packs_epi32( temp6, temp7 );
		temp2 = _mm_packs_epi32( temp2, temp6 );	// d2


		// diff from color
		temp4 = _mm_shuffle_ps( temp4, temp5, R_SHUFFLE_D( 0, 2, 0, 2 ) );		// c3
		temp4 = _mm_srli_epi32( temp4, 24 );
		temp4 = _mm_packs_epi32( temp4, temp4 );
		temp3 = _mm_unpacklo_epi64( temp3, temp4 );

		temp7 = _mm_slli_epi32( result, 16 );


		// diff from color
		temp4 = _mm_cmpgt_epi16( temp2, temp1 );						// b1
		temp2 = _mm_cmpgt_epi16( temp2, temp0 );						// b0
		temp1 = _mm_cmpgt_epi16( temp1, temp0 );						// b2
		temp3 = _mm_max_epi16( temp3, (const __m128i &)SIMD_SSE2_word_127 );	// b3
		temp3 = _mm_cmpeq_epi16( temp3, (const __m128i &)SIMD_SSE2_word_127 );

		temp2 = _mm_and_si128( temp2, temp4 );
		temp2 = _mm_or_si128( temp2, temp3 );							// b0 & b1 | b3
		temp1 = _mm_xor_si128( temp1, temp4 );
		temp1 = _mm_or_si128( temp1, temp3 );							// b2 ^ b1 | b3
		temp2 = _mm_and_si128( temp2, (const __m128i &)SIMD_SSE2_word_2 );
		temp1 = _mm_and_si128( temp1, (const __m128i &)SIMD_SSE2_word_1 );
		temp2 = _mm_or_si128( temp2, temp1 );



		temp5 = _mm_shuffle_epi32( temp2, R_SHUFFLE_D( 2, 3, 0, 1 ) );
		temp2 = _mm_unpacklo_epi16( temp2, (const __m128i &)SIMD_SSE2_word_0 );
		temp5 = _mm_unpacklo_epi16( temp5, (const __m128i &)SIMD_SSE2_word_0 );
		temp5 = _mm_slli_epi32( temp5, 8 );
		temp7 = _mm_or_si128( temp7, temp5 );
		result = _mm_or_si128( temp7, temp2 );
	}

	temp4 = _mm_shuffle_epi32( result, R_SHUFFLE_D( 1, 2, 3, 0 ) );
	temp5 = _mm_shuffle_epi32( result, R_SHUFFLE_D( 2, 3, 0, 1 ) );
	temp6 = _mm_shuffle_epi32( result, R_SHUFFLE_D( 3, 0, 1, 2 ) );
	temp4 = _mm_slli_epi32( temp4, 2 );
	temp5 = _mm_slli_epi32( temp5, 4 );
	temp6 = _mm_slli_epi32( temp6, 6 );
	temp7 = _mm_or_si128( result, temp4 );
	temp7 = _mm_or_si128( temp7, temp5 );
	temp7 = _mm_or_si128( temp7, temp6 );

	unsigned int out = _mm_cvtsi128_si32( temp7 );
	EmitUInt( out );
#else
	assert( false );
#endif
}

/*
========================
idDxtEncoder::EmitCoCgIndices_SSE2

params:	colorBlock	- 16 pixel block for which to find color indices
paramO:	minColor	- Min alpha found
paramO:	maxColor	- Max alpha found
return: 4 byte color index block
========================
*/
void idDxtEncoder::EmitCoCgIndices_SSE2( const byte *colorBlock, const byte *minColor_, const byte *maxColor_ ) {
#if ( defined( ID_WIN_X86_ASM ) || defined( ID_MAC_X86_ASM ) ) || defined ( ID_QNX_X86_SSE2_ASM )
	byte *outPtr = outData;

#if ( defined( ID_WIN_X86_ASM ) || defined( ID_MAC_X86_ASM ) )
	ALIGN16( byte color0[16] );
	ALIGN16( byte color1[16] );
	ALIGN16( byte color2[16] );
	ALIGN16( byte color3[16] );
	ALIGN16( byte result[16] );

	__asm {
		mov			esi, maxColor_
		mov			edi, minColor_
		pxor		xmm7, xmm7
		movdqa		result, xmm7

		movd		xmm0, dword ptr [esi]
		pand		xmm0, SIMD_SSE2_byte_colorMask2
		pshufd		xmm0, xmm0, R_SHUFFLE_D( 0, 1, 0, 1 )
		movdqa		color0, xmm0

		movd		xmm1, dword ptr [edi]
		pand		xmm1, SIMD_SSE2_byte_colorMask2
		pshufd		xmm1, xmm1, R_SHUFFLE_D( 0, 1, 0, 1 )
		movdqa		color1, xmm1

		punpcklbw	xmm0, xmm7
		punpcklbw	xmm1, xmm7

		movdqa		xmm6, xmm1
		paddw		xmm1, xmm0
		paddw		xmm0, xmm1
		pmulhw		xmm0, SIMD_SSE2_word_div_by_3	// * ( ( 1 << 16 ) / 3 + 1 ) ) >> 16
		packuswb	xmm0, xmm7
		pshufd		xmm0, xmm0, R_SHUFFLE_D( 0, 1, 0, 1 )
		movdqa		color2, xmm0

		paddw		xmm1, xmm6
		pmulhw		xmm1, SIMD_SSE2_word_div_by_3	// * ( ( 1 << 16 ) / 3 + 1 ) ) >> 16
		packuswb	xmm1, xmm7
		pshufd		xmm1, xmm1, R_SHUFFLE_D( 0, 1, 0, 1 )
		movdqa		color3, xmm1

		mov			eax, 32
		mov			esi, colorBlock

	loop1:			// iterates 2 times
		movq		xmm3, qword ptr [esi+eax+0]
		pshufd		xmm3, xmm3, R_SHUFFLE_D( 0, 2, 1, 3 )		// punpckldq	xmm4, SIMD_SSE2_dword_0
		movq		xmm5, qword ptr [esi+eax+8]
		pshufd		xmm5, xmm5, R_SHUFFLE_D( 0, 2, 1, 3 )		// punpckldq	xmm5, SIMD_SSE2_dword_0

		movdqa		xmm0, xmm3
		movdqa		xmm6, xmm5
		psadbw		xmm0, color0
		psadbw		xmm6, color0
		packssdw	xmm0, xmm6
		movdqa		xmm1, xmm3
		movdqa		xmm6, xmm5
		psadbw		xmm1, color1
		psadbw		xmm6, color1
		packssdw	xmm1, xmm6
		movdqa		xmm2, xmm3
		movdqa		xmm6, xmm5
		psadbw		xmm2, color2
		psadbw		xmm6, color2
		packssdw	xmm2, xmm6
		psadbw		xmm3, color3
		psadbw		xmm5, color3
		packssdw	xmm3, xmm5

		movq		xmm4, qword ptr [esi+eax+16]
		pshufd		xmm4, xmm4, R_SHUFFLE_D( 0, 2, 1, 3 )
		movq		xmm5, qword ptr [esi+eax+24]
		pshufd		xmm5, xmm5, R_SHUFFLE_D( 0, 2, 1, 3 )

		movdqa		xmm6, xmm4
		movdqa		xmm7, xmm5
		psadbw		xmm6, color0
		psadbw		xmm7, color0
		packssdw	xmm6, xmm7
		packssdw	xmm0, xmm6				// d1
		movdqa		xmm6, xmm4
		movdqa		xmm7, xmm5
		psadbw		xmm6, color1
		psadbw		xmm7, color1
		packssdw	xmm6, xmm7
		packssdw	xmm1, xmm6				// d1
		movdqa		xmm6, xmm4
		movdqa		xmm7, xmm5
		psadbw		xmm6, color2
		psadbw		xmm7, color2
		packssdw	xmm6, xmm7
		packssdw	xmm2, xmm6				// d2
		psadbw		xmm4, color3
		psadbw		xmm5, color3
		packssdw	xmm4, xmm5
		packssdw	xmm3, xmm4				// d3

		movdqa		xmm7, result
		pslld		xmm7, 16

		movdqa		xmm4, xmm0
		movdqa		xmm5, xmm1
		pcmpgtw		xmm0, xmm3				// b0
		pcmpgtw		xmm1, xmm2				// b1
		pcmpgtw		xmm4, xmm2				// b2
		pcmpgtw		xmm5, xmm3				// b3
		pcmpgtw		xmm2, xmm3				// b4
		pand		xmm4, xmm1				// x0
		pand		xmm5, xmm0				// x1
		pand		xmm2, xmm0				// x2
		por			xmm4, xmm5
		pand		xmm2, SIMD_SSE2_word_1
		pand		xmm4, SIMD_SSE2_word_2
		por			xmm2, xmm4

		pshufd		xmm5, xmm2, R_SHUFFLE_D( 2, 3, 0, 1 )
		punpcklwd	xmm2, SIMD_SSE2_word_0
		punpcklwd	xmm5, SIMD_SSE2_word_0
		pslld		xmm5, 8
		por			xmm7, xmm5
		por			xmm7, xmm2
		movdqa		result, xmm7

		sub			eax, 32
		jge			loop1

		mov			esi, outPtr
		pshufd		xmm4, xmm7, R_SHUFFLE_D( 1, 2, 3, 0 )
		pshufd		xmm5, xmm7, R_SHUFFLE_D( 2, 3, 0, 1 )
		pshufd		xmm6, xmm7, R_SHUFFLE_D( 3, 0, 1, 2 )
		pslld		xmm4, 2
		pslld		xmm5, 4
		pslld		xmm6, 6
		por			xmm7, xmm4
		por			xmm7, xmm5
		por			xmm7, xmm6
		movd		dword ptr [esi], xmm7
	}
#else
	ALIGN16( byte data[16 * 5] ); // 5 separate arrays end up using too many registers for inline ASM, so consolidate them.

	__asm__ __volatile__(
			"pxor		%%xmm7, %%xmm7\n"
			"movdqa		%%xmm7, 64(%[data])\n"

			"movd		(%[maxColor]), %%xmm0\n"
			"pand		_ZL25SIMD_SSE2_byte_colorMask2, %%xmm0\n"
			"pshufd		%[shuf0101], %%xmm0, %%xmm0\n"
			"movdqa		%%xmm0, (%[data])\n"

			"movd		(%[minColor]), %%xmm1\n"
			"pand		_ZL25SIMD_SSE2_byte_colorMask2, %%xmm1\n"
			"pshufd		%[shuf0101], %%xmm1, %%xmm1\n"
			"movdqa		%%xmm1, 16(%[data])\n"

			"punpcklbw	%%xmm7, %%xmm0\n"
			"punpcklbw	%%xmm7, %%xmm1\n"

			"movdqa		%%xmm1, %%xmm6\n"
			"paddw		%%xmm0, %%xmm1\n"
			"paddw		%%xmm1, %%xmm0\n"
			"pmulhw		_ZL23SIMD_SSE2_word_div_by_3, %%xmm0\n"	// * ( ( 1 << 16 ) / 3 + 1 ) ) >> 16
			"packuswb	%%xmm7, %%xmm0\n"
			"pshufd		%[shuf0101], %%xmm0, %%xmm0\n"
			"movdqa		%%xmm0, 32(%[data])\n"

			"paddw		%%xmm6, %%xmm1\n"
			"pmulhw		_ZL23SIMD_SSE2_word_div_by_3, %%xmm1\n"	// * ( ( 1 << 16 ) / 3 + 1 ) ) >> 16
			"packuswb	%%xmm7, %%xmm1\n"
			"pshufd		%[shuf0101], %%xmm1, %%xmm1\n"
			"movdqa		%%xmm1, 48(%[data])\n"

			"mov		$32, %%eax\n"

		"loop1_EmitCoCgIndices_SSE2:\n"		// iterates 2 times
			"movq		0(%[colorBlock],%%eax), %%xmm3\n"
			"pshufd		%[shuf0213], %%xmm3, %%xmm3\n"			// punpckldq	xmm4, SIMD_SSE2_dword_0
			"movq		8(%[colorBlock],%%eax), %%xmm5\n"
			"pshufd		%[shuf0213], %%xmm5, %%xmm5\n"			// punpckldq	xmm5, SIMD_SSE2_dword_0

			"movdqa		%%xmm3, %%xmm0\n"
			"movdqa		%%xmm5, %%xmm6\n"
			"psadbw		(%[data]), %%xmm0\n"
			"psadbw		(%[data]), %%xmm6\n"
			"packssdw	%%xmm6, %%xmm0\n"
			"movdqa		%%xmm3, %%xmm1\n"
			"movdqa		%%xmm5, %%xmm6\n"
			"psadbw		16(%[data]), %%xmm1\n"
			"psadbw		16(%[data]), %%xmm6\n"
			"packssdw	%%xmm6, %%xmm1\n"
			"movdqa		%%xmm3, %%xmm2\n"
			"movdqa		%%xmm5, %%xmm6\n"
			"psadbw		32(%[data]), %%xmm2\n"
			"psadbw		32(%[data]), %%xmm6\n"
			"packssdw	%%xmm6, %%xmm2\n"
			"psadbw		48(%[data]), %%xmm3\n"
			"psadbw		48(%[data]), %%xmm5\n"
			"packssdw	%%xmm5, %%xmm3\n"

			"movq		16(%[colorBlock],%%eax), %%xmm4\n"
			"pshufd		%[shuf0213], %%xmm4, %%xmm4\n"
			"movq		24(%[colorBlock],%%eax), %%xmm5\n"
			"pshufd		%[shuf0213], %%xmm5, %%xmm5\n"

			"movdqa		%%xmm4, %%xmm6\n"
			"movdqa		%%xmm5, %%xmm7\n"
			"psadbw		(%[data]), %%xmm6\n"
			"psadbw		(%[data]), %%xmm7\n"
			"packssdw	%%xmm7, %%xmm6\n"
			"packssdw	%%xmm6, %%xmm0\n"						// d1
			"movdqa		%%xmm4, %%xmm6\n"
			"movdqa		%%xmm5, %%xmm7\n"
			"psadbw		16(%[data]), %%xmm6\n"
			"psadbw		16(%[data]), %%xmm7\n"
			"packssdw	%%xmm7, %%xmm6\n"
			"packssdw	%%xmm6, %%xmm1\n"						// d1
			"movdqa		%%xmm4, %%xmm6\n"
			"movdqa		%%xmm5, %%xmm7\n"
			"psadbw		32(%[data]), %%xmm6\n"
			"psadbw		32(%[data]), %%xmm7\n"
			"packssdw	%%xmm7, %%xmm6\n"
			"packssdw	%%xmm6, %%xmm2\n"						// d2
			"psadbw		48(%[data]), %%xmm4\n"
			"psadbw		48(%[data]), %%xmm5\n"
			"packssdw	%%xmm5, %%xmm4\n"
			"packssdw	%%xmm4, %%xmm3\n"						// d3

			"movdqa		64(%[data]), %%xmm7\n"
			"pslld		$16, %%xmm7\n"

			"movdqa		%%xmm0, %%xmm4\n"
			"movdqa		%%xmm1, %%xmm5\n"
			"pcmpgtw	%%xmm3, %%xmm0\n"						// b0
			"pcmpgtw	%%xmm2, %%xmm1\n"						// b1
			"pcmpgtw	%%xmm2, %%xmm4\n"						// b2
			"pcmpgtw	%%xmm3, %%xmm5\n"						// b3
			"pcmpgtw	%%xmm3, %%xmm2\n"						// b4
			"pand		%%xmm1, %%xmm4\n"						// x0
			"pand		%%xmm0, %%xmm5\n"						// x1
			"pand		%%xmm0, %%xmm2\n"						// x2
			"por		%%xmm5, %%xmm4\n"
			"pand		_ZL16SIMD_SSE2_word_1, %%xmm2\n"
			"pand		_ZL16SIMD_SSE2_word_2, %%xmm4\n"
			"por		%%xmm4, %%xmm2\n"

			"pshufd		%[shuf2301], %%xmm2, %%xmm5\n"
			"punpcklwd	_ZL16SIMD_SSE2_word_0, %%xmm2\n"
			"punpcklwd	_ZL16SIMD_SSE2_word_0, %%xmm5\n"
			"pslld		$8, %%xmm5\n"
			"por		%%xmm5, %%xmm7\n"
			"por		%%xmm2, %%xmm7\n"
			"movdqa		%%xmm7, 64(%[data])\n"

			"sub		$32, %%eax\n"
			"jge		loop1_EmitCoCgIndices_SSE2\n"

			"pshufd		%[shuf1230], %%xmm7, %%xmm4\n"
			"pshufd		%[shuf2301], %%xmm7, %%xmm5\n"
			"pshufd		%[shuf3012], %%xmm7, %%xmm6\n"
			"pslld		$2, %%xmm4\n"
			"pslld		$4, %%xmm5\n"
			"pslld		$6, %%xmm6\n"
			"por		%%xmm4, %%xmm7\n"
			"por		%%xmm5, %%xmm7\n"
			"por		%%xmm6, %%xmm7\n"
			"movd		%%xmm7, (%[outPtr])\n"
			:: [colorBlock] "r" (colorBlock), [minColor] "r" (minColor_), [maxColor] "r" (maxColor_), [outPtr] "r" (outPtr), [data] "r" (data),
			[shuf0101] "i" R_SHUFFLE_D( 0, 1, 0, 1 ), [shuf0213] "i" R_SHUFFLE_D( 0, 2, 1, 3 ), [shuf2301] "i" R_SHUFFLE_D( 2, 3, 0, 1 ), [shuf1230] "i" R_SHUFFLE_D( 1, 2, 3, 0 ),
			[shuf3012] "i" R_SHUFFLE_D( 3, 0, 1, 2 )
			: "eax", "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "memory", "cc");
#endif

	outData += 4;
#elif defined ( ID_WIN_X86_SSE2_INTRIN ) || defined ( ID_QNX_X86_SSE2_INTRIN )
	__m128c zero = SIMD_SSE2_zero;
	__m128c result = SIMD_SSE2_zero;
	__m128c color0, color1, color2, color3;
	__m128c temp0, temp1, temp2, temp3, temp4, temp5, temp6, temp7;
	__m128c minColor = _mm_cvtsi32_si128( *(int *)minColor_ );
	__m128c maxColor = _mm_cvtsi32_si128( *(int *)maxColor_ );
	__m128c blocka[2], blockb[2];
	blocka[0] = *((__m128i *)(&colorBlock[ 0]));
	blocka[1] = *((__m128i *)(&colorBlock[32]));
	blockb[0] = *((__m128i *)(&colorBlock[16]));
	blockb[1] = *((__m128i *)(&colorBlock[48]));

	temp7 = zero;

	temp0 = maxColor;
	temp0 = _mm_and_si128( temp0, *(__m128c*)SIMD_SSE2_byte_colorMask2 );
	color0 = _mm_shuffle_epi32( temp0, R_SHUFFLE_D( 0, 1, 0, 1 ) );

	temp1 = minColor;
	temp1 = _mm_and_si128( temp1, *(__m128c*)SIMD_SSE2_byte_colorMask2 );
	color1 = _mm_shuffle_epi32( temp1, R_SHUFFLE_D( 0, 1, 0, 1 ) );

	temp0 = _mm_unpacklo_epi8( color0, zero );
	temp1 = _mm_unpacklo_epi8( color1, zero );

	temp6 = _mm_add_epi16( temp1, temp0 );
	temp0 = _mm_add_epi16( temp0, temp6 );
	temp0 = _mm_mulhi_epi16( temp0, (const __m128i &)SIMD_SSE2_word_div_by_3 );		// * ( ( 1 << 16 ) / 3 + 1 ) ) >> 16
	temp0 = _mm_packus_epi16( temp0, zero );
	color2 = _mm_shuffle_epi32( temp0, R_SHUFFLE_D( 0, 1, 0, 1 ) );

	temp1 = _mm_add_epi16( temp1, temp6 );
	temp1 = _mm_mulhi_epi16( temp1, (const __m128i &)SIMD_SSE2_word_div_by_3 );		// * ( ( 1 << 16 ) / 3 + 1 ) ) >> 16
	temp1 = _mm_packus_epi16( temp1, zero );
	color3 = _mm_shuffle_epi32( temp1, R_SHUFFLE_D( 0, 1, 0, 1 ) );

	for ( int i = 1; i >= 0; i-- ) {
		// Load block
		temp3 = _mm_shuffle_epi32( blocka[i], R_SHUFFLE_D( 0, 2, 1, 3 ) );
		temp5 = _mm_shuffle_ps( blocka[i], zero, R_SHUFFLE_D( 2, 3, 0, 1 ) );
		temp5 = _mm_shuffle_epi32( temp5, R_SHUFFLE_D( 0, 2, 1, 3 ) );

		temp0 = _mm_sad_epu8( temp3, color0 );
		temp6 = _mm_sad_epu8( temp5, color0 );
		temp0 = _mm_packs_epi32( temp0, temp6 );

		temp1 = _mm_sad_epu8( temp3, color1 );
		temp6 = _mm_sad_epu8( temp5, color1 );
		temp1 = _mm_packs_epi32( temp1, temp6 );

		temp2 = _mm_sad_epu8( temp3, color2 );
		temp6 = _mm_sad_epu8( temp5, color2 );
		temp2 = _mm_packs_epi32( temp2, temp6 );

		temp3 = _mm_sad_epu8( temp3, color3 );
		temp5 = _mm_sad_epu8( temp5, color3 );
		temp3 = _mm_packs_epi32( temp3, temp5 );


		// Load block
		temp4 = _mm_shuffle_epi32( blockb[i], R_SHUFFLE_D( 0, 2, 1, 3 ) );
		temp5 = _mm_shuffle_ps( blockb[i], zero, R_SHUFFLE_D( 2, 3, 0, 1 ) );
		temp5 = _mm_shuffle_epi32( temp5, R_SHUFFLE_D( 0, 2, 1, 3 ) );

		temp6 = _mm_sad_epu8( temp4, color0 );
		temp7 = _mm_sad_epu8( temp5, color0 );
		temp6 = _mm_packs_epi32( temp6, temp7 );
		temp0 = _mm_packs_epi32( temp0, temp6 );	// d0

		temp6 = _mm_sad_epu8( temp4, color1 );
		temp7 = _mm_sad_epu8( temp5, color1 );
		temp6 = _mm_packs_epi32( temp6, temp7 );
		temp1 = _mm_packs_epi32( temp1, temp6 );	// d1

		temp6 = _mm_sad_epu8( temp4, color2 );
		temp7 = _mm_sad_epu8( temp5, color2 );
		temp6 = _mm_packs_epi32( temp6, temp7 );
		temp2 = _mm_packs_epi32( temp2, temp6 );	// d2

		temp4 = _mm_sad_epu8( temp4, color3 );
		temp5 = _mm_sad_epu8( temp5, color3 );
		temp4 = _mm_packs_epi32( temp4, temp5 );
		temp3 = _mm_packs_epi32( temp3, temp4 );	// d3

		temp7 = _mm_slli_epi32( result, 16 );

		temp4 = _mm_cmpgt_epi16( temp0, temp2 );	// b2
		temp5 = _mm_cmpgt_epi16( temp1, temp3 );	// b3
		temp0 = _mm_cmpgt_epi16( temp0, temp3 );	// b0
		temp1 = _mm_cmpgt_epi16( temp1, temp2 );	// b1
		temp2 = _mm_cmpgt_epi16( temp2, temp3 );	// b4
		temp4 = _mm_and_si128( temp4, temp1 );		// x0
		temp5 = _mm_and_si128( temp5, temp0 );		// x1
		temp2 = _mm_and_si128( temp2, temp0 );		// x2
		temp4 = _mm_or_si128( temp4, temp5 );
		temp2 = _mm_and_si128( temp2, (const __m128i &)SIMD_SSE2_word_1 );
		temp4 = _mm_and_si128( temp4, (const __m128i &)SIMD_SSE2_word_2 );
		temp2 = _mm_or_si128( temp2, temp4 );

		temp5 = _mm_shuffle_epi32( temp2, R_SHUFFLE_D( 2, 3, 0, 1 ) );
		temp2 = _mm_unpacklo_epi16( temp2, (const __m128i &)SIMD_SSE2_word_0 );
		temp5 = _mm_unpacklo_epi16( temp5, (const __m128i &)SIMD_SSE2_word_0 );
		temp5 = _mm_slli_epi32( temp5, 8 );
		temp7 = _mm_or_si128( temp7, temp5 );
		result = _mm_or_si128( temp7, temp2 );
	}

	temp4 = _mm_shuffle_epi32( result, R_SHUFFLE_D( 1, 2, 3, 0 ) );
	temp5 = _mm_shuffle_epi32( result, R_SHUFFLE_D( 2, 3, 0, 1 ) );
	temp6 = _mm_shuffle_epi32( result, R_SHUFFLE_D( 3, 0, 1, 2 ) );
	temp4 = _mm_slli_epi32( temp4, 2 );
	temp5 = _mm_slli_epi32( temp5, 4 );
	temp6 = _mm_slli_epi32( temp6, 6 );
	temp7 = _mm_or_si128( result, temp4 );
	temp7 = _mm_or_si128( temp7, temp5 );
	temp7 = _mm_or_si128( temp7, temp6 );

	unsigned int out = _mm_cvtsi128_si32( temp7 );
	EmitUInt( out );
#else
	assert( false );
#endif
}

/*
========================
idDxtEncoder::EmitAlphaIndices_SSE2

params:	block		- 16 pixel block for which to find alpha indices
paramO:	minAlpha	- Min alpha found
paramO:	maxAlpha	- Max alpha found
========================
*/
void idDxtEncoder::EmitAlphaIndices_SSE2( const byte *block, const int minAlpha_, const int maxAlpha_ ) {
#if ( defined( ID_WIN_X86_ASM ) || defined( ID_MAC_X86_ASM ) ) || defined ( ID_QNX_X86_SSE2_ASM )
	assert( maxAlpha_ >= minAlpha_ );

	byte *outPtr = outData;

#if ( defined( ID_WIN_X86_ASM ) || defined( ID_MAC_X86_ASM ) )
	__asm {
		mov			esi, block
		movdqa		xmm0, xmmword ptr [esi+	0]
		movdqa		xmm5, xmmword ptr [esi+16]
		movdqa		xmm6, xmmword ptr [esi+32]
		movdqa		xmm4, xmmword ptr [esi+48]

		psrld		xmm0, 24
		psrld		xmm5, 24
		psrld		xmm6, 24
		psrld		xmm4, 24

		packuswb	xmm0, xmm5
		packuswb	xmm6, xmm4

		//---------------------

		// ab0 = (  7 * maxAlpha +  7 * minAlpha + ALPHA_RANGE ) / 14
		// ab3 = (  9 * maxAlpha +  5 * minAlpha + ALPHA_RANGE ) / 14
		// ab2 = ( 11 * maxAlpha +  3 * minAlpha + ALPHA_RANGE ) / 14
		// ab1 = ( 13 * maxAlpha +  1 * minAlpha + ALPHA_RANGE ) / 14

		// ab4 = (  7 * maxAlpha +  7 * minAlpha + ALPHA_RANGE ) / 14
		// ab5 = (  5 * maxAlpha +  9 * minAlpha + ALPHA_RANGE ) / 14
		// ab6 = (  3 * maxAlpha + 11 * minAlpha + ALPHA_RANGE ) / 14
		// ab7 = (  1 * maxAlpha + 13 * minAlpha + ALPHA_RANGE ) / 14

		movd		xmm5, maxAlpha_
		pshuflw		xmm5, xmm5,	R_SHUFFLE_D( 0,	0, 0, 0	)
		pshufd		xmm5, xmm5,	R_SHUFFLE_D( 0,	0, 0, 0	)
		movdqa		xmm7, xmm5

		movd		xmm2, minAlpha_
		pshuflw		xmm2, xmm2,	R_SHUFFLE_D( 0,	0, 0, 0	)
		pshufd		xmm2, xmm2,	R_SHUFFLE_D( 0,	0, 0, 0	)
		movdqa		xmm3, xmm2

		pmullw		xmm5, SIMD_SSE2_word_scale_7_9_11_13
		pmullw		xmm7, SIMD_SSE2_word_scale_7_5_3_1
		pmullw		xmm2, SIMD_SSE2_word_scale_7_5_3_1
		pmullw		xmm3, SIMD_SSE2_word_scale_7_9_11_13

		paddw		xmm5, xmm2
		paddw		xmm7, xmm3

		paddw		xmm5, SIMD_SSE2_word_7
		paddw		xmm7, SIMD_SSE2_word_7

		pmulhw		xmm5, SIMD_SSE2_word_div_by_14			// * ( ( 1 << 16 ) / 14	+ 1	) )	>> 16
		pmulhw		xmm7, SIMD_SSE2_word_div_by_14			// * ( ( 1 << 16 ) / 14	+ 1	) )	>> 16

		pshufd		xmm1, xmm5,	R_SHUFFLE_D( 3, 3, 3, 3	)
		pshufd		xmm2, xmm5,	R_SHUFFLE_D( 2, 2, 2, 2	)
		pshufd		xmm3, xmm5,	R_SHUFFLE_D( 1, 1, 1, 1	)
		packuswb	xmm1, xmm1								// ab1
		packuswb	xmm2, xmm2								// ab2
		packuswb	xmm3, xmm3								// ab3

		packuswb	xmm0, xmm6								// alpha block

		pshufd		xmm4, xmm7,	R_SHUFFLE_D( 0,	0, 0, 0	)
		pshufd		xmm5, xmm7,	R_SHUFFLE_D( 1,	1, 1, 1	)
		pshufd		xmm6, xmm7,	R_SHUFFLE_D( 2,	2, 2, 2	)
		pshufd		xmm7, xmm7,	R_SHUFFLE_D( 3,	3, 3, 3	)
		packuswb	xmm4, xmm4								// ab4
		packuswb	xmm5, xmm5								// ab5
		packuswb	xmm6, xmm6								// ab6
		packuswb	xmm7, xmm7								// ab7

		pmaxub		xmm1, xmm0
		pmaxub		xmm2, xmm0
		pmaxub		xmm3, xmm0
		pcmpeqb		xmm1, xmm0
		pcmpeqb		xmm2, xmm0
		pcmpeqb		xmm3, xmm0
		pmaxub		xmm4, xmm0
		pmaxub		xmm5, xmm0
		pmaxub		xmm6, xmm0
		pmaxub		xmm7, xmm0
		pcmpeqb		xmm4, xmm0
		pcmpeqb		xmm5, xmm0
		pcmpeqb		xmm6, xmm0
		pcmpeqb		xmm7, xmm0
		movdqa		xmm0, SIMD_SSE2_byte_8
		paddsb		xmm0, xmm1
		paddsb		xmm2, xmm3
		paddsb		xmm4, xmm5
		paddsb		xmm6, xmm7
		paddsb		xmm0, xmm2
		paddsb		xmm4, xmm6
		paddsb		xmm0, xmm4
		pand		xmm0, SIMD_SSE2_byte_7
		movdqa		xmm1, SIMD_SSE2_byte_2
		pcmpgtb		xmm1, xmm0
		pand		xmm1, SIMD_SSE2_byte_1
		pxor		xmm0, xmm1
		movdqa		xmm1, xmm0
		movdqa		xmm2, xmm0
		movdqa		xmm3, xmm0
		movdqa		xmm4, xmm0
		movdqa		xmm5, xmm0
		movdqa		xmm6, xmm0
		movdqa		xmm7, xmm0
		psrlq		xmm1,  8- 3
		psrlq		xmm2, 16- 6
		psrlq		xmm3, 24- 9
		psrlq		xmm4, 32-12
		psrlq		xmm5, 40-15
		psrlq		xmm6, 48-18
		psrlq		xmm7, 56-21
		pand		xmm0, SIMD_SSE2_dword_alpha_bit_mask0
		pand		xmm1, SIMD_SSE2_dword_alpha_bit_mask1
		pand		xmm2, SIMD_SSE2_dword_alpha_bit_mask2
		pand		xmm3, SIMD_SSE2_dword_alpha_bit_mask3
		pand		xmm4, SIMD_SSE2_dword_alpha_bit_mask4
		pand		xmm5, SIMD_SSE2_dword_alpha_bit_mask5
		pand		xmm6, SIMD_SSE2_dword_alpha_bit_mask6
		pand		xmm7, SIMD_SSE2_dword_alpha_bit_mask7
		por			xmm0, xmm1
		por			xmm2, xmm3
		por			xmm4, xmm5
		por			xmm6, xmm7
		por			xmm0, xmm2
		por			xmm4, xmm6
		por			xmm0, xmm4
		mov			esi, outPtr
		movd		[esi+0], xmm0
		pshufd		xmm1, xmm0, R_SHUFFLE_D( 2, 3, 0, 1 )
		movd		[esi+3], xmm1
	}
#else
	__asm__ __volatile__(
			"movdqa		 0(%[block]), %%xmm0\n"
			"movdqa		16(%[block]), %%xmm5\n"
			"movdqa		32(%[block]), %%xmm6\n"
			"movdqa		48(%[block]), %%xmm4\n"

			"psrld		$24, %%xmm0\n"
			"psrld		$24, %%xmm5\n"
			"psrld		$24, %%xmm6\n"
			"psrld		$24, %%xmm4\n"

			"packuswb	%%xmm5, %%xmm0\n"
			"packuswb	%%xmm4, %%xmm6\n"

			//---------------------

			// ab0 = (  7 * maxAlpha +  7 * minAlpha + ALPHA_RANGE ) / 14
			// ab3 = (  9 * maxAlpha +  5 * minAlpha + ALPHA_RANGE ) / 14
			// ab2 = ( 11 * maxAlpha +  3 * minAlpha + ALPHA_RANGE ) / 14
			// ab1 = ( 13 * maxAlpha +  1 * minAlpha + ALPHA_RANGE ) / 14

			// ab4 = (  7 * maxAlpha +  7 * minAlpha + ALPHA_RANGE ) / 14
			// ab5 = (  5 * maxAlpha +  9 * minAlpha + ALPHA_RANGE ) / 14
			// ab6 = (  3 * maxAlpha + 11 * minAlpha + ALPHA_RANGE ) / 14
			// ab7 = (  1 * maxAlpha + 13 * minAlpha + ALPHA_RANGE ) / 14

			"movd		%[maxAlpha], %%xmm5\n"
			"pshuflw	%[shuf0000], %%xmm5, %%xmm5\n"
			"pshufd		%[shuf0000], %%xmm5, %%xmm5\n"
			"movdqa		%%xmm5, %%xmm7\n"

			"movd		%[minAlpha], %%xmm2\n"
			"pshuflw	%[shuf0000], %%xmm2, %%xmm2\n"
			"pshufd		%[shuf0000], %%xmm2, %%xmm2\n"
			"movdqa		%%xmm2, %%xmm3\n"

			"pmullw		_ZL30SIMD_SSE2_word_scale_7_9_11_13, %%xmm5\n"
			"pmullw		_ZL28SIMD_SSE2_word_scale_7_5_3_1, %%xmm7\n"
			"pmullw		_ZL28SIMD_SSE2_word_scale_7_5_3_1, %%xmm2\n"
			"pmullw		_ZL30SIMD_SSE2_word_scale_7_9_11_13, %%xmm3\n"

			"paddw		%%xmm2, %%xmm5\n"
			"paddw		%%xmm3, %%xmm7\n"

			"paddw		_ZL16SIMD_SSE2_word_7, %%xmm5\n"
			"paddw		_ZL16SIMD_SSE2_word_7, %%xmm7\n"

			"pmulhw		_ZL24SIMD_SSE2_word_div_by_14, %%xmm5\n"		// * ( ( 1 << 16 ) / 14	+ 1	) )	>> 16
			"pmulhw		_ZL24SIMD_SSE2_word_div_by_14, %%xmm7\n"		// * ( ( 1 << 16 ) / 14	+ 1	) )	>> 16

			"pshufd		%[shuf3333], %%xmm5, %%xmm1\n"
			"pshufd		%[shuf2222], %%xmm5, %%xmm2\n"
			"pshufd		%[shuf1111], %%xmm5, %%xmm3\n"
			"packuswb	%%xmm1, %%xmm1\n"								// ab1
			"packuswb	%%xmm2, %%xmm2\n"								// ab2
			"packuswb	%%xmm3, %%xmm3\n"								// ab3

			"packuswb	%%xmm6, %%xmm0\n"								// alpha block

			"pshufd		%[shuf0000], %%xmm7, %%xmm4\n"
			"pshufd		%[shuf1111], %%xmm7, %%xmm5\n"
			"pshufd		%[shuf2222], %%xmm7, %%xmm6\n"
			"pshufd		%[shuf3333], %%xmm7, %%xmm7\n"
			"packuswb	%%xmm4, %%xmm4\n"								// ab4
			"packuswb	%%xmm5, %%xmm5\n"								// ab5
			"packuswb	%%xmm6, %%xmm6\n"								// ab6
			"packuswb	%%xmm7, %%xmm7\n"								// ab7

			"pmaxub		%%xmm0, %%xmm1\n"
			"pmaxub		%%xmm0, %%xmm2\n"
			"pmaxub		%%xmm0, %%xmm3\n"
			"pcmpeqb	%%xmm0, %%xmm1\n"
			"pcmpeqb	%%xmm0, %%xmm2\n"
			"pcmpeqb	%%xmm0, %%xmm3\n"
			"pmaxub		%%xmm0, %%xmm4\n"
			"pmaxub		%%xmm0, %%xmm5\n"
			"pmaxub		%%xmm0, %%xmm6\n"
			"pmaxub		%%xmm0, %%xmm7\n"
			"pcmpeqb	%%xmm0, %%xmm4\n"
			"pcmpeqb	%%xmm0, %%xmm5\n"
			"pcmpeqb	%%xmm0, %%xmm6\n"
			"pcmpeqb	%%xmm0, %%xmm7\n"
			"movdqa		_ZL16SIMD_SSE2_byte_8, %%xmm0\n"
			"paddsb		%%xmm1, %%xmm0\n"
			"paddsb		%%xmm3, %%xmm2\n"
			"paddsb		%%xmm5, %%xmm4\n"
			"paddsb		%%xmm7, %%xmm6\n"
			"paddsb		%%xmm2, %%xmm0\n"
			"paddsb		%%xmm6, %%xmm4\n"
			"paddsb		%%xmm4, %%xmm0\n"
			"pand		_ZL16SIMD_SSE2_byte_7, %%xmm0\n"
			"movdqa		_ZL16SIMD_SSE2_byte_2, %%xmm1\n"
			"pcmpgtb	%%xmm0, %%xmm1\n"
			"pand		_ZL16SIMD_SSE2_byte_1, %%xmm1\n"
			"pxor		%%xmm1, %%xmm0\n"
			"movdqa		%%xmm0, %%xmm1\n"
			"movdqa		%%xmm0, %%xmm2\n"
			"movdqa		%%xmm0, %%xmm3\n"
			"movdqa		%%xmm0, %%xmm4\n"
			"movdqa		%%xmm0, %%xmm5\n"
			"movdqa		%%xmm0, %%xmm6\n"
			"movdqa		%%xmm0, %%xmm7\n"
			"psrlq		$( 8- 3), %%xmm1\n"
			"psrlq		$(16- 6), %%xmm2\n"
			"psrlq		$(24- 9), %%xmm3\n"
			"psrlq		$(32-12), %%xmm4\n"
			"psrlq		$(40-15), %%xmm5\n"
			"psrlq		$(48-18), %%xmm6\n"
			"psrlq		$(56-21), %%xmm7\n"
			"pand		_ZL31SIMD_SSE2_dword_alpha_bit_mask0, %%xmm0\n"
			"pand		_ZL31SIMD_SSE2_dword_alpha_bit_mask1, %%xmm1\n"
			"pand		_ZL31SIMD_SSE2_dword_alpha_bit_mask2, %%xmm2\n"
			"pand		_ZL31SIMD_SSE2_dword_alpha_bit_mask3, %%xmm3\n"
			"pand		_ZL31SIMD_SSE2_dword_alpha_bit_mask4, %%xmm4\n"
			"pand		_ZL31SIMD_SSE2_dword_alpha_bit_mask5, %%xmm5\n"
			"pand		_ZL31SIMD_SSE2_dword_alpha_bit_mask6, %%xmm6\n"
			"pand		_ZL31SIMD_SSE2_dword_alpha_bit_mask7, %%xmm7\n"
			"por		%%xmm1, %%xmm0\n"
			"por		%%xmm3, %%xmm2\n"
			"por		%%xmm5, %%xmm4\n"
			"por		%%xmm7, %%xmm6\n"
			"por		%%xmm2, %%xmm0\n"
			"por		%%xmm6, %%xmm4\n"
			"por		%%xmm4, %%xmm0\n"
			"movd		%%xmm0, 0(%[outPtr])\n"
			"pshufd		%[shuf2301], %%xmm0, %%xmm1\n"
			"movd		%%xmm1, 3(%[outPtr])\n"
			:: [block] "r" (block), [minAlpha] "r" (minAlpha_), [maxAlpha] "r" (maxAlpha_), [outPtr] "r" (outPtr),
			[shuf0000] "i" R_SHUFFLE_D( 0, 0, 0, 0 ), [shuf1111] "i" R_SHUFFLE_D( 1, 1, 1, 1 ), [shuf2222] "i" R_SHUFFLE_D( 2, 2, 2, 2 ), [shuf3333] "i" R_SHUFFLE_D( 3, 3, 3, 3 ),
			[shuf2301] "i" R_SHUFFLE_D( 2, 3, 0, 1 )
			: "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "memory");
#endif

	outData += 6;
#elif defined ( ID_WIN_X86_SSE2_INTRIN ) || defined ( ID_QNX_X86_SSE2_INTRIN )
	__m128i block0 = *((__m128i *)(&block[ 0]));
	__m128i block1 = *((__m128i *)(&block[16]));
	__m128i block2 = *((__m128i *)(&block[32]));
	__m128i block3 = *((__m128i *)(&block[48]));
	__m128c temp0, temp1, temp2, temp3, temp4, temp5, temp6, temp7;

	temp0 = _mm_srli_epi32( block0, 24 );
	temp5 = _mm_srli_epi32( block1, 24 );
	temp6 = _mm_srli_epi32( block2, 24 );
	temp4 = _mm_srli_epi32( block3, 24 );

	temp0 = _mm_packus_epi16( temp0, temp5 );
	temp6 = _mm_packus_epi16( temp6, temp4 );

	//---------------------

	// ab0 = (  7 * maxAlpha +  7 * minAlpha + ALPHA_RANGE ) / 14
	// ab3 = (  9 * maxAlpha +  5 * minAlpha + ALPHA_RANGE ) / 14
	// ab2 = ( 11 * maxAlpha +  3 * minAlpha + ALPHA_RANGE ) / 14
	// ab1 = ( 13 * maxAlpha +  1 * minAlpha + ALPHA_RANGE ) / 14

	// ab4 = (  7 * maxAlpha +  7 * minAlpha + ALPHA_RANGE ) / 14
	// ab5 = (  5 * maxAlpha +  9 * minAlpha + ALPHA_RANGE ) / 14
	// ab6 = (  3 * maxAlpha + 11 * minAlpha + ALPHA_RANGE ) / 14
	// ab7 = (  1 * maxAlpha + 13 * minAlpha + ALPHA_RANGE ) / 14

	temp5 = _mm_cvtsi32_si128( maxAlpha_ );
	temp5 = _mm_shufflelo_epi16( temp5, R_SHUFFLE_D( 0, 0, 0, 0 ) );
	temp5 = _mm_shuffle_epi32( temp5, R_SHUFFLE_D( 0, 0, 0, 0 ) );

	temp2 = _mm_cvtsi32_si128( minAlpha_ );
	temp2 = _mm_shufflelo_epi16( temp2, R_SHUFFLE_D( 0, 0, 0, 0 ) );
	temp2 = _mm_shuffle_epi32( temp2, R_SHUFFLE_D( 0, 0, 0, 0 ) );

	temp7 = _mm_mullo_epi16( temp5, (const __m128i &)SIMD_SSE2_word_scale_7_5_3_1 );
	temp5 = _mm_mullo_epi16( temp5, (const __m128i &)SIMD_SSE2_word_scale_7_9_11_13 );
	temp3 = _mm_mullo_epi16( temp2, (const __m128i &)SIMD_SSE2_word_scale_7_9_11_13 );
	temp2 = _mm_mullo_epi16( temp2, (const __m128i &)SIMD_SSE2_word_scale_7_5_3_1 );

	temp5 = _mm_add_epi16( temp5, temp2 );
	temp7 = _mm_add_epi16( temp7, temp3 );

	temp5 = _mm_add_epi16( temp5, (const __m128i &)SIMD_SSE2_word_7 );
	temp7 = _mm_add_epi16( temp7, (const __m128i &)SIMD_SSE2_word_7 );

	temp5 = _mm_mulhi_epi16( temp5, (const __m128i &)SIMD_SSE2_word_div_by_14 );
	temp7 = _mm_mulhi_epi16( temp7, (const __m128i &)SIMD_SSE2_word_div_by_14 );

	temp1 = _mm_shuffle_epi32( temp5, R_SHUFFLE_D( 3, 3, 3, 3 ) );
	temp2 = _mm_shuffle_epi32( temp5, R_SHUFFLE_D( 2, 2, 2, 2 ) );
	temp3 = _mm_shuffle_epi32( temp5, R_SHUFFLE_D( 1, 1, 1, 1 ) );
	temp1 = _mm_packus_epi16( temp1, temp1 );
	temp2 = _mm_packus_epi16( temp2, temp2 );
	temp3 = _mm_packus_epi16( temp3, temp3 );

	temp0 = _mm_packus_epi16( temp0, temp6 );

	temp4 = _mm_shuffle_epi32( temp7, R_SHUFFLE_D( 0, 0, 0, 0 ) );
	temp5 = _mm_shuffle_epi32( temp7, R_SHUFFLE_D( 1, 1, 1, 1 ) );
	temp6 = _mm_shuffle_epi32( temp7, R_SHUFFLE_D( 2, 2, 2, 2 ) );
	temp7 = _mm_shuffle_epi32( temp7, R_SHUFFLE_D( 3, 3, 3, 3 ) );
	temp4 = _mm_packus_epi16( temp4, temp4 );
	temp5 = _mm_packus_epi16( temp5, temp5 );
	temp6 = _mm_packus_epi16( temp6, temp6 );
	temp7 = _mm_packus_epi16( temp7, temp7 );

	temp1 = _mm_max_epu8( temp1, temp0 );
	temp2 = _mm_max_epu8( temp2, temp0 );
	temp3 = _mm_max_epu8( temp3, temp0 );
	temp1 = _mm_cmpeq_epi8( temp1, temp0 );
	temp2 = _mm_cmpeq_epi8( temp2, temp0 );
	temp3 = _mm_cmpeq_epi8( temp3, temp0 );
	temp4 = _mm_max_epu8( temp4, temp0 );
	temp5 = _mm_max_epu8( temp5, temp0 );
	temp6 = _mm_max_epu8( temp6, temp0 );
	temp7 = _mm_max_epu8( temp7, temp0 );
	temp4 = _mm_cmpeq_epi8( temp4, temp0 );
	temp5 = _mm_cmpeq_epi8( temp5, temp0 );
	temp6 = _mm_cmpeq_epi8( temp6, temp0 );
	temp7 = _mm_cmpeq_epi8( temp7, temp0 );
	temp0 = _mm_adds_epi8( (const __m128i &)SIMD_SSE2_byte_8, temp1 );
	temp2 = _mm_adds_epi8( temp2, temp3 );
	temp4 = _mm_adds_epi8( temp4, temp5 );
	temp6 = _mm_adds_epi8( temp6, temp7 );
	temp0 = _mm_adds_epi8( temp0, temp2 );
	temp4 = _mm_adds_epi8( temp4, temp6 );
	temp0 = _mm_adds_epi8( temp0, temp4 );
	temp0 = _mm_and_si128( temp0, (const __m128i &)SIMD_SSE2_byte_7 );
	temp1 = _mm_cmpgt_epi8( (const __m128i &)SIMD_SSE2_byte_2, temp0 );
	temp1 = _mm_and_si128( temp1, (const __m128i &)SIMD_SSE2_byte_1 );
	temp0 = _mm_xor_si128( temp0, temp1 );

	temp1 = _mm_srli_epi64( temp0,  8 -  3 );
	temp2 = _mm_srli_epi64( temp0, 16 -  6 );
	temp3 = _mm_srli_epi64( temp0, 24 -  9 );
	temp4 = _mm_srli_epi64( temp0, 32 - 12 );
	temp5 = _mm_srli_epi64( temp0, 40 - 15 );
	temp6 = _mm_srli_epi64( temp0, 48 - 18 );
	temp7 = _mm_srli_epi64( temp0, 56 - 21 );
	temp0 = _mm_and_si128( temp0, (const __m128i &)SIMD_SSE2_dword_alpha_bit_mask0 );
	temp1 = _mm_and_si128( temp1, (const __m128i &)SIMD_SSE2_dword_alpha_bit_mask1 );
	temp2 = _mm_and_si128( temp2, (const __m128i &)SIMD_SSE2_dword_alpha_bit_mask2 );
	temp3 = _mm_and_si128( temp3, (const __m128i &)SIMD_SSE2_dword_alpha_bit_mask3 );
	temp4 = _mm_and_si128( temp4, (const __m128i &)SIMD_SSE2_dword_alpha_bit_mask4 );
	temp5 = _mm_and_si128( temp5, (const __m128i &)SIMD_SSE2_dword_alpha_bit_mask5 );
	temp6 = _mm_and_si128( temp6, (const __m128i &)SIMD_SSE2_dword_alpha_bit_mask6 );
	temp7 = _mm_and_si128( temp7, (const __m128i &)SIMD_SSE2_dword_alpha_bit_mask7 );
	temp0 = _mm_or_si128( temp0, temp1 );
	temp2 = _mm_or_si128( temp2, temp3 );
	temp4 = _mm_or_si128( temp4, temp5 );
	temp6 = _mm_or_si128( temp6, temp7 );
	temp0 = _mm_or_si128( temp0, temp2 );
	temp4 = _mm_or_si128( temp4, temp6 );
	temp0 = _mm_or_si128( temp0, temp4 );


	int out = _mm_cvtsi128_si32( temp0 );
	EmitUInt( out );
	outData--;

	temp1 = _mm_shuffle_epi32( temp0, R_SHUFFLE_D( 2, 3, 0, 1 ) );

	out = _mm_cvtsi128_si32( temp1 );
	EmitUInt( out );
	outData--;
#else
	assert( false );
#endif
}

/*
========================
idDxtEncoder::EmitAlphaIndices_SSE2
========================
*/
void idDxtEncoder::EmitAlphaIndices_SSE2( const byte *block, const int channelBitOffset, const int minAlpha_, const int maxAlpha_ ) {
#if ( defined( ID_WIN_X86_ASM ) || defined( ID_MAC_X86_ASM ) ) || defined ( ID_QNX_X86_SSE2_ASM )
	assert( maxAlpha_ >= minAlpha_ );

	byte *outPtr = outData;

#if ( defined( ID_WIN_X86_ASM ) || defined( ID_MAC_X86_ASM ) )
	__asm {
		movd		xmm7, channelBitOffset

		mov			esi, block
		movdqa		xmm0, xmmword ptr [esi+	0]
		movdqa		xmm5, xmmword ptr [esi+16]
		movdqa		xmm6, xmmword ptr [esi+32]
		movdqa		xmm4, xmmword ptr [esi+48]

		psrld		xmm0, xmm7
		psrld		xmm5, xmm7
		psrld		xmm6, xmm7
		psrld		xmm4, xmm7

		pand		xmm0, SIMD_SSE2_dword_byte_mask
		pand		xmm5, SIMD_SSE2_dword_byte_mask
		pand		xmm6, SIMD_SSE2_dword_byte_mask
		pand		xmm4, SIMD_SSE2_dword_byte_mask

		packuswb	xmm0, xmm5
		packuswb	xmm6, xmm4

		//---------------------

		// ab0 = (  7 * maxAlpha +  7 * minAlpha + ALPHA_RANGE ) / 14
		// ab3 = (  9 * maxAlpha +  5 * minAlpha + ALPHA_RANGE ) / 14
		// ab2 = ( 11 * maxAlpha +  3 * minAlpha + ALPHA_RANGE ) / 14
		// ab1 = ( 13 * maxAlpha +  1 * minAlpha + ALPHA_RANGE ) / 14

		// ab4 = (  7 * maxAlpha +  7 * minAlpha + ALPHA_RANGE ) / 14
		// ab5 = (  5 * maxAlpha +  9 * minAlpha + ALPHA_RANGE ) / 14
		// ab6 = (  3 * maxAlpha + 11 * minAlpha + ALPHA_RANGE ) / 14
		// ab7 = (  1 * maxAlpha + 13 * minAlpha + ALPHA_RANGE ) / 14

		movd		xmm5, maxAlpha_
		pshuflw		xmm5, xmm5,	R_SHUFFLE_D( 0,	0, 0, 0	)
		pshufd		xmm5, xmm5,	R_SHUFFLE_D( 0,	0, 0, 0	)
		movdqa		xmm7, xmm5

		movd		xmm2, minAlpha_
		pshuflw		xmm2, xmm2,	R_SHUFFLE_D( 0,	0, 0, 0	)
		pshufd		xmm2, xmm2,	R_SHUFFLE_D( 0,	0, 0, 0	)
		movdqa		xmm3, xmm2

		pmullw		xmm5, SIMD_SSE2_word_scale_7_9_11_13
		pmullw		xmm7, SIMD_SSE2_word_scale_7_5_3_1
		pmullw		xmm2, SIMD_SSE2_word_scale_7_5_3_1
		pmullw		xmm3, SIMD_SSE2_word_scale_7_9_11_13

		paddw		xmm5, xmm2
		paddw		xmm7, xmm3

		paddw		xmm5, SIMD_SSE2_word_7
		paddw		xmm7, SIMD_SSE2_word_7

		pmulhw		xmm5, SIMD_SSE2_word_div_by_14			// * ( ( 1 << 16 ) / 14	+ 1	) )	>> 16
		pmulhw		xmm7, SIMD_SSE2_word_div_by_14			// * ( ( 1 << 16 ) / 14	+ 1	) )	>> 16

		pshufd		xmm1, xmm5,	R_SHUFFLE_D( 3, 3, 3, 3	)
		pshufd		xmm2, xmm5,	R_SHUFFLE_D( 2, 2, 2, 2	)
		pshufd		xmm3, xmm5,	R_SHUFFLE_D( 1, 1, 1, 1	)
		packuswb	xmm1, xmm1								// ab1
		packuswb	xmm2, xmm2								// ab2
		packuswb	xmm3, xmm3								// ab3

		packuswb	xmm0, xmm6								// alpha block

		pshufd		xmm4, xmm7,	R_SHUFFLE_D( 0,	0, 0, 0	)
		pshufd		xmm5, xmm7,	R_SHUFFLE_D( 1,	1, 1, 1	)
		pshufd		xmm6, xmm7,	R_SHUFFLE_D( 2,	2, 2, 2	)
		pshufd		xmm7, xmm7,	R_SHUFFLE_D( 3,	3, 3, 3	)
		packuswb	xmm4, xmm4								// ab4
		packuswb	xmm5, xmm5								// ab5
		packuswb	xmm6, xmm6								// ab6
		packuswb	xmm7, xmm7								// ab7

		pmaxub		xmm1, xmm0
		pmaxub		xmm2, xmm0
		pmaxub		xmm3, xmm0
		pcmpeqb		xmm1, xmm0
		pcmpeqb		xmm2, xmm0
		pcmpeqb		xmm3, xmm0
		pmaxub		xmm4, xmm0
		pmaxub		xmm5, xmm0
		pmaxub		xmm6, xmm0
		pmaxub		xmm7, xmm0
		pcmpeqb		xmm4, xmm0
		pcmpeqb		xmm5, xmm0
		pcmpeqb		xmm6, xmm0
		pcmpeqb		xmm7, xmm0
		movdqa		xmm0, SIMD_SSE2_byte_8
		paddsb		xmm0, xmm1
		paddsb		xmm2, xmm3
		paddsb		xmm4, xmm5
		paddsb		xmm6, xmm7
		paddsb		xmm0, xmm2
		paddsb		xmm4, xmm6
		paddsb		xmm0, xmm4
		pand		xmm0, SIMD_SSE2_byte_7
		movdqa		xmm1, SIMD_SSE2_byte_2
		pcmpgtb		xmm1, xmm0
		pand		xmm1, SIMD_SSE2_byte_1
		pxor		xmm0, xmm1
		movdqa		xmm1, xmm0
		movdqa		xmm2, xmm0
		movdqa		xmm3, xmm0
		movdqa		xmm4, xmm0
		movdqa		xmm5, xmm0
		movdqa		xmm6, xmm0
		movdqa		xmm7, xmm0
		psrlq		xmm1,  8- 3
		psrlq		xmm2, 16- 6
		psrlq		xmm3, 24- 9
		psrlq		xmm4, 32-12
		psrlq		xmm5, 40-15
		psrlq		xmm6, 48-18
		psrlq		xmm7, 56-21
		pand		xmm0, SIMD_SSE2_dword_alpha_bit_mask0
		pand		xmm1, SIMD_SSE2_dword_alpha_bit_mask1
		pand		xmm2, SIMD_SSE2_dword_alpha_bit_mask2
		pand		xmm3, SIMD_SSE2_dword_alpha_bit_mask3
		pand		xmm4, SIMD_SSE2_dword_alpha_bit_mask4
		pand		xmm5, SIMD_SSE2_dword_alpha_bit_mask5
		pand		xmm6, SIMD_SSE2_dword_alpha_bit_mask6
		pand		xmm7, SIMD_SSE2_dword_alpha_bit_mask7
		por			xmm0, xmm1
		por			xmm2, xmm3
		por			xmm4, xmm5
		por			xmm6, xmm7
		por			xmm0, xmm2
		por			xmm4, xmm6
		por			xmm0, xmm4
		mov			esi, outPtr
		movd		[esi+0], xmm0
		pshufd		xmm1, xmm0, R_SHUFFLE_D( 2, 3, 0, 1 )
		movd		[esi+3], xmm1
	}
#else
	__asm__ __volatile__(
			"movd		%[channelBitOffset], %%xmm7\n"

			"movdqa		 0(%[block]), %%xmm0\n"
			"movdqa		16(%[block]), %%xmm5\n"
			"movdqa		32(%[block]), %%xmm6\n"
			"movdqa		48(%[block]), %%xmm4\n"

			"psrld		%%xmm7, %%xmm0\n"
			"psrld		%%xmm7, %%xmm5\n"
			"psrld		%%xmm7, %%xmm6\n"
			"psrld		%%xmm7, %%xmm4\n"

			"pand		_ZL25SIMD_SSE2_dword_byte_mask, %%xmm0\n"
			"pand		_ZL25SIMD_SSE2_dword_byte_mask, %%xmm5\n"
			"pand		_ZL25SIMD_SSE2_dword_byte_mask, %%xmm6\n"
			"pand		_ZL25SIMD_SSE2_dword_byte_mask, %%xmm4\n"

			"packuswb	%%xmm5, %%xmm0\n"
			"packuswb	%%xmm4, %%xmm6\n"

			//---------------------

			// ab0 = (  7 * maxAlpha +  7 * minAlpha + ALPHA_RANGE ) / 14
			// ab3 = (  9 * maxAlpha +  5 * minAlpha + ALPHA_RANGE ) / 14
			// ab2 = ( 11 * maxAlpha +  3 * minAlpha + ALPHA_RANGE ) / 14
			// ab1 = ( 13 * maxAlpha +  1 * minAlpha + ALPHA_RANGE ) / 14

			// ab4 = (  7 * maxAlpha +  7 * minAlpha + ALPHA_RANGE ) / 14
			// ab5 = (  5 * maxAlpha +  9 * minAlpha + ALPHA_RANGE ) / 14
			// ab6 = (  3 * maxAlpha + 11 * minAlpha + ALPHA_RANGE ) / 14
			// ab7 = (  1 * maxAlpha + 13 * minAlpha + ALPHA_RANGE ) / 14

			"movd		%[maxAlpha], %%xmm5\n"
			"pshuflw	%[shuf0000], %%xmm5, %%xmm5\n"
			"pshufd		%[shuf0000], %%xmm5, %%xmm5\n"
			"movdqa		%%xmm5, %%xmm7\n"

			"movd		%[minAlpha], %%xmm2\n"
			"pshuflw	%[shuf0000], %%xmm2, %%xmm2\n"
			"pshufd		%[shuf0000], %%xmm2, %%xmm2\n"
			"movdqa		%%xmm2, %%xmm3\n"

			"pmullw		_ZL30SIMD_SSE2_word_scale_7_9_11_13, %%xmm5\n"
			"pmullw		_ZL28SIMD_SSE2_word_scale_7_5_3_1, %%xmm7\n"
			"pmullw		_ZL28SIMD_SSE2_word_scale_7_5_3_1, %%xmm2\n"
			"pmullw		_ZL30SIMD_SSE2_word_scale_7_9_11_13, %%xmm3\n"

			"paddw		%%xmm2, %%xmm5\n"
			"paddw		%%xmm3, %%xmm7\n"

			"paddw		_ZL16SIMD_SSE2_word_7, %%xmm5\n"
			"paddw		_ZL16SIMD_SSE2_word_7, %%xmm7\n"

			"pmulhw		_ZL24SIMD_SSE2_word_div_by_14, %%xmm5\n"		// * ( ( 1 << 16 ) / 14	+ 1	) )	>> 16
			"pmulhw		_ZL24SIMD_SSE2_word_div_by_14, %%xmm7\n"		// * ( ( 1 << 16 ) / 14	+ 1	) )	>> 16

			"pshufd		%[shuf3333], %%xmm5, %%xmm1\n"
			"pshufd		%[shuf2222], %%xmm5, %%xmm2\n"
			"pshufd		%[shuf1111], %%xmm5, %%xmm3\n"
			"packuswb	%%xmm1, %%xmm1\n"								// ab1
			"packuswb	%%xmm2, %%xmm2\n"								// ab2
			"packuswb	%%xmm3, %%xmm3\n"								// ab3

			"packuswb	%%xmm6, %%xmm0\n"								// alpha block

			"pshufd		%[shuf0000], %%xmm7, %%xmm4\n"
			"pshufd		%[shuf1111], %%xmm7, %%xmm5\n"
			"pshufd		%[shuf2222], %%xmm7, %%xmm6\n"
			"pshufd		%[shuf3333], %%xmm7, %%xmm7\n"
			"packuswb	%%xmm4, %%xmm4\n"								// ab4
			"packuswb	%%xmm5, %%xmm5\n"								// ab5
			"packuswb	%%xmm6, %%xmm6\n"								// ab6
			"packuswb	%%xmm7, %%xmm7\n"								// ab7

			"pmaxub		%%xmm0, %%xmm1\n"
			"pmaxub		%%xmm0, %%xmm2\n"
			"pmaxub		%%xmm0, %%xmm3\n"
			"pcmpeqb	%%xmm0, %%xmm1\n"
			"pcmpeqb	%%xmm0, %%xmm2\n"
			"pcmpeqb	%%xmm0, %%xmm3\n"
			"pmaxub		%%xmm0, %%xmm4\n"
			"pmaxub		%%xmm0, %%xmm5\n"
			"pmaxub		%%xmm0, %%xmm6\n"
			"pmaxub		%%xmm0, %%xmm7\n"
			"pcmpeqb	%%xmm0, %%xmm4\n"
			"pcmpeqb	%%xmm0, %%xmm5\n"
			"pcmpeqb	%%xmm0, %%xmm6\n"
			"pcmpeqb	%%xmm0, %%xmm7\n"
			"movdqa		_ZL16SIMD_SSE2_byte_8, %%xmm0\n"
			"paddsb		%%xmm1, %%xmm0\n"
			"paddsb		%%xmm3, %%xmm2\n"
			"paddsb		%%xmm5, %%xmm4\n"
			"paddsb		%%xmm7, %%xmm6\n"
			"paddsb		%%xmm2, %%xmm0\n"
			"paddsb		%%xmm6, %%xmm4\n"
			"paddsb		%%xmm4, %%xmm0\n"
			"pand		_ZL16SIMD_SSE2_byte_7, %%xmm0\n"
			"movdqa		_ZL16SIMD_SSE2_byte_2, %%xmm1\n"
			"pcmpgtb	%%xmm0, %%xmm1\n"
			"pand		_ZL16SIMD_SSE2_byte_1, %%xmm1\n"
			"pxor		%%xmm1, %%xmm0\n"
			"movdqa		%%xmm0, %%xmm1\n"
			"movdqa		%%xmm0, %%xmm2\n"
			"movdqa		%%xmm0, %%xmm3\n"
			"movdqa		%%xmm0, %%xmm4\n"
			"movdqa		%%xmm0, %%xmm5\n"
			"movdqa		%%xmm0, %%xmm6\n"
			"movdqa		%%xmm0, %%xmm7\n"
			"psrlq		$( 8- 3), %%xmm1\n"
			"psrlq		$(16- 6), %%xmm2\n"
			"psrlq		$(24- 9), %%xmm3\n"
			"psrlq		$(32-12), %%xmm4\n"
			"psrlq		$(40-15), %%xmm5\n"
			"psrlq		$(48-18), %%xmm6\n"
			"psrlq		$(56-21), %%xmm7\n"
			"pand		_ZL31SIMD_SSE2_dword_alpha_bit_mask0, %%xmm0\n"
			"pand		_ZL31SIMD_SSE2_dword_alpha_bit_mask1, %%xmm1\n"
			"pand		_ZL31SIMD_SSE2_dword_alpha_bit_mask2, %%xmm2\n"
			"pand		_ZL31SIMD_SSE2_dword_alpha_bit_mask3, %%xmm3\n"
			"pand		_ZL31SIMD_SSE2_dword_alpha_bit_mask4, %%xmm4\n"
			"pand		_ZL31SIMD_SSE2_dword_alpha_bit_mask5, %%xmm5\n"
			"pand		_ZL31SIMD_SSE2_dword_alpha_bit_mask6, %%xmm6\n"
			"pand		_ZL31SIMD_SSE2_dword_alpha_bit_mask7, %%xmm7\n"
			"por		%%xmm1, %%xmm0\n"
			"por		%%xmm3, %%xmm2\n"
			"por		%%xmm5, %%xmm4\n"
			"por		%%xmm7, %%xmm6\n"
			"por		%%xmm2, %%xmm0\n"
			"por		%%xmm6, %%xmm4\n"
			"por		%%xmm4, %%xmm0\n"
			"movd		%%xmm0, 0(%[outPtr])\n"
			"pshufd		%[shuf2301], %%xmm0, %%xmm1\n"
			"movd		%%xmm1, 3(%[outPtr])\n"
			:: [block] "r" (block), [channelBitOffset] "r" (channelBitOffset), [minAlpha] "r" (minAlpha_), [maxAlpha] "r" (maxAlpha_), [outPtr] "r" (outPtr),
			[shuf0000] "i" R_SHUFFLE_D( 0, 0, 0, 0 ), [shuf1111] "i" R_SHUFFLE_D( 1, 1, 1, 1 ), [shuf2222] "i" R_SHUFFLE_D( 2, 2, 2, 2 ), [shuf3333] "i" R_SHUFFLE_D( 3, 3, 3, 3 ),
			[shuf2301] "i" R_SHUFFLE_D( 2, 3, 0, 1 )
			: "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "memory");
#endif

	outData += 6;
#elif defined ( ID_WIN_X86_SSE2_INTRIN ) || defined ( ID_QNX_X86_SSE2_INTRIN )
	__m128i block0 = *((__m128i *)(&block[ 0]));
	__m128i block1 = *((__m128i *)(&block[16]));
	__m128i block2 = *((__m128i *)(&block[32]));
	__m128i block3 = *((__m128i *)(&block[48]));
	__m128c temp0, temp1, temp2, temp3, temp4, temp5, temp6, temp7;

	temp7 = _mm_cvtsi32_si128( channelBitOffset );

	temp0 = _mm_srl_epi32( block0, temp7 );
	temp5 = _mm_srl_epi32( block1, temp7 );
	temp6 = _mm_srl_epi32( block2, temp7 );
	temp4 = _mm_srl_epi32( block3, temp7 );

	temp0 = _mm_and_si128( temp0, (const __m128i &)SIMD_SSE2_dword_byte_mask );
	temp5 = _mm_and_si128( temp5, (const __m128i &)SIMD_SSE2_dword_byte_mask );
	temp6 = _mm_and_si128( temp6, (const __m128i &)SIMD_SSE2_dword_byte_mask );
	temp4 = _mm_and_si128( temp4, (const __m128i &)SIMD_SSE2_dword_byte_mask );

	temp0 = _mm_packus_epi16( temp0, temp5 );
	temp6 = _mm_packus_epi16( temp6, temp4 );

	//---------------------

	// ab0 = (  7 * maxAlpha +  7 * minAlpha + ALPHA_RANGE ) / 14
	// ab3 = (  9 * maxAlpha +  5 * minAlpha + ALPHA_RANGE ) / 14
	// ab2 = ( 11 * maxAlpha +  3 * minAlpha + ALPHA_RANGE ) / 14
	// ab1 = ( 13 * maxAlpha +  1 * minAlpha + ALPHA_RANGE ) / 14

	// ab4 = (  7 * maxAlpha +  7 * minAlpha + ALPHA_RANGE ) / 14
	// ab5 = (  5 * maxAlpha +  9 * minAlpha + ALPHA_RANGE ) / 14
	// ab6 = (  3 * maxAlpha + 11 * minAlpha + ALPHA_RANGE ) / 14
	// ab7 = (  1 * maxAlpha + 13 * minAlpha + ALPHA_RANGE ) / 14

	temp5 = _mm_cvtsi32_si128( maxAlpha_ );
	temp5 = _mm_shufflelo_epi16( temp5, R_SHUFFLE_D( 0, 0, 0, 0 ) );
	temp5 = _mm_shuffle_epi32( temp5, R_SHUFFLE_D( 0, 0, 0, 0 ) );

	temp2 = _mm_cvtsi32_si128( minAlpha_ );
	temp2 = _mm_shufflelo_epi16( temp2, R_SHUFFLE_D( 0, 0, 0, 0 ) );
	temp2 = _mm_shuffle_epi32( temp2, R_SHUFFLE_D( 0, 0, 0, 0 ) );

	temp7 = _mm_mullo_epi16( temp5, (const __m128i &)SIMD_SSE2_word_scale_7_5_3_1 );
	temp5 = _mm_mullo_epi16( temp5, (const __m128i &)SIMD_SSE2_word_scale_7_9_11_13 );
	temp3 = _mm_mullo_epi16( temp2, (const __m128i &)SIMD_SSE2_word_scale_7_9_11_13 );
	temp2 = _mm_mullo_epi16( temp2, (const __m128i &)SIMD_SSE2_word_scale_7_5_3_1 );

	temp5 = _mm_add_epi16( temp5, temp2 );
	temp7 = _mm_add_epi16( temp7, temp3 );

	temp5 = _mm_add_epi16( temp5, (const __m128i &)SIMD_SSE2_word_7 );
	temp7 = _mm_add_epi16( temp7, (const __m128i &)SIMD_SSE2_word_7 );

	temp5 = _mm_mulhi_epi16( temp5, (const __m128i &)SIMD_SSE2_word_div_by_14 );
	temp7 = _mm_mulhi_epi16( temp7, (const __m128i &)SIMD_SSE2_word_div_by_14 );

	temp1 = _mm_shuffle_epi32( temp5, R_SHUFFLE_D( 3, 3, 3, 3 ) );
	temp2 = _mm_shuffle_epi32( temp5, R_SHUFFLE_D( 2, 2, 2, 2 ) );
	temp3 = _mm_shuffle_epi32( temp5, R_SHUFFLE_D( 1, 1, 1, 1 ) );
	temp1 = _mm_packus_epi16( temp1, temp1 );
	temp2 = _mm_packus_epi16( temp2, temp2 );
	temp3 = _mm_packus_epi16( temp3, temp3 );

	temp0 = _mm_packus_epi16( temp0, temp6 );

	temp4 = _mm_shuffle_epi32( temp7, R_SHUFFLE_D( 0, 0, 0, 0 ) );
	temp5 = _mm_shuffle_epi32( temp7, R_SHUFFLE_D( 1, 1, 1, 1 ) );
	temp6 = _mm_shuffle_epi32( temp7, R_SHUFFLE_D( 2, 2, 2, 2 ) );
	temp7 = _mm_shuffle_epi32( temp7, R_SHUFFLE_D( 3, 3, 3, 3 ) );
	temp4 = _mm_packus_epi16( temp4, temp4 );
	temp5 = _mm_packus_epi16( temp5, temp5 );
	temp6 = _mm_packus_epi16( temp6, temp6 );
	temp7 = _mm_packus_epi16( temp7, temp7 );

	temp1 = _mm_max_epu8( temp1, temp0 );
	temp2 = _mm_max_epu8( temp2, temp0 );
	temp3 = _mm_max_epu8( temp3, temp0 );
	temp1 = _mm_cmpeq_epi8( temp1, temp0 );
	temp2 = _mm_cmpeq_epi8( temp2, temp0 );
	temp3 = _mm_cmpeq_epi8( temp3, temp0 );
	temp4 = _mm_max_epu8( temp4, temp0 );
	temp5 = _mm_max_epu8( temp5, temp0 );
	temp6 = _mm_max_epu8( temp6, temp0 );
	temp7 = _mm_max_epu8( temp7, temp0 );
	temp4 = _mm_cmpeq_epi8( temp4, temp0 );
	temp5 = _mm_cmpeq_epi8( temp5, temp0 );
	temp6 = _mm_cmpeq_epi8( temp6, temp0 );
	temp7 = _mm_cmpeq_epi8( temp7, temp0 );
	temp0 = _mm_adds_epi8( (const __m128i &)SIMD_SSE2_byte_8, temp1 );
	temp2 = _mm_adds_epi8( temp2, temp3 );
	temp4 = _mm_adds_epi8( temp4, temp5 );
	temp6 = _mm_adds_epi8( temp6, temp7 );
	temp0 = _mm_adds_epi8( temp0, temp2 );
	temp4 = _mm_adds_epi8( temp4, temp6 );
	temp0 = _mm_adds_epi8( temp0, temp4 );
	temp0 = _mm_and_si128( temp0, (const __m128i &)SIMD_SSE2_byte_7 );
	temp1 = _mm_cmpgt_epi8( (const __m128i &)SIMD_SSE2_byte_2, temp0 );
	temp1 = _mm_and_si128( temp1, (const __m128i &)SIMD_SSE2_byte_1 );
	temp0 = _mm_xor_si128( temp0, temp1 );

	temp1 = _mm_srli_epi64( temp0,  8 -  3 );
	temp2 = _mm_srli_epi64( temp0, 16 -  6 );
	temp3 = _mm_srli_epi64( temp0, 24 -  9 );
	temp4 = _mm_srli_epi64( temp0, 32 - 12 );
	temp5 = _mm_srli_epi64( temp0, 40 - 15 );
	temp6 = _mm_srli_epi64( temp0, 48 - 18 );
	temp7 = _mm_srli_epi64( temp0, 56 - 21 );
	temp0 = _mm_and_si128( temp0, (const __m128i &)SIMD_SSE2_dword_alpha_bit_mask0 );
	temp1 = _mm_and_si128( temp1, (const __m128i &)SIMD_SSE2_dword_alpha_bit_mask1 );
	temp2 = _mm_and_si128( temp2, (const __m128i &)SIMD_SSE2_dword_alpha_bit_mask2 );
	temp3 = _mm_and_si128( temp3, (const __m128i &)SIMD_SSE2_dword_alpha_bit_mask3 );
	temp4 = _mm_and_si128( temp4, (const __m128i &)SIMD_SSE2_dword_alpha_bit_mask4 );
	temp5 = _mm_and_si128( temp5, (const __m128i &)SIMD_SSE2_dword_alpha_bit_mask5 );
	temp6 = _mm_and_si128( temp6, (const __m128i &)SIMD_SSE2_dword_alpha_bit_mask6 );
	temp7 = _mm_and_si128( temp7, (const __m128i &)SIMD_SSE2_dword_alpha_bit_mask7 );
	temp0 = _mm_or_si128( temp0, temp1 );
	temp2 = _mm_or_si128( temp2, temp3 );
	temp4 = _mm_or_si128( temp4, temp5 );
	temp6 = _mm_or_si128( temp6, temp7 );
	temp0 = _mm_or_si128( temp0, temp2 );
	temp4 = _mm_or_si128( temp4, temp6 );
	temp0 = _mm_or_si128( temp0, temp4 );


	int out = _mm_cvtsi128_si32( temp0 );
	EmitUInt( out );
	outData--;

	temp1 = _mm_shuffle_epi32( temp0, R_SHUFFLE_D( 2, 3, 0, 1 ) );

	out = _mm_cvtsi128_si32( temp1 );
	EmitUInt( out );
	outData--;
#else
	assert( false );
#endif
}

/*
========================
idDxtEncoder::CompressImageDXT1Fast_SSE2

params:	inBuf		- image to compress
paramO:	outBuf		- result of compression
params:	width		- width of image
params:	height		- height of image
========================
*/
void idDxtEncoder::CompressImageDXT1Fast_SSE2( const byte *inBuf, byte *outBuf, int width, int height ) {
	ALIGN16( byte block[64] );
	ALIGN16( byte minColor[4] );
	ALIGN16( byte maxColor[4] );

	assert( width >= 4 && ( width & 3 ) == 0 );
	assert( height >= 4 && ( height & 3 ) == 0 );

	this->width = width;
	this->height = height;
	this->outData = outBuf;


	for ( int j = 0; j < height; j += 4, inBuf += width * 4*4 ) {
		for ( int i = 0; i < width; i += 4 ) {
			ExtractBlock_SSE2( inBuf + i * 4, width, block );
			GetMinMaxBBox_SSE2( block, minColor, maxColor );
			InsetColorsBBox_SSE2( minColor, maxColor );

			EmitUShort( ColorTo565( maxColor ) );
			EmitUShort( ColorTo565( minColor ) );

			EmitColorIndices_SSE2( block, minColor, maxColor );
		}
		outData += dstPadding;
		inBuf += srcPadding;
	}

#ifdef TEST_COMPRESSION
	int tmpDstPadding = dstPadding;
	dstPadding = 0;
	byte * testOutBuf = (byte *) _alloca16( width * height / 2 );
	CompressImageDXT1Fast_Generic( inBuf, testOutBuf, width, height );
	for ( int j = 0; j < height/4; j++ ) {
		for ( int i = 0; i < width/4; i++ ) {
			byte * ptr1 = outBuf + ( j * width/4 + i ) * 8 + j * tmpDstPadding;
			byte * ptr2 = testOutBuf + ( j * width/4 + i ) * 8;
			for ( int k = 0; k < 8; k++ ) {
				assert( ptr1[k] == ptr2[k] );
			}
		}
	}
	dstPadding = tmpDstPadding;
#endif
}

/*
========================
idDxtEncoder::CompressImageDXT1AlphaFast_SSE2

params:	inBuf		- image to compress
paramO:	outBuf		- result of compression
params:	width		- width of image
params:	height		- height of image
========================
*/
void idDxtEncoder::CompressImageDXT1AlphaFast_SSE2( const byte *inBuf, byte *outBuf, int width, int height ) {
	ALIGN16( byte block[64] );
	ALIGN16( byte minColor[4] );
	ALIGN16( byte maxColor[4] );

	assert( width >= 4 && ( width & 3 ) == 0 );
	assert( height >= 4 && ( height & 3 ) == 0 );

	this->width = width;
	this->height = height;
	this->outData = outBuf;

	for ( int j = 0; j < height; j += 4, inBuf += width * 4*4 ) {
		for ( int i = 0; i < width; i += 4 ) {
			ExtractBlock_SSE2( inBuf + i * 4, width, block );
			GetMinMaxBBox_SSE2( block, minColor, maxColor );
			byte minAlpha = minColor[3];
			InsetColorsBBox_SSE2( minColor, maxColor );

			if ( minAlpha >= 128 ) {
				EmitUShort( ColorTo565( maxColor ) );
				EmitUShort( ColorTo565( minColor ) );
				EmitColorIndices_SSE2( block, minColor, maxColor );
			} else {
				EmitUShort( ColorTo565( minColor ) );
				EmitUShort( ColorTo565( maxColor ) );
				EmitColorAlphaIndices_SSE2( block, minColor, maxColor );
			}
		}
		outData += dstPadding;
		inBuf += srcPadding;
	}

#ifdef TEST_COMPRESSION
	int tmpDstPadding = dstPadding;
	dstPadding = 0;
	byte * testOutBuf = (byte *) _alloca16( width * height / 2 );
	CompressImageDXT1AlphaFast_Generic( inBuf, testOutBuf, width, height );
	for ( int j = 0; j < height/4; j++ ) {
		for ( int i = 0; i < width/4; i++ ) {
			byte * ptr1 = outBuf + ( j * width/4 + i ) * 8 + j * tmpDstPadding;
			byte * ptr2 = testOutBuf + ( j * width/4 + i ) * 8;
			for ( int k = 0; k < 8; k++ ) {
				assert( ptr1[k] == ptr2[k] );
			}
		}
	}
	dstPadding = tmpDstPadding;
#endif
}

/*
========================
idDxtEncoder::CompressImageDXT5Fast_SSE2

params:	inBuf		- image to compress
paramO:	outBuf		- result of compression
params:	width		- width of image
params:	height		- height of image
========================
*/
void idDxtEncoder::CompressImageDXT5Fast_SSE2( const byte *inBuf, byte *outBuf, int width, int height ) {
	ALIGN16( byte block[64] );
	ALIGN16( byte minColor[4] );
	ALIGN16( byte maxColor[4] );

	assert( width >= 4 && ( width & 3 ) == 0 );
	assert( height >= 4 && ( height & 3 ) == 0 );

	this->width = width;
	this->height = height;
	this->outData = outBuf;

	for ( int j = 0; j < height; j += 4, inBuf += width * 4*4 ) {
		for ( int i = 0; i < width; i += 4 ) {
			ExtractBlock_SSE2( inBuf + i * 4, width, block );
			GetMinMaxBBox_SSE2( block, minColor, maxColor );
			InsetColorsBBox_SSE2( minColor, maxColor );

			EmitByte( maxColor[3] );
			EmitByte( minColor[3] );

			EmitAlphaIndices_SSE2( block, minColor[3], maxColor[3] );

			EmitUShort( ColorTo565( maxColor ) );
			EmitUShort( ColorTo565( minColor ) );

			EmitColorIndices_SSE2( block, minColor, maxColor );
		}
		outData += dstPadding;
		inBuf += srcPadding;
	}

#ifdef TEST_COMPRESSION
	int tmpDstPadding = dstPadding;
	dstPadding = 0;
	byte * testOutBuf = (byte *) _alloca16( width * height );
	CompressImageDXT5Fast_Generic( inBuf, testOutBuf, width, height );
	for ( int j = 0; j < height / 4; j++ ) {
		for ( int i = 0; i < width / 4; i++ ) {
			byte * ptr1 = outBuf + ( j * width/4 + i ) * 16 + j * tmpDstPadding;
			byte * ptr2 = testOutBuf + ( j * width/4 + i ) * 16;
			for ( int k = 0; k < 16; k++ ) {
				assert( ptr1[k] == ptr2[k] );
			}
		}
	}
	dstPadding = tmpDstPadding;
#endif
}

/*
========================
idDxtEncoder::ScaleYCoCg_SSE2
========================
*/
ID_INLINE void idDxtEncoder::ScaleYCoCg_SSE2( byte *colorBlock, byte *minColor, byte *maxColor ) const {
#if ( defined( ID_WIN_X86_ASM ) || defined( ID_MAC_X86_ASM ) )
	__asm {
		mov			esi, colorBlock
		mov			edx, minColor
		mov			ecx, maxColor

		movd		xmm0, dword ptr [edx]
		movd		xmm1, dword ptr [ecx]

		punpcklbw	xmm0, SIMD_SSE2_byte_0
		punpcklbw	xmm1, SIMD_SSE2_byte_0

		movdqa		xmm6, SIMD_SSE2_word_center_128
		movdqa		xmm7, SIMD_SSE2_word_center_128

		psubw		xmm6, xmm0
		psubw		xmm7, xmm1

		psubw		xmm0, SIMD_SSE2_word_center_128
		psubw		xmm1, SIMD_SSE2_word_center_128

		pmaxsw		xmm6, xmm0
		pmaxsw		xmm7, xmm1

		pmaxsw		xmm6, xmm7
		pshuflw		xmm7, xmm6, R_SHUFFLE_D( 1, 0, 1, 0 )
		pmaxsw		xmm6, xmm7
		pshufd		xmm6, xmm6, R_SHUFFLE_D( 0, 0, 0, 0 )

		movdqa		xmm7, xmm6
		pcmpgtw		xmm6, SIMD_SSE2_word_63				// mask0
		pcmpgtw		xmm7, SIMD_SSE2_word_31				// mask1

		pandn		xmm7, SIMD_SSE2_byte_2
		por			xmm7, SIMD_SSE2_byte_1
		pandn		xmm6, xmm7
		movdqa		xmm3, xmm6
		movdqa		xmm7, xmm6
		pxor		xmm7, SIMD_SSE2_byte_not
		por			xmm7, SIMD_SSE2_byte_scale_mask0	// 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00
		paddw		xmm6, SIMD_SSE2_byte_1
		pand		xmm6, SIMD_SSE2_byte_scale_mask1	// 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF
		por			xmm6, SIMD_SSE2_byte_scale_mask2	// 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00

		movd		xmm4, dword ptr [edx]
		movd		xmm5, dword ptr [ecx]

		pand		xmm4, SIMD_SSE2_byte_scale_mask3	// 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0xFF, 0xFF
		pand		xmm5, SIMD_SSE2_byte_scale_mask3

		pslld		xmm3, 3
		pand		xmm3, SIMD_SSE2_byte_scale_mask4	// 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00

		por			xmm4, xmm3
		por			xmm5, xmm3

		paddb		xmm4, SIMD_SSE2_byte_minus_128_0
		paddb		xmm5, SIMD_SSE2_byte_minus_128_0

		pmullw		xmm4, xmm6
		pmullw		xmm5, xmm6

		pand		xmm4, xmm7
		pand		xmm5, xmm7

		psubb		xmm4, SIMD_SSE2_byte_minus_128_0
		psubb		xmm5, SIMD_SSE2_byte_minus_128_0

		movd		dword ptr [edx], xmm4
		movd		dword ptr [ecx], xmm5

		movdqa		xmm0, xmmword ptr [esi+ 0*4]
		movdqa		xmm1, xmmword ptr [esi+ 4*4]
		movdqa		xmm2, xmmword ptr [esi+ 8*4]
		movdqa		xmm3, xmmword ptr [esi+12*4]

		paddb		xmm0, SIMD_SSE2_byte_minus_128_0
		paddb		xmm1, SIMD_SSE2_byte_minus_128_0
		paddb		xmm2, SIMD_SSE2_byte_minus_128_0
		paddb		xmm3, SIMD_SSE2_byte_minus_128_0

		pmullw		xmm0, xmm6
		pmullw		xmm1, xmm6
		pmullw		xmm2, xmm6
		pmullw		xmm3, xmm6

		pand		xmm0, xmm7
		pand		xmm1, xmm7
		pand		xmm2, xmm7
		pand		xmm3, xmm7

		psubb		xmm0, SIMD_SSE2_byte_minus_128_0
		psubb		xmm1, SIMD_SSE2_byte_minus_128_0
		psubb		xmm2, SIMD_SSE2_byte_minus_128_0
		psubb		xmm3, SIMD_SSE2_byte_minus_128_0

		movdqa		xmmword ptr [esi+ 0*4], xmm0
		movdqa		xmmword ptr [esi+ 4*4], xmm1
		movdqa		xmmword ptr [esi+ 8*4], xmm2
		movdqa		xmmword ptr [esi+12*4], xmm3
	}
#elif defined ( ID_QNX_X86_SSE2_ASM )
	__asm__ __volatile__(
			"movd		(%[minColor]), %%xmm0\n"
			"movd		(%[maxColor]), %%xmm1\n"

			"punpcklbw	_ZL16SIMD_SSE2_byte_0, %%xmm0\n"
			"punpcklbw	_ZL16SIMD_SSE2_byte_0, %%xmm1\n"

			"movdqa		_ZL25SIMD_SSE2_word_center_128, %%xmm6\n"
			"movdqa		_ZL25SIMD_SSE2_word_center_128, %%xmm7\n"

			"psubw		%%xmm0, %%xmm6\n"
			"psubw		%%xmm1, %%xmm7\n"

			"psubw		_ZL25SIMD_SSE2_word_center_128, %%xmm0\n"
			"psubw		_ZL25SIMD_SSE2_word_center_128, %%xmm1\n"

			"pmaxsw		%%xmm0, %%xmm6\n"
			"pmaxsw		%%xmm1, %%xmm7\n"

			"pmaxsw		%%xmm7, %%xmm6\n"
			"pshuflw	%[shuf1010], %%xmm6, %%xmm7\n"
			"pmaxsw		%%xmm7, %%xmm6\n"
			"pshufd		%[shuf0000], %%xmm6, %%xmm6\n"

			"movdqa		%%xmm6, %%xmm7\n"
			"pcmpgtw	_ZL17SIMD_SSE2_word_63, %%xmm6\n"			// mask0
			"pcmpgtw	_ZL17SIMD_SSE2_word_31, %%xmm7\n"			// mask1

			"pandn		_ZL16SIMD_SSE2_byte_2, %%xmm7\n"
			"por		_ZL16SIMD_SSE2_byte_1, %%xmm7\n"
			"pandn		%%xmm7, %%xmm6\n"
			"movdqa		%%xmm6, %%xmm3\n"
			"movdqa		%%xmm6, %%xmm7\n"
			"pxor		_ZL18SIMD_SSE2_byte_not, %%xmm7\n"
			"por		_ZL26SIMD_SSE2_byte_scale_mask0, %%xmm7\n"	// 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00
			"paddw		_ZL16SIMD_SSE2_byte_1, %%xmm6\n"
			"pand		_ZL26SIMD_SSE2_byte_scale_mask1, %%xmm6\n"	// 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF
			"por		_ZL26SIMD_SSE2_byte_scale_mask2, %%xmm6\n"	// 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00

			"movd		(%[minColor]), %%xmm4\n"
			"movd		(%[maxColor]), %%xmm5\n"

			"pand		_ZL26SIMD_SSE2_byte_scale_mask3, %%xmm4\n"	// 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0xFF, 0xFF
			"pand		_ZL26SIMD_SSE2_byte_scale_mask3, %%xmm5\n"

			"pslld		$3, %%xmm3\n"
			"pand		_ZL26SIMD_SSE2_byte_scale_mask4, %%xmm3\n"	// 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00

			"por		%%xmm3, %%xmm4\n"
			"por		%%xmm3, %%xmm5\n"

			"paddb		_ZL26SIMD_SSE2_byte_minus_128_0, %%xmm4\n"
			"paddb		_ZL26SIMD_SSE2_byte_minus_128_0, %%xmm5\n"

			"pmullw		%%xmm6, %%xmm4\n"
			"pmullw		%%xmm6, %%xmm5\n"

			"pand		%%xmm7, %%xmm4\n"
			"pand		%%xmm7, %%xmm5\n"

			"psubb		_ZL26SIMD_SSE2_byte_minus_128_0, %%xmm4\n"
			"psubb		_ZL26SIMD_SSE2_byte_minus_128_0, %%xmm5\n"

			"movd		%%xmm4, (%[minColor])\n"
			"movd		%%xmm5, (%[maxColor])\n"

			"movdqa		 0(%[colorBlock]), %%xmm0\n"
			"movdqa		16(%[colorBlock]), %%xmm1\n"
			"movdqa		32(%[colorBlock]), %%xmm2\n"
			"movdqa		48(%[colorBlock]), %%xmm3\n"

			"paddb		_ZL26SIMD_SSE2_byte_minus_128_0, %%xmm0\n"
			"paddb		_ZL26SIMD_SSE2_byte_minus_128_0, %%xmm1\n"
			"paddb		_ZL26SIMD_SSE2_byte_minus_128_0, %%xmm2\n"
			"paddb		_ZL26SIMD_SSE2_byte_minus_128_0, %%xmm3\n"

			"pmullw		%%xmm6, %%xmm0\n"
			"pmullw		%%xmm6, %%xmm1\n"
			"pmullw		%%xmm6, %%xmm2\n"
			"pmullw		%%xmm6, %%xmm3\n"

			"pand		%%xmm7, %%xmm0\n"
			"pand		%%xmm7, %%xmm1\n"
			"pand		%%xmm7, %%xmm2\n"
			"pand		%%xmm7, %%xmm3\n"

			"psubb		_ZL26SIMD_SSE2_byte_minus_128_0, %%xmm0\n"
			"psubb		_ZL26SIMD_SSE2_byte_minus_128_0, %%xmm1\n"
			"psubb		_ZL26SIMD_SSE2_byte_minus_128_0, %%xmm2\n"
			"psubb		_ZL26SIMD_SSE2_byte_minus_128_0, %%xmm3\n"

			"movdqa		%%xmm0,  0(%[colorBlock])\n"
			"movdqa		%%xmm1, 16(%[colorBlock])\n"
			"movdqa		%%xmm2, 32(%[colorBlock])\n"
			"movdqa		%%xmm3, 48(%[colorBlock])\n"
			:: [colorBlock] "r" (colorBlock), [minColor] "r" (minColor), [maxColor] "r" (maxColor),
			[shuf0000] "i" R_SHUFFLE_D( 0, 0, 0, 0 ), [shuf1010] "i" R_SHUFFLE_D( 1, 0, 1, 0 )
			: "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "memory");
#elif defined ( ID_WIN_X86_SSE2_INTRIN ) || defined ( ID_QNX_X86_SSE2_INTRIN )
	__m128i block0 = *((__m128i *)(&colorBlock[ 0]));
	__m128i block1 = *((__m128i *)(&colorBlock[16]));
	__m128i block2 = *((__m128i *)(&colorBlock[32]));
	__m128i block3 = *((__m128i *)(&colorBlock[48]));

	__m128c temp0, temp1, temp2, temp3, temp4, temp5, temp6, temp7;
	temp0 = _mm_cvtsi32_si128( *(int *)minColor );
	temp1 = _mm_cvtsi32_si128( *(int *)maxColor );

	temp0 = _mm_unpacklo_epi8( temp0, (const __m128i &)SIMD_SSE2_byte_0 );
	temp1 = _mm_unpacklo_epi8( temp1, (const __m128i &)SIMD_SSE2_byte_0 );

	// TODO: Algorithm seems to be get the absolute difference
	temp6 = _mm_sub_epi16( (const __m128i &)SIMD_SSE2_word_center_128, temp0 );
	temp7 = _mm_sub_epi16( (const __m128i &)SIMD_SSE2_word_center_128, temp1 );
	temp0 = _mm_sub_epi16( temp0, (const __m128i &)SIMD_SSE2_word_center_128 );
	temp1 = _mm_sub_epi16( temp1, (const __m128i &)SIMD_SSE2_word_center_128 );
	temp6 = _mm_max_epi16( temp6, temp0 );
	temp7 = _mm_max_epi16( temp7, temp1 );

	temp6 = _mm_max_epi16( temp6, temp7 );
	temp7 = _mm_shufflelo_epi16( temp6, R_SHUFFLE_D( 1, 0, 1, 0 ) );
	temp6 = _mm_max_epi16( temp6, temp7 );
	temp6 = _mm_shuffle_epi32( temp6, R_SHUFFLE_D( 0, 0, 0, 0 ) );

	temp7 = temp6;
	temp6 = _mm_cmpgt_epi16( temp6, (const __m128i &)SIMD_SSE2_word_63 );			// mask0
	temp7 = _mm_cmpgt_epi16( temp7, (const __m128i &)SIMD_SSE2_word_31 );			// mask1

	temp7 = _mm_andnot_si128( temp7, (const __m128i &)SIMD_SSE2_byte_2 );
	temp7 = _mm_or_si128( temp7, (const __m128i &)SIMD_SSE2_byte_1 );
	temp6 = _mm_andnot_si128( temp6, temp7 );
	temp3 = temp6;
	temp7 = temp6;
	temp7 = _mm_xor_si128( temp7, (const __m128i &)SIMD_SSE2_byte_not );
	temp7 = _mm_or_si128( temp7, (const __m128i &)SIMD_SSE2_byte_scale_mask0 );		// 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00
	temp6 = _mm_add_epi16( temp6, (const __m128i &)SIMD_SSE2_byte_1 );
	temp6 = _mm_and_si128( temp6, (const __m128i &)SIMD_SSE2_byte_scale_mask1 );	// 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF
	temp6 = _mm_or_si128( temp6, (const __m128i &)SIMD_SSE2_byte_scale_mask2 );		// 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00

	// TODO: remove this second store
	temp4 = _mm_cvtsi32_si128( *(int *)minColor );
	temp5 = _mm_cvtsi32_si128( *(int *)maxColor );

	temp4 = _mm_and_si128( temp4, (const __m128i &)SIMD_SSE2_byte_scale_mask3 );	// 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0xFF, 0xFF
	temp5 = _mm_and_si128( temp5, (const __m128i &)SIMD_SSE2_byte_scale_mask3 );

	temp3 = _mm_slli_epi32( temp3, 3 );
	temp3 = _mm_and_si128( temp3, (const __m128i &)SIMD_SSE2_byte_scale_mask4 );	// 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00

	temp4 = _mm_or_si128( temp4, temp3 );
	temp5 = _mm_or_si128( temp5, temp3 );

	temp4 = _mm_add_epi8( temp4, (const __m128i &)SIMD_SSE2_byte_minus_128_0 );
	temp5 = _mm_add_epi8( temp5, (const __m128i &)SIMD_SSE2_byte_minus_128_0 );

	temp4 = _mm_mullo_epi16( temp4, temp6 );
	temp5 = _mm_mullo_epi16( temp5, temp6 );

	temp4 = _mm_and_si128( temp4, temp7 );
	temp5 = _mm_and_si128( temp5, temp7 );

	temp4 = _mm_sub_epi8( temp4, (const __m128i &)SIMD_SSE2_byte_minus_128_0 );
	temp5 = _mm_sub_epi8( temp5, (const __m128i &)SIMD_SSE2_byte_minus_128_0 );

	*(int *)minColor = _mm_cvtsi128_si32( temp4 );
	*(int *)maxColor = _mm_cvtsi128_si32( temp5 );

	temp0 = _mm_add_epi8( block0, (const __m128i &)SIMD_SSE2_byte_minus_128_0 );
	temp1 = _mm_add_epi8( block1, (const __m128i &)SIMD_SSE2_byte_minus_128_0 );
	temp2 = _mm_add_epi8( block2, (const __m128i &)SIMD_SSE2_byte_minus_128_0 );
	temp3 = _mm_add_epi8( block3, (const __m128i &)SIMD_SSE2_byte_minus_128_0 );

	temp0 = _mm_mullo_epi16( temp0, temp6 );
	temp1 = _mm_mullo_epi16( temp1, temp6 );
	temp2 = _mm_mullo_epi16( temp2, temp6 );
	temp3 = _mm_mullo_epi16( temp3, temp6 );

	temp0 = _mm_and_si128( temp0, temp7 );
	temp1 = _mm_and_si128( temp1, temp7 );
	temp2 = _mm_and_si128( temp2, temp7 );
	temp3 = _mm_and_si128( temp3, temp7 );

	*((__m128i *)(&colorBlock[ 0])) = _mm_sub_epi8( temp0, (const __m128i &)SIMD_SSE2_byte_minus_128_0 );
	*((__m128i *)(&colorBlock[16])) = _mm_sub_epi8( temp1, (const __m128i &)SIMD_SSE2_byte_minus_128_0 );
	*((__m128i *)(&colorBlock[32])) = _mm_sub_epi8( temp2, (const __m128i &)SIMD_SSE2_byte_minus_128_0 );
	*((__m128i *)(&colorBlock[48])) = _mm_sub_epi8( temp3, (const __m128i &)SIMD_SSE2_byte_minus_128_0 );
#else
	assert( false );
#endif
}

/*
========================
idDxtEncoder::InsetYCoCgBBox_SSE2
========================
*/
ID_INLINE void idDxtEncoder::InsetYCoCgBBox_SSE2( byte *minColor, byte *maxColor ) const {
#if ( defined( ID_WIN_X86_ASM ) || defined( ID_MAC_X86_ASM ) )
	__asm {
		mov			esi, minColor
		mov			edi, maxColor
		movd		xmm0, dword ptr [esi]
		movd		xmm1, dword ptr [edi]
		punpcklbw	xmm0, SIMD_SSE2_byte_0
		punpcklbw	xmm1, SIMD_SSE2_byte_0
		movdqa		xmm2, xmm1
		psubw		xmm2, xmm0
		psubw		xmm2, SIMD_SSE2_word_insetYCoCgRound
		pand		xmm2, SIMD_SSE2_word_insetYCoCgMask
		pmullw		xmm0, SIMD_SSE2_word_insetYCoCgShiftUp
		pmullw		xmm1, SIMD_SSE2_word_insetYCoCgShiftUp
		paddw		xmm0, xmm2
		psubw		xmm1, xmm2
		pmulhw		xmm0, SIMD_SSE2_word_insetYCoCgShiftDown
		pmulhw		xmm1, SIMD_SSE2_word_insetYCoCgShiftDown
		pmaxsw		xmm0, SIMD_SSE2_word_0
		pmaxsw		xmm1, SIMD_SSE2_word_0
		pand		xmm0, SIMD_SSE2_word_insetYCoCgQuantMask
		pand		xmm1, SIMD_SSE2_word_insetYCoCgQuantMask
		movdqa		xmm2, xmm0
		movdqa		xmm3, xmm1
		pmulhw		xmm2, SIMD_SSE2_word_insetYCoCgRep
		pmulhw		xmm3, SIMD_SSE2_word_insetYCoCgRep
		por			xmm0, xmm2
		por			xmm1, xmm3
		packuswb	xmm0, xmm0
		packuswb	xmm1, xmm1
		movd		dword ptr [esi], xmm0
		movd		dword ptr [edi], xmm1
	}
#elif defined ( ID_QNX_X86_SSE2_ASM )
	__asm__ __volatile__(
			"movd		(%[min]), %%xmm0\n"
			"movd		(%[max]), %%xmm1\n"
			"punpcklbw	_ZL16SIMD_SSE2_byte_0, %%xmm0\n"
			"punpcklbw	_ZL16SIMD_SSE2_byte_0, %%xmm1\n"
			"movdqa		%%xmm1, %%xmm2\n"
			"psubw		%%xmm0, %%xmm2\n"
			"psubw		_ZL30SIMD_SSE2_word_insetYCoCgRound, %%xmm2\n"
			"pand		_ZL29SIMD_SSE2_word_insetYCoCgMask, %%xmm2\n"
			"pmullw		_ZL32SIMD_SSE2_word_insetYCoCgShiftUp, %%xmm0\n"
			"pmullw		_ZL32SIMD_SSE2_word_insetYCoCgShiftUp, %%xmm1\n"
			"paddw		%%xmm2, %%xmm0\n"
			"psubw		%%xmm2, %%xmm1\n"
			"pmulhw		_ZL34SIMD_SSE2_word_insetYCoCgShiftDown, %%xmm0\n"
			"pmulhw		_ZL34SIMD_SSE2_word_insetYCoCgShiftDown, %%xmm1\n"
			"pmaxsw		_ZL16SIMD_SSE2_word_0, %%xmm0\n"
			"pmaxsw		_ZL16SIMD_SSE2_word_0, %%xmm1\n"
			"pand		_ZL34SIMD_SSE2_word_insetYCoCgQuantMask, %%xmm0\n"
			"pand		_ZL34SIMD_SSE2_word_insetYCoCgQuantMask, %%xmm1\n"
			"movdqa		%%xmm0, %%xmm2\n"
			"movdqa		%%xmm1, %%xmm3\n"
			"pmulhw		_ZL28SIMD_SSE2_word_insetYCoCgRep, %%xmm2\n"
			"pmulhw		_ZL28SIMD_SSE2_word_insetYCoCgRep, %%xmm3\n"
			"por		%%xmm2, %%xmm0\n"
			"por		%%xmm3, %%xmm1\n"
			"packuswb	%%xmm0, %%xmm0\n"
			"packuswb	%%xmm1, %%xmm1\n"
			"movd		%%xmm0, (%[min])\n"
			"movd		%%xmm1, (%[max])\n"
			:: [min] "r" (minColor), [max] "r" (maxColor) : "xmm0", "xmm1", "xmm2", "xmm3", "memory");
#elif defined ( ID_WIN_X86_SSE2_INTRIN ) || defined ( ID_QNX_X86_SSE2_INTRIN )
	__m128c temp0, temp1, temp2, temp3, temp4, temp5, temp6, temp7;

	temp0 = _mm_cvtsi32_si128( *(int *)minColor );
	temp1 = _mm_cvtsi32_si128( *(int *)maxColor );

	temp0 = _mm_unpacklo_epi8( temp0, (const __m128i &)SIMD_SSE2_byte_0 );
	temp1 = _mm_unpacklo_epi8( temp1, (const __m128i &)SIMD_SSE2_byte_0 );

	temp2 = _mm_sub_epi16( temp1, temp0 );
	temp2 = _mm_sub_epi16( temp2, (const __m128i &)SIMD_SSE2_word_insetYCoCgRound );
	temp2 = _mm_and_si128( temp2, (const __m128i &)SIMD_SSE2_word_insetYCoCgMask );
	temp0 = _mm_mullo_epi16( temp0, (const __m128i &)SIMD_SSE2_word_insetYCoCgShiftUp );
	temp1 = _mm_mullo_epi16( temp1, (const __m128i &)SIMD_SSE2_word_insetYCoCgShiftUp );
	temp0 = _mm_add_epi16( temp0, temp2 );
	temp1 = _mm_sub_epi16( temp1, temp2 );
	temp0 = _mm_mulhi_epi16( temp0, (const __m128i &)SIMD_SSE2_word_insetYCoCgShiftDown );
	temp1 = _mm_mulhi_epi16( temp1, (const __m128i &)SIMD_SSE2_word_insetYCoCgShiftDown );
	temp0 = _mm_max_epi16( temp0, (const __m128i &)SIMD_SSE2_word_0 );
	temp1 = _mm_max_epi16( temp1, (const __m128i &)SIMD_SSE2_word_0 );
	temp0 = _mm_and_si128( temp0, (const __m128i &)SIMD_SSE2_word_insetYCoCgQuantMask );
	temp1 = _mm_and_si128( temp1, (const __m128i &)SIMD_SSE2_word_insetYCoCgQuantMask );
	temp2 = _mm_mulhi_epi16( temp0, (const __m128i &)SIMD_SSE2_word_insetYCoCgRep );
	temp3 = _mm_mulhi_epi16( temp1, (const __m128i &)SIMD_SSE2_word_insetYCoCgRep );
	temp0 = _mm_or_si128( temp0, temp2 );
	temp1 = _mm_or_si128( temp1, temp3 );
	temp0 = _mm_packus_epi16( temp0, temp0 );
	temp1 = _mm_packus_epi16( temp1, temp1 );

	*(int *)minColor = _mm_cvtsi128_si32( temp0 );
	*(int *)maxColor = _mm_cvtsi128_si32( temp1 );
#else
	assert( false );
#endif
}

/*
========================
idDxtEncoder::SelectYCoCgDiagonal_SSE2

params:	colorBlock	- 16 pixel block to find color indexes for
paramO:	minColor	- min color found
paramO:	maxColor	- max color found
return: diagonal to use
========================
*/
ID_INLINE void idDxtEncoder::SelectYCoCgDiagonal_SSE2( const byte *colorBlock, byte *minColor, byte *maxColor ) const {
#if ( defined( ID_WIN_X86_ASM ) || defined( ID_MAC_X86_ASM ) )
	__asm {
		mov			esi, colorBlock
		mov			edx, minColor
		mov			ecx, maxColor

		movdqa		xmm0, xmmword ptr [esi+	0]
		movdqa		xmm1, xmmword ptr [esi+16]
		movdqa		xmm2, xmmword ptr [esi+32]
		movdqa		xmm3, xmmword ptr [esi+48]

		pand		xmm0, SIMD_SSE2_dword_word_mask
		pand		xmm1, SIMD_SSE2_dword_word_mask
		pand		xmm2, SIMD_SSE2_dword_word_mask
		pand		xmm3, SIMD_SSE2_dword_word_mask

		pslldq		xmm1, 2
		pslldq		xmm3, 2
		por			xmm0, xmm1
		por			xmm2, xmm3

		movd		xmm1, dword ptr [edx]					// minColor
		movd		xmm3, dword ptr [ecx]					// maxColor

		movdqa		xmm6, xmm1
		movdqa		xmm7, xmm3

		pavgb		xmm1, xmm3
		pshuflw		xmm1, xmm1, R_SHUFFLE_D( 0, 0, 0, 0 )
		pshufd		xmm1, xmm1, R_SHUFFLE_D( 0, 0, 0, 0 )
		movdqa		xmm3, xmm1

		pmaxub		xmm1, xmm0
		pmaxub		xmm3, xmm2
		pcmpeqb		xmm1, xmm0
		pcmpeqb		xmm3, xmm2

		movdqa		xmm0, xmm1
		movdqa		xmm2, xmm3
		psrldq		xmm0, 1
		psrldq		xmm2, 1

		pxor		xmm0, xmm1
		pxor		xmm2, xmm3
		pand		xmm0, SIMD_SSE2_word_1
		pand		xmm2, SIMD_SSE2_word_1

		paddw		xmm0, xmm2
		psadbw		xmm0, SIMD_SSE2_byte_0
		pshufd		xmm1, xmm0, R_SHUFFLE_D( 2, 3, 0, 1 )

#ifdef NVIDIA_7X_HARDWARE_BUG_FIX
		paddw		xmm1, xmm0								// side
		pcmpgtw		xmm1, SIMD_SSE2_word_8					// mask = -( side > 8 )
		pand		xmm1, SIMD_SSE2_byte_diagonalMask
		movdqa		xmm0, xmm6
		pcmpeqb		xmm0, xmm7								// mask &= -( minColor[0] != maxColor[0] )
		pslldq		xmm0, 1
		pandn		xmm0, xmm1
#else
		paddw		xmm0, xmm1								// side
		pcmpgtw		xmm0, SIMD_SSE2_word_8					// mask = -( side > 8 )
		pand		xmm0, SIMD_SSE2_byte_diagonalMask
#endif

		pxor		xmm6, xmm7
		pand		xmm0, xmm6
		pxor		xmm7, xmm0
		pxor		xmm6, xmm7

		movd		dword ptr [edx], xmm6
		movd		dword ptr [ecx], xmm7
	}
#elif defined ( ID_QNX_X86_SSE2_ASM )
	__asm__ __volatile__(
			"movdqa		 0(%[colorBlock]), %%xmm0\n"
			"movdqa		16(%[colorBlock]), %%xmm1\n"
			"movdqa		32(%[colorBlock]), %%xmm2\n"
			"movdqa		48(%[colorBlock]), %%xmm3\n"

			"pand		_ZL25SIMD_SSE2_dword_word_mask, %%xmm0\n"
			"pand		_ZL25SIMD_SSE2_dword_word_mask, %%xmm1\n"
			"pand		_ZL25SIMD_SSE2_dword_word_mask, %%xmm2\n"
			"pand		_ZL25SIMD_SSE2_dword_word_mask, %%xmm3\n"

			"pslldq		$2, %%xmm1\n"
			"pslldq		$2, %%xmm3\n"
			"por		%%xmm1, %%xmm0\n"
			"por		%%xmm3, %%xmm2\n"

			"movd		(%[minColor]), %%xmm1\n"						// minColor
			"movd		(%[maxColor]), %%xmm3\n"						// maxColor

			"movdqa		%%xmm1, %%xmm6\n"
			"movdqa		%%xmm3, %%xmm7\n"

			"pavgb		%%xmm3, %%xmm1\n"
			"pshuflw	%[shuf0000], %%xmm1, %%xmm1\n"
			"pshufd		%[shuf0000], %%xmm1, %%xmm1\n"
			"movdqa		%%xmm1, %%xmm3\n"

			"pmaxub		%%xmm0, %%xmm1\n"
			"pmaxub		%%xmm2, %%xmm3\n"
			"pcmpeqb	%%xmm0, %%xmm1\n"
			"pcmpeqb	%%xmm2, %%xmm3\n"

			"movdqa		%%xmm1, %%xmm0\n"
			"movdqa		%%xmm3, %%xmm2\n"
			"psrldq		$1, %%xmm0\n"
			"psrldq		$1, %%xmm2\n"

			"pxor		%%xmm1, %%xmm0\n"
			"pxor		%%xmm3, %%xmm2\n"
			"pand		_ZL16SIMD_SSE2_word_1, %%xmm0\n"
			"pand		_ZL16SIMD_SSE2_word_1, %%xmm2\n"

			"paddw		%%xmm2, %%xmm0\n"
			"psadbw		_ZL16SIMD_SSE2_byte_0, %%xmm0\n"
			"pshufd		%[shuf2301], %%xmm0, %%xmm1\n"

#ifdef NVIDIA_7X_HARDWARE_BUG_FIX
			"paddw		%%xmm0, %%xmm1\n"								// side
			"pcmpgtw	_ZL16SIMD_SSE2_word_8, %%xmm1\n"				// mask = -( side > 8 )
			"pand		_ZL27SIMD_SSE2_byte_diagonalMask, %%xmm1\n"
			"movdqa		%%xmm6, %%xmm0\n"
			"pcmpeqb	%%xmm7, %%xmm0\n"								// mask &= -( minColor[0] != maxColor[0] )
			"pslldq		$1, %%xmm0\n"
			"pandn		%%xmm1, %%xmm0\n"
#else
			"paddw		%%xmm1, %%xmm0\n"								// side
			"pcmpgtw	_ZL16SIMD_SSE2_word_8, %%xmm0\n"				// mask = -( side > 8 )
			"pand		_ZL27SIMD_SSE2_byte_diagonalMask, %%xmm0\n"
#endif

			"pxor		%%xmm7, %%xmm6\n"
			"pand		%%xmm6, %%xmm0\n"
			"pxor		%%xmm0, %%xmm7\n"
			"pxor		%%xmm7, %%xmm6\n"

			"movd		%%xmm6, (%[minColor])\n"
			"movd		%%xmm7, (%[maxColor])\n"
			:: [colorBlock] "r" (colorBlock), [minColor] "r" (minColor), [maxColor] "r" (maxColor),
			[shuf0000] "i" R_SHUFFLE_D( 0, 0, 0, 0 ), [shuf2301] "i" R_SHUFFLE_D( 2, 3, 0, 1 )
			: "xmm0", "xmm1", "xmm2", "xmm3", "xmm6", "xmm7", "memory");
#elif defined ( ID_WIN_X86_SSE2_INTRIN ) || defined ( ID_QNX_X86_SSE2_INTRIN )
	__m128i block0 = *((__m128i *)(&colorBlock[ 0]));
	__m128i block1 = *((__m128i *)(&colorBlock[16]));
	__m128i block2 = *((__m128i *)(&colorBlock[32]));
	__m128i block3 = *((__m128i *)(&colorBlock[48]));
	__m128c temp0, temp1, temp2, temp3, temp4, temp5, temp6, temp7;

	temp0 = _mm_and_si128( block0, (const __m128i &)SIMD_SSE2_dword_word_mask );
	temp1 = _mm_and_si128( block1, (const __m128i &)SIMD_SSE2_dword_word_mask );
	temp2 = _mm_and_si128( block2, (const __m128i &)SIMD_SSE2_dword_word_mask );
	temp3 = _mm_and_si128( block3, (const __m128i &)SIMD_SSE2_dword_word_mask );

	temp1 = _mm_slli_si128( temp1, 2 );
	temp3 = _mm_slli_si128( temp3, 2 );
	temp0 = _mm_or_si128( temp0, temp1 );
	temp2 = _mm_or_si128( temp2, temp3 );

	temp6 = _mm_cvtsi32_si128( *(int *)minColor );
	temp7 = _mm_cvtsi32_si128( *(int *)maxColor );

	temp1 = _mm_avg_epu8( temp6, temp7 );
	temp1 = _mm_shufflelo_epi16( temp1, R_SHUFFLE_D( 0, 0, 0, 0 ) );
	temp1 = _mm_shuffle_epi32( temp1, R_SHUFFLE_D( 0, 0, 0, 0 ) );

	temp3 = _mm_max_epu8( temp1, temp2 );
	temp1 = _mm_max_epu8( temp1, temp0 );
	temp1 = _mm_cmpeq_epi8( temp1, temp0 );
	temp3 = _mm_cmpeq_epi8( temp3, temp2 );

	temp0 = _mm_srli_si128( temp1, 1 );
	temp2 = _mm_srli_si128( temp3, 1 );

	temp0 = _mm_xor_si128( temp0, temp1 );
	temp2 = _mm_xor_si128( temp2, temp3 );
	temp0 = _mm_and_si128( temp0, (const __m128i &)SIMD_SSE2_word_1 );
	temp2 = _mm_and_si128( temp2, (const __m128i &)SIMD_SSE2_word_1 );

	temp0 = _mm_add_epi16( temp0, temp2 );
	temp0 = _mm_sad_epu8( temp0, (const __m128i &)SIMD_SSE2_byte_0 );
	temp1 = _mm_shuffle_epi32( temp0, R_SHUFFLE_D( 2, 3, 0, 1 ) );

#ifdef NVIDIA_7X_HARDWARE_BUG_FIX
	temp1 = _mm_add_epi16( temp1, temp0 );
	temp1 = _mm_cmpgt_epi16( temp1, (const __m128i &)SIMD_SSE2_word_8 );
	temp1 = _mm_and_si128( temp1, (const __m128i &)SIMD_SSE2_byte_diagonalMask );
	temp0 = _mm_cmpeq_epi8( temp6, temp7 );
	temp0 = _mm_slli_si128( temp0, 1 );
	temp0 = _mm_andnot_si128( temp0, temp1 );
#else
	temp0 = _mm_add_epi16( temp0, temp1 );
	temp0 = _mm_cmpgt_epi16( temp0, (const __m128i &)SIMD_SSE2_word_8 );
	temp0 = _mm_and_si128( temp0, (const __m128i &)SIMD_SSE2_byte_diagonalMask );
#endif

	temp6 = _mm_xor_si128( temp6, temp7 );
	temp0 = _mm_and_si128( temp0, temp6 );
	temp7 = _mm_xor_si128( temp7, temp0 );
	temp6 = _mm_xor_si128( temp6, temp7 );

	*(int *)minColor = _mm_cvtsi128_si32( temp6 );
	*(int *)maxColor = _mm_cvtsi128_si32( temp7 );
#else
	assert( false );
#endif
}

/*
========================
idDxtEncoder::CompressYCoCgDXT5Fast_SSE2

params:	inBuf		- image to compress
paramO:	outBuf		- result of compression
params:	width		- width of image
params:	height		- height of image
========================
*/
void idDxtEncoder::CompressYCoCgDXT5Fast_SSE2( const byte *inBuf, byte *outBuf, int width, int height ) {
	ALIGN16( byte block[64] );
	ALIGN16( byte minColor[4] );
	ALIGN16( byte maxColor[4] );

	//assert( HasConstantValuePer4x4Block( inBuf, width, height, 2 ) );
	assert( width >= 4 && ( width & 3 ) == 0 );
	assert( height >= 4 && ( height & 3 ) == 0 );

	this->width = width;
	this->height = height;
	this->outData = outBuf;

	for ( int j = 0; j < height; j += 4, inBuf += width * 4*4 ) {
		for ( int i = 0; i < width; i += 4 ) {
			ExtractBlock_SSE2( inBuf + i * 4, width, block );
			GetMinMaxBBox_SSE2( block, minColor, maxColor );

			ScaleYCoCg_SSE2( block, minColor, maxColor );
			InsetYCoCgBBox_SSE2( minColor, maxColor );
			SelectYCoCgDiagonal_SSE2( block, minColor, maxColor );

			EmitByte( maxColor[3] );
			EmitByte( minColor[3] );

			EmitAlphaIndices_SSE2( block, minColor[3], maxColor[3] );

			EmitUShort( ColorTo565( maxColor ) );
			EmitUShort( ColorTo565( minColor ) );

			EmitCoCgIndices_SSE2( block, minColor, maxColor );
		}
		outData += dstPadding;
		inBuf += srcPadding;
	}

#ifdef TEST_COMPRESSION
	int tmpDstPadding = dstPadding;
	dstPadding = 0;
	byte * testOutBuf = (byte *) _alloca16( width * height );
	CompressYCoCgDXT5Fast_Generic( inBuf, testOutBuf, width, height );
	for ( int j = 0; j < height / 4; j++ ) {
		for ( int i = 0; i < width / 4; i++ ) {
			byte * ptr1 = outBuf + ( j * width/4 + i ) * 16 + j * tmpDstPadding;
			byte * ptr2 = testOutBuf + ( j * width/4 + i ) * 16;
			for ( int k = 0; k < 16; k++ ) {
				assert( ptr1[k] == ptr2[k] );
			}
		}
	}
	dstPadding = tmpDstPadding;
#endif
}

/*
========================
idDxtEncoder::EmitGreenIndices_SSE2

params:	block		- 16-normal block for which to find normal Y indices
paramO:	minGreen	- Minimal normal Y found
paramO:	maxGreen	- Maximal normal Y found
========================
*/
void idDxtEncoder::EmitGreenIndices_SSE2( const byte *block, const int channelBitOffset, const int minGreen, const int maxGreen ) {
#if ( defined( ID_WIN_X86_ASM ) || defined( ID_MAC_X86_ASM ) ) || defined ( ID_QNX_X86_SSE2_ASM )
	assert( maxGreen >= minGreen );

	byte *outPtr = outData;

#if ( defined( ID_WIN_X86_ASM ) || defined( ID_MAC_X86_ASM ) )
	__asm {
		movd		xmm7, channelBitOffset

		mov			esi, block
		movdqa		xmm0, xmmword ptr [esi+	0]
		movdqa		xmm5, xmmword ptr [esi+16]
		movdqa		xmm6, xmmword ptr [esi+32]
		movdqa		xmm4, xmmword ptr [esi+48]

		psrld		xmm0, xmm7
		psrld		xmm5, xmm7
		psrld		xmm6, xmm7
		psrld		xmm4, xmm7

		pand		xmm0, SIMD_SSE2_dword_byte_mask
		pand		xmm5, SIMD_SSE2_dword_byte_mask
		pand		xmm6, SIMD_SSE2_dword_byte_mask
		pand		xmm4, SIMD_SSE2_dword_byte_mask

		packuswb	xmm0, xmm5
		packuswb	xmm6, xmm4

		//---------------------

		movd		xmm2, maxGreen
		pshuflw		xmm2, xmm2, R_SHUFFLE_D( 0, 0, 0, 0 )

		movd		xmm3, minGreen
		pshuflw		xmm3, xmm3, R_SHUFFLE_D( 0, 0, 0, 0 )

		pmullw		xmm2, SIMD_SSE2_word_scale_5_3_1
		pmullw		xmm3, SIMD_SSE2_word_scale_1_3_5
		paddw		xmm2, SIMD_SSE2_word_3
		paddw		xmm3, xmm2
		pmulhw		xmm3, SIMD_SSE2_word_div_by_6

		pshuflw		xmm1, xmm3, R_SHUFFLE_D( 0, 0, 0, 0 )
		pshuflw		xmm2, xmm3, R_SHUFFLE_D( 1, 1, 1, 1 )
		pshuflw		xmm3, xmm3, R_SHUFFLE_D( 2, 2, 2, 2 )

		pshufd		xmm1, xmm1, R_SHUFFLE_D( 0, 0, 0, 0 )
		pshufd		xmm2, xmm2, R_SHUFFLE_D( 0, 0, 0, 0 )
		pshufd		xmm3, xmm3, R_SHUFFLE_D( 0, 0, 0, 0 )

		packuswb	xmm1, xmm1
		packuswb	xmm2, xmm2
		packuswb	xmm3, xmm3

		packuswb	xmm0, xmm6

		pmaxub		xmm1, xmm0
		pmaxub		xmm2, xmm0
		pmaxub		xmm3, xmm0
		pcmpeqb		xmm1, xmm0
		pcmpeqb		xmm2, xmm0
		pcmpeqb		xmm3, xmm0
		movdqa		xmm0, SIMD_SSE2_byte_4
		paddsb		xmm0, xmm1
		paddsb		xmm2, xmm3
		paddsb		xmm0, xmm2
		pand		xmm0, SIMD_SSE2_byte_3
		movdqa		xmm4, SIMD_SSE2_byte_2
		pcmpgtb		xmm4, xmm0
		pand		xmm4, SIMD_SSE2_byte_1
		pxor		xmm0, xmm4
		movdqa		xmm4, xmm0
		movdqa		xmm5, xmm0
		movdqa		xmm6, xmm0
		movdqa		xmm7, xmm0
		psrlq		xmm4,  8- 2
		psrlq		xmm5, 16- 4
		psrlq		xmm6, 24- 6
		psrlq		xmm7, 32- 8
		pand		xmm4, SIMD_SSE2_dword_color_bit_mask1
		pand		xmm5, SIMD_SSE2_dword_color_bit_mask2
		pand		xmm6, SIMD_SSE2_dword_color_bit_mask3
		pand		xmm7, SIMD_SSE2_dword_color_bit_mask4
		por			xmm5, xmm4
		por			xmm7, xmm6
		por			xmm7, xmm5
		movdqa		xmm4, xmm0
		movdqa		xmm5, xmm0
		movdqa		xmm6, xmm0
		psrlq		xmm4, 40-10
		psrlq		xmm5, 48-12
		psrlq		xmm6, 56-14
		pand		xmm0, SIMD_SSE2_dword_color_bit_mask0
		pand		xmm4, SIMD_SSE2_dword_color_bit_mask5
		pand		xmm5, SIMD_SSE2_dword_color_bit_mask6
		pand		xmm6, SIMD_SSE2_dword_color_bit_mask7
		por			xmm4, xmm5
		por			xmm0, xmm6
		por			xmm7, xmm4
		por			xmm7, xmm0
		mov			esi, outPtr
		pshufd		xmm7, xmm7, R_SHUFFLE_D( 0, 2, 1, 3 )
		pshuflw		xmm7, xmm7, R_SHUFFLE_D( 0, 2, 1, 3 )
		movd		[esi], xmm7
	}
#else
	__asm__ __volatile__(
			"movd		%[channelBitOffset], %%xmm7\n"

			"movdqa		0(%[block]), %%xmm0\n"
			"movdqa		16(%[block]), %%xmm5\n"
			"movdqa		32(%[block]), %%xmm6\n"
			"movdqa		48(%[block]), %%xmm4\n"

			"psrld		%%xmm7, %%xmm0\n"
			"psrld		%%xmm7, %%xmm5\n"
			"psrld		%%xmm7, %%xmm6\n"
			"psrld		%%xmm7, %%xmm4\n"

			"pand		_ZL25SIMD_SSE2_dword_byte_mask, %%xmm0\n"
			"pand		_ZL25SIMD_SSE2_dword_byte_mask, %%xmm5\n"
			"pand		_ZL25SIMD_SSE2_dword_byte_mask, %%xmm6\n"
			"pand		_ZL25SIMD_SSE2_dword_byte_mask, %%xmm4\n"

			"packuswb	%%xmm5, %%xmm0\n"
			"packuswb	%%xmm4, %%xmm6\n"

			//---------------------

			"movd		%[maxGreen], %%xmm2\n"
			"pshuflw	%[shuf0000], %%xmm2, %%xmm2\n"

			"movd		%[minGreen], %%xmm3\n"
			"pshuflw	%[shuf0000], %%xmm3, %%xmm3\n"

			"pmullw		_ZL26SIMD_SSE2_word_scale_5_3_1, %%xmm2\n"
			"pmullw		_ZL26SIMD_SSE2_word_scale_1_3_5, %%xmm3\n"
			"paddw		_ZL16SIMD_SSE2_word_3, %%xmm2\n"
			"paddw		%%xmm2, %%xmm3\n"
			"pmulhw		_ZL23SIMD_SSE2_word_div_by_6, %%xmm3\n"

			"pshuflw	%[shuf0000], %%xmm3, %%xmm1\n"
			"pshuflw	%[shuf1111], %%xmm3, %%xmm2\n"
			"pshuflw	%[shuf2222], %%xmm3, %%xmm3\n"

			"pshufd		%[shuf0000], %%xmm1, %%xmm1\n"
			"pshufd		%[shuf0000], %%xmm2, %%xmm2\n"
			"pshufd		%[shuf0000], %%xmm3, %%xmm3\n"

			"packuswb	%%xmm1, %%xmm1\n"
			"packuswb	%%xmm2, %%xmm2\n"
			"packuswb	%%xmm3, %%xmm3\n"

			"packuswb	%%xmm6, %%xmm0\n"

			"pmaxub		%%xmm0, %%xmm1\n"
			"pmaxub		%%xmm0, %%xmm2\n"
			"pmaxub		%%xmm0, %%xmm3\n"
			"pcmpeqb	%%xmm0, %%xmm1\n"
			"pcmpeqb	%%xmm0, %%xmm2\n"
			"pcmpeqb	%%xmm0, %%xmm3\n"
			"movdqa		_ZL16SIMD_SSE2_byte_4, %%xmm0\n"
			"paddsb		%%xmm1, %%xmm0\n"
			"paddsb		%%xmm3, %%xmm2\n"
			"paddsb		%%xmm2, %%xmm0\n"
			"pand		_ZL16SIMD_SSE2_byte_3, %%xmm0\n"
			"movdqa		_ZL16SIMD_SSE2_byte_2, %%xmm4\n"
			"pcmpgtb	%%xmm0, %%xmm4\n"
			"pand		_ZL16SIMD_SSE2_byte_1, %%xmm4\n"
			"pxor		%%xmm4, %%xmm0\n"
			"movdqa		%%xmm0, %%xmm4\n"
			"movdqa		%%xmm0, %%xmm5\n"
			"movdqa		%%xmm0, %%xmm6\n"
			"movdqa		%%xmm0, %%xmm7\n"
			"psrlq		$( 8-2), %%xmm4\n"
			"psrlq		$(16-4), %%xmm5\n"
			"psrlq		$(24-6), %%xmm6\n"
			"psrlq		$(32-8), %%xmm7\n"
			"pand		_ZL31SIMD_SSE2_dword_color_bit_mask1, %%xmm4\n"
			"pand		_ZL31SIMD_SSE2_dword_color_bit_mask2, %%xmm5\n"
			"pand		_ZL31SIMD_SSE2_dword_color_bit_mask3, %%xmm6\n"
			"pand		_ZL31SIMD_SSE2_dword_color_bit_mask4, %%xmm7\n"
			"por		%%xmm4, %%xmm5\n"
			"por		%%xmm6, %%xmm7\n"
			"por		%%xmm5, %%xmm7\n"
			"movdqa		%%xmm0, %%xmm4\n"
			"movdqa		%%xmm0, %%xmm5\n"
			"movdqa		%%xmm0, %%xmm6\n"
			"psrlq		$(40-10), %%xmm4\n"
			"psrlq		$(48-12), %%xmm5\n"
			"psrlq		$(56-14), %%xmm6\n"
			"pand		_ZL31SIMD_SSE2_dword_color_bit_mask0, %%xmm0\n"
			"pand		_ZL31SIMD_SSE2_dword_color_bit_mask5, %%xmm4\n"
			"pand		_ZL31SIMD_SSE2_dword_color_bit_mask6, %%xmm5\n"
			"pand		_ZL31SIMD_SSE2_dword_color_bit_mask7, %%xmm6\n"
			"por		%%xmm5, %%xmm4\n"
			"por		%%xmm6, %%xmm0\n"
			"por		%%xmm4, %%xmm7\n"
			"por		%%xmm0, %%xmm7\n"
			"pshufd		%[shuf0213], %%xmm7, %%xmm7\n"
			"pshuflw	%[shuf0213], %%xmm7, %%xmm7\n"
			"movd		%%xmm7, (%[outPtr])\n"
			:: [outPtr] "r" (outPtr), [block] "r" (block), [channelBitOffset] "r" (channelBitOffset), [minGreen] "r" (minGreen), [maxGreen] "r" (maxGreen),
			[shuf0000] "i" R_SHUFFLE_D( 0, 0, 0, 0 ), [shuf1111] "i" R_SHUFFLE_D( 1, 1, 1, 1 ), [shuf2222] "i" R_SHUFFLE_D( 2, 2, 2, 2 ), [shuf0213] "i" R_SHUFFLE_D( 0, 2, 1, 3 )
			: "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "memory");
#endif

	outData += 4;
#elif defined ( ID_WIN_X86_SSE2_INTRIN ) || defined ( ID_QNX_X86_SSE2_INTRIN )
	__m128i block0 = *((__m128i *)(&block[ 0]));
	__m128i block1 = *((__m128i *)(&block[16]));
	__m128i block2 = *((__m128i *)(&block[32]));
	__m128i block3 = *((__m128i *)(&block[48]));
	__m128c temp0, temp1, temp2, temp3, temp4, temp5, temp6, temp7;

	temp7 = _mm_cvtsi32_si128( channelBitOffset );

	temp0 = _mm_srl_epi32( block0, temp7 );
	temp5 = _mm_srl_epi32( block1, temp7 );
	temp6 = _mm_srl_epi32( block2, temp7 );
	temp4 = _mm_srl_epi32( block3, temp7 );

	temp0 = _mm_and_si128( temp0, (const __m128i &)SIMD_SSE2_dword_byte_mask );
	temp5 = _mm_and_si128( temp5, (const __m128i &)SIMD_SSE2_dword_byte_mask );
	temp6 = _mm_and_si128( temp6, (const __m128i &)SIMD_SSE2_dword_byte_mask );
	temp4 = _mm_and_si128( temp4, (const __m128i &)SIMD_SSE2_dword_byte_mask );

	temp0 = _mm_packus_epi16( temp0, temp5 );
	temp6 = _mm_packus_epi16( temp6, temp4 );

	//---------------------

	temp2 = _mm_cvtsi32_si128( maxGreen );
	temp2 = _mm_shufflelo_epi16( temp2, R_SHUFFLE_D( 0, 0, 0, 0 ) );

	temp3 = _mm_cvtsi32_si128( minGreen );
	temp3 = _mm_shufflelo_epi16( temp3, R_SHUFFLE_D( 0, 0, 0, 0 ) );

	temp2 = _mm_mullo_epi16( temp2, (const __m128i &)SIMD_SSE2_word_scale_5_3_1 );
	temp3 = _mm_mullo_epi16( temp3, (const __m128i &)SIMD_SSE2_word_scale_1_3_5 );
	temp2 = _mm_add_epi16( temp2, (const __m128i &)SIMD_SSE2_word_3 );
	temp3 = _mm_add_epi16( temp3, temp2 );
	temp3 = _mm_mulhi_epi16( temp3, (const __m128i &)SIMD_SSE2_word_div_by_6 );

	temp1 = _mm_shufflelo_epi16( temp3, R_SHUFFLE_D( 0, 0, 0, 0 ) );
	temp2 = _mm_shufflelo_epi16( temp3, R_SHUFFLE_D( 1, 1, 1, 1 ) );
	temp3 = _mm_shufflelo_epi16( temp3, R_SHUFFLE_D( 2, 2, 2, 2 ) );

	temp1 = _mm_shuffle_epi32( temp1, R_SHUFFLE_D( 0, 0, 0, 0 ) );
	temp2 = _mm_shuffle_epi32( temp2, R_SHUFFLE_D( 0, 0, 0, 0 ) );
	temp3 = _mm_shuffle_epi32( temp3, R_SHUFFLE_D( 0, 0, 0, 0 ) );

	temp1 = _mm_packus_epi16( temp1, temp1 );
	temp2 = _mm_packus_epi16( temp2, temp2 );
	temp3 = _mm_packus_epi16( temp3, temp3 );

	temp0 = _mm_packus_epi16( temp0, temp6 );

	temp1 = _mm_max_epu8( temp1, temp0 );
	temp2 = _mm_max_epu8( temp2, temp0 );
	temp3 = _mm_max_epu8( temp3, temp0 );
	temp1 = _mm_cmpeq_epi8( temp1, temp0 );
	temp2 = _mm_cmpeq_epi8( temp2, temp0 );
	temp3 = _mm_cmpeq_epi8( temp3, temp0 );
	temp0 = (const __m128i &)SIMD_SSE2_byte_4;

	temp0 = _mm_adds_epi8( temp0, temp1 );
	temp2 = _mm_adds_epi8( temp2, temp3 );
	temp0 = _mm_adds_epi8( temp0, temp2 );
	temp0 = _mm_and_si128( temp0, (const __m128i &)SIMD_SSE2_byte_3 );
	temp4 = (const __m128i &)SIMD_SSE2_byte_2;
	temp4 = _mm_cmpgt_epi8( temp4, temp0 );
	temp4 = _mm_and_si128( temp4, (const __m128i &)SIMD_SSE2_byte_1 );

	temp0 = _mm_xor_si128( temp0, temp4 );
	temp4 = _mm_srli_epi64( temp0,  8 - 2 );
	temp5 = _mm_srli_epi64( temp0, 16 - 4 );
	temp6 = _mm_srli_epi64( temp0, 24 - 6 );
	temp7 = _mm_srli_epi64( temp0, 32 - 8 );

	temp4 = _mm_and_si128( temp4, (const __m128i &)SIMD_SSE2_dword_color_bit_mask1 );
	temp5 = _mm_and_si128( temp5, (const __m128i &)SIMD_SSE2_dword_color_bit_mask2 );
	temp6 = _mm_and_si128( temp6, (const __m128i &)SIMD_SSE2_dword_color_bit_mask3 );
	temp7 = _mm_and_si128( temp7, (const __m128i &)SIMD_SSE2_dword_color_bit_mask4 );
	temp5 = _mm_or_si128( temp5, temp4 );
	temp7 = _mm_or_si128( temp7, temp6 );
	temp7 = _mm_or_si128( temp7, temp5 );

	temp4 = _mm_srli_epi64( temp0, 40 - 10 );
	temp5 = _mm_srli_epi64( temp0, 48 - 12 );
	temp6 = _mm_srli_epi64( temp0, 56 - 14 );
	temp0 = _mm_and_si128( temp0, (const __m128i &)SIMD_SSE2_dword_color_bit_mask0 );
	temp4 = _mm_and_si128( temp4, (const __m128i &)SIMD_SSE2_dword_color_bit_mask5 );
	temp5 = _mm_and_si128( temp5, (const __m128i &)SIMD_SSE2_dword_color_bit_mask6 );
	temp6 = _mm_and_si128( temp6, (const __m128i &)SIMD_SSE2_dword_color_bit_mask7 );
	temp4 = _mm_or_si128( temp4, temp5 );
	temp0 = _mm_or_si128( temp0, temp6 );
	temp7 = _mm_or_si128( temp7, temp4 );
	temp7 = _mm_or_si128( temp7, temp0 );

	temp7 = _mm_shuffle_epi32( temp7, R_SHUFFLE_D( 0, 2, 1, 3 ) );
	temp7 = _mm_shufflelo_epi16( temp7, R_SHUFFLE_D( 0, 2, 1, 3 ) );

	int result = _mm_cvtsi128_si32( temp7 );
	EmitUInt( result );
#else
	assert( false );
#endif
}

/*
========================
idDxtEncoder::InsetNormalsBBoxDXT5_SSE2
========================
*/
void idDxtEncoder::InsetNormalsBBoxDXT5_SSE2( byte *minNormal, byte *maxNormal ) const {
#if ( defined( ID_WIN_X86_ASM ) || defined( ID_MAC_X86_ASM ) )
	__asm {
        mov         esi, minNormal
        mov         edi, maxNormal
        movd        xmm0, dword ptr [esi]							// xmm0 = minNormal
        movd        xmm1, dword ptr [edi]							// xmm1 = maxNormal
        punpcklbw   xmm0, SIMD_SSE2_byte_0
        punpcklbw   xmm1, SIMD_SSE2_byte_0
        movdqa      xmm2, xmm1
        psubw       xmm2, xmm0
        psubw       xmm2, SIMD_SSE2_word_insetNormalDXT5Round
        pand        xmm2, SIMD_SSE2_word_insetNormalDXT5Mask		// xmm2 = inset (1 & 3)

        pmullw      xmm0, SIMD_SSE2_word_insetNormalDXT5ShiftUp
        pmullw      xmm1, SIMD_SSE2_word_insetNormalDXT5ShiftUp
		paddw		xmm0, xmm2
		psubw		xmm1, xmm2
		pmulhw      xmm0, SIMD_SSE2_word_insetNormalDXT5ShiftDown	// xmm0 = mini
        pmulhw      xmm1, SIMD_SSE2_word_insetNormalDXT5ShiftDown	// xmm1 = maxi

		// mini and maxi must be >= 0 and <= 255
        pmaxsw      xmm0, SIMD_SSE2_word_0
        pmaxsw      xmm1, SIMD_SSE2_word_0
        pminsw      xmm0, SIMD_SSE2_word_255
        pminsw      xmm1, SIMD_SSE2_word_255

        movdqa      xmm2, xmm0
        movdqa      xmm3, xmm1
        pand        xmm0, SIMD_SSE2_word_insetNormalDXT5QuantMask
        pand        xmm1, SIMD_SSE2_word_insetNormalDXT5QuantMask
        pmulhw      xmm2, SIMD_SSE2_word_insetNormalDXT5Rep
        pmulhw      xmm3, SIMD_SSE2_word_insetNormalDXT5Rep
        por         xmm0, xmm2
        por         xmm1, xmm3
        packuswb    xmm0, xmm0
        packuswb    xmm1, xmm1
        movd        dword ptr [esi], xmm0
        movd        dword ptr [edi], xmm1
    }
#elif defined ( ID_QNX_X86_SSE2_ASM )
	__asm__ __volatile__(
			"movd		(%[min]), %%xmm0\n"										// xmm0 = minNormal
			"movd		(%[max]), %%xmm1\n"										// xmm1 = maxNormal
			"punpcklbw	_ZL16SIMD_SSE2_byte_0, %%xmm0\n"
			"punpcklbw	_ZL16SIMD_SSE2_byte_0, %%xmm1\n"
			"movdqa		%%xmm1, %%xmm2\n"
			"psubw		%%xmm0, %%xmm2\n"
			"psubw		_ZL35SIMD_SSE2_word_insetNormalDXT5Round, %%xmm2\n"
			"pand		_ZL34SIMD_SSE2_word_insetNormalDXT5Mask, %%xmm2\n"		// xmm2 = inset (1 & 3)

			"pmullw		_ZL37SIMD_SSE2_word_insetNormalDXT5ShiftUp, %%xmm0\n"
			"pmullw		_ZL37SIMD_SSE2_word_insetNormalDXT5ShiftUp, %%xmm1\n"
			"paddw		%%xmm2, %%xmm0\n"
			"psubw		%%xmm2, %%xmm1\n"
			"pmulhw		_ZL39SIMD_SSE2_word_insetNormalDXT5ShiftDown, %%xmm0\n"	// xmm0 = mini
			"pmulhw		_ZL39SIMD_SSE2_word_insetNormalDXT5ShiftDown, %%xmm1\n"	// xmm1 = maxi

			// mini and maxi must be >= 0 and <= 255
			"pmaxsw		_ZL16SIMD_SSE2_word_0, %%xmm0\n"
			"pmaxsw		_ZL16SIMD_SSE2_word_0, %%xmm1\n"
			"pminsw		_ZL18SIMD_SSE2_word_255, %%xmm0\n"
			"pminsw		_ZL18SIMD_SSE2_word_255, %%xmm1\n"

			"movdqa		%%xmm0, %%xmm2\n"
			"movdqa		%%xmm1, %%xmm3\n"
			"pand		_ZL39SIMD_SSE2_word_insetNormalDXT5QuantMask, %%xmm0\n"
			"pand		_ZL39SIMD_SSE2_word_insetNormalDXT5QuantMask, %%xmm1\n"
			"pmulhw		_ZL33SIMD_SSE2_word_insetNormalDXT5Rep, %%xmm2\n"
			"pmulhw		_ZL33SIMD_SSE2_word_insetNormalDXT5Rep, %%xmm3\n"
			"por		%%xmm2, %%xmm0\n"
			"por		%%xmm3, %%xmm1\n"
			"packuswb	%%xmm0, %%xmm0\n"
			"packuswb	%%xmm1, %%xmm1\n"
			"movd		%%xmm0, (%[min])\n"
			"movd		%%xmm1, (%[max])\n"
			:: [min] "r" (minNormal), [max] "r" (maxNormal) : "xmm0", "xmm1", "xmm2", "xmm3", "memory");
#elif defined ( ID_WIN_X86_SSE2_INTRIN ) || defined ( ID_QNX_X86_SSE2_INTRIN )
	__m128i temp0, temp1, temp2, temp3;

	temp0 = _mm_cvtsi32_si128( *(int *)minNormal );
	temp1 = _mm_cvtsi32_si128( *(int *)maxNormal );

	temp0 = _mm_unpacklo_epi8( temp0, (const __m128i &)SIMD_SSE2_byte_0 );
	temp1 = _mm_unpacklo_epi8( temp1, (const __m128i &)SIMD_SSE2_byte_0 );

	temp2 = _mm_sub_epi16( temp1, temp0 );
	temp2 = _mm_sub_epi16( temp2, (const __m128i &)SIMD_SSE2_word_insetNormalDXT5Round );
	temp2 = _mm_and_si128( temp2, (const __m128i &)SIMD_SSE2_word_insetNormalDXT5Mask );		// xmm2 = inset (1 & 3)

	temp0 = _mm_mullo_epi16( temp0, (const __m128i &)SIMD_SSE2_word_insetNormalDXT5ShiftUp );
	temp1 = _mm_mullo_epi16( temp1, (const __m128i &)SIMD_SSE2_word_insetNormalDXT5ShiftUp );
	temp0 = _mm_add_epi16( temp0, temp2 );
	temp1 = _mm_sub_epi16( temp1, temp2 );
	temp0 = _mm_mulhi_epi16( temp0, (const __m128i &)SIMD_SSE2_word_insetNormalDXT5ShiftDown );	// xmm0 = mini
	temp1 = _mm_mulhi_epi16( temp1, (const __m128i &)SIMD_SSE2_word_insetNormalDXT5ShiftDown );	// xmm1 = maxi

	// mini and maxi must be >= 0 and <= 255
	temp0 = _mm_max_epi16( temp0, (const __m128i &)SIMD_SSE2_word_0 );
	temp1 = _mm_max_epi16( temp1, (const __m128i &)SIMD_SSE2_word_0 );
	temp0 = _mm_min_epi16( temp0, (const __m128i &)SIMD_SSE2_word_255 );
	temp1 = _mm_min_epi16( temp1, (const __m128i &)SIMD_SSE2_word_255 );

	temp0 = _mm_and_si128( temp0, (const __m128i &)SIMD_SSE2_word_insetNormalDXT5QuantMask );
	temp1 = _mm_and_si128( temp1, (const __m128i &)SIMD_SSE2_word_insetNormalDXT5QuantMask );
	temp2 = _mm_mulhi_epi16( temp0, (const __m128i &)SIMD_SSE2_word_insetNormalDXT5Rep );
	temp3 = _mm_mulhi_epi16( temp1, (const __m128i &)SIMD_SSE2_word_insetNormalDXT5Rep );
	temp0 = _mm_or_si128( temp0, temp2 );
	temp1 = _mm_or_si128( temp1, temp3 );
	temp0 = _mm_packus_epi16( temp0, temp0 );
	temp1 = _mm_packus_epi16( temp1, temp1 );

	*(int *)minNormal = _mm_cvtsi128_si32( temp0 );
	*(int *)maxNormal = _mm_cvtsi128_si32( temp1 );
#else
	assert( false );
#endif
}

/*
========================
idDxtEncoder::CompressNormalMapDXT5Fast_SSE2

params:	inBuf		- image to compress in _y_x component order
paramO:	outBuf		- result of compression
params:	width		- width of image
params:	height		- height of image
========================
*/
void idDxtEncoder::CompressNormalMapDXT5Fast_SSE2( const byte *inBuf, byte *outBuf, int width, int height ) {
	ALIGN16( byte block[64] );
	ALIGN16( byte normal1[4] );
	ALIGN16( byte normal2[4] );

	assert( width >= 4 && ( width & 3 ) == 0 );
	assert( height >= 4 && ( height & 3 ) == 0 );

	this->width = width;
	this->height = height;
	this->outData = outBuf;

	for ( int j = 0; j < height; j += 4, inBuf += width * 4*4 ) {
		for ( int i = 0; i < width; i += 4 ) {
			ExtractBlock_SSE2( inBuf + i * 4, width, block );
			GetMinMaxBBox_SSE2( block, normal1, normal2 );
			InsetNormalsBBoxDXT5_SSE2( normal1, normal2 );

			// Write out Nx into alpha channel.
			EmitByte( normal2[3] );
			EmitByte( normal1[3] );
			EmitAlphaIndices_SSE2( block, 3*8, normal1[3], normal2[3] );

			// Write out Ny into green channel.
			EmitUShort( ColorTo565( block[0], normal2[1], block[2] ) );
			EmitUShort( ColorTo565( block[0], normal1[1], block[2] ) );
			EmitGreenIndices_SSE2( block, 1*8, normal1[1], normal2[1] );
		}
		outData += dstPadding;
		inBuf += srcPadding;
	}

#ifdef TEST_COMPRESSION
	int tmpDstPadding = dstPadding;
	dstPadding = 0;
	byte * testOutBuf = (byte *) _alloca16( width * height );
	CompressNormalMapDXT5Fast_Generic( inBuf, testOutBuf, width, height );
	for ( int j = 0; j < height / 4; j++ ) {
		for ( int i = 0; i < width / 4; i++ ) {
			byte * ptr1 = outBuf + ( j * width/4 + i ) * 16 + j * tmpDstPadding;
			byte * ptr2 = testOutBuf + ( j * width/4 + i ) * 16;
			for ( int k = 0; k < 16; k++ ) {
				assert( ptr1[k] == ptr2[k] );
			}
		}
	}
	dstPadding = tmpDstPadding;
#endif
}

#endif
