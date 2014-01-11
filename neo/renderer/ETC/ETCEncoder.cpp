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
Contains the EtcEncoder implementation.
================================================================================================
*/

#pragma hdrstop
#include "ETCCodec_local.h"
#include "ETCCodec.h"

/*
========================
idEtcEncoder::CompressImageETC1_Quality

params:	inBuf		- image to compress
paramO:	outBuf		- result of compression
params:	width		- width of image
params:	height		- height of image
params: quality		- quality of compression (1 - 3, negative for dithering)
========================
*/
void idEtcEncoder::CompressImageETC1_Quality( const byte *inBuf, byte *outBuf, int width, int height, int quality ) {
	ALIGN16( uint32 block[16] );

	// init etc1 block if needed
	static bool etc1_block_init = false;
	if ( !etc1_block_init ) {
		rg_etc1::pack_etc1_block_init();
		etc1_block_init = true;
	}

	this->width = width;
	this->height = height;
	this->outData = outBuf;

	if ( width > 4 && ( width & 3 ) != 0 ) {
		return;
	}
	if ( height > 4 && ( height & 3 ) != 0 ) {
		return;
	}

	int absQuality = abs( quality );
	if ( absQuality < 1 || absQuality > 3 ) {
		return;
	}

	if ( width < 4 || height < 4 ) {
		WriteTinyETC1Block( (uint32 *)inBuf, width, height, absQuality - 1, quality < 0 );
		return;
	}

	absQuality--;
	bool dither = quality < 0;

	//TODO
	for ( int j = 0; j < height; j += 4, inBuf += width * 4*4 ) {
		for ( int i = 0; i < width; i += 4 ) {
			ExtractBlock( block, (uint32 *)inBuf ); //TODO

			WriteETC1Block( block, absQuality, dither );

			//idLib::Printf( "\r%3d%%", ( j * width + i ) * 100 / ( width * height ) );
		}
		outData += dstPadding;
		inBuf += srcPadding;
	}

	//idLib::Printf( "\r100%%\n" );
}

/*
========================
idEtcEncoder::IsAlphaSignificant
========================
*/
idEtcEncoder::AlphaSignificance idEtcEncoder::IsAlphaSignificant( const byte *inBuf, int width, int height ) {
	if ( width <= 0 ) {
		return AlphaError;
	}
	byte alpha = inBuf[3];
	if ( alpha != 0 && alpha != 0xFF ) {
		return Alpha;
	}
	//XXX optimize?
	idEtcEncoder::AlphaSignificance ret = NoAlpha;
	for ( int i = 4; i < width * height * 4; i += 4 ) {
		if ( inBuf[i + 3] != alpha ) {
			if ( inBuf[i + 3] != 0xFF && inBuf[i + 3] != 0 ) {
				return Alpha;
			} else if ( ( alpha == 0xFF	&& inBuf[i + 3] == 0 ) ||
						( alpha == 0	&& inBuf[i + 3] == 0xFF ) ) {
				ret = Punchthrough;
			}
		}
	}
	return ret;
}

/*
========================
idEtcEncoder::ExtractBlock
========================
*/
void idEtcEncoder::ExtractBlock( uint32 * outBlock, const uint32 * inBuf ) const {
	memcpy( &outBlock[0], &inBuf[0], 4 * sizeof( uint32 ) );
	//TODO
}

/*
========================
idEtcEncoder::PadBlock

Pads a color block with 0xFF
========================
*/
void idEtcEncoder::PadBlock( uint32 * outBuf, const uint32 * inBuf, int width, int height ) const {
	if ( width >= 4 && height >= 4 ) {
		return;
	}
	int pad = 4 - width;
	for ( int i = 0; i < 4; i++ ) {
		if ( i < height ) {
			// copy existing data by row
			memcpy( &outBuf[i * 4], &inBuf[i * width], width * sizeof( uint32 ) );
			// pad remaining of row
			memset( &( ( ( byte * )outBuf )[width * sizeof( uint32 ) + i * 4] ), 0xFF, pad * sizeof( uint32 ) );
		} else {
			outBuf[i] = 0xFFFFFFFFU;
		}
	}
}

/*
========================
idEtcEncoder::WriteTinyETC1Block
========================
*/
void idEtcEncoder::WriteTinyETC1Block( const uint32 * inBuf, int width, int height, int quality, bool dither ) {
	ALIGN16( uint32 block[16] );
	PadBlock( block, inBuf, width, height );
	WriteETC1Block( block, quality, dither );
}

/*
========================
idEtcEncoder::WriteETC1Block

Takes 4x4 block, emits one ETC1 block
========================
*/
uint32 idEtcEncoder::WriteETC1Block( const uint32 * inBuf, int quality, bool dither ) {
	ALIGN16( uint32 block[2] );

	rg_etc1::etc1_pack_params params;
	params.m_dithering = dither;
	params.m_quality = (rg_etc1::etc1_quality)quality;

	uint32 err = rg_etc1::pack_etc1_block( (void *)block, inBuf, params );

	EmitUInt( block[0] );
	EmitUInt( block[1] );

	return err;
}

/*
========================
idEtcEncoder::CompressImageETC2_Quality

params:	inBuf		- image to compress
paramO:	outBuf		- result of compression
params:	width		- width of image
params:	height		- height of image
params: punchAlpha	- if alpha should be punchthrough, otherwise full alpha
params: quality		- ?
========================
*/
void idEtcEncoder::CompressImageETC2_Quality( const byte *inBuf, byte *outBuf, int width, int height, bool punchAlpha, int quality ) {
	//TODO
}

//TODO
