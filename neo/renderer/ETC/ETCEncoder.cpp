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

static void compress_callback(texgen::BlockUserData *user_data) {
	// Do nothing.
}

/*
========================
idEtcEncoder::GenericCompress

params:	inBuf		- image to compress
paramO:	outBuf		- result of compression
params:	width		- width of image
params:	height		- height of image
params: quality		- quality of compression (1 - 4)
params: type		- the format to compress to
========================
*/
void idEtcEncoder::GenericCompress( const byte *inBuf, byte *outBuf, int width, int height, int quality, int type ) {
	quality--;
	if ( quality < SPEED_ULTRA || quality > SPEED_SLOW ) {
		return;
	}

	texgen::Image sourceImage;
	sourceImage.pixels = (uint32 *)inBuf;
	sourceImage.width = width;
	sourceImage.height = height;
	sourceImage.extended_width = width;
	sourceImage.extended_height = height;
	sourceImage.alpha_bits = 8;
	sourceImage.nu_components = 4;
	sourceImage.bits_per_component = 8;
	sourceImage.is_signed = 0;
	sourceImage.srgb = 0;
	sourceImage.is_half_float = 0;

	texgen::Options opt;
	texgen::init_options( &opt );
	opt.speed = quality;

	texgen::Texture destTexture;
	texgen::compress_image( &opt, &sourceImage, type, compress_callback, &destTexture, 0, 0, 0 );

	int n = (destTexture.extended_height / destTexture.block_height) * (destTexture.extended_width / destTexture.block_width);
	memcpy( outBuf, destTexture.pixels, n * (destTexture.bits_per_block / 8) );

	texgen::destroy_texture( &destTexture );
}

/*
========================
idEtcEncoder::CompressImageETC1_Quality

params:	inBuf		- image to compress
paramO:	outBuf		- result of compression
params:	width		- width of image
params:	height		- height of image
params: quality		- quality of compression (1 - 4)
========================
*/
void idEtcEncoder::CompressImageETC1_Quality( const byte *inBuf, byte *outBuf, int width, int height, int quality ) {
	GenericCompress( inBuf, outBuf, width, height, quality, TEXTURE_TYPE_ETC1 );
}

/*
========================
idEtcEncoder::CompressImageETC2_Quality

params:	inBuf		- image to compress
paramO:	outBuf		- result of compression
params:	width		- width of image
params:	height		- height of image
params: punchAlpha	- if alpha should be punchthrough, otherwise full alpha
params: quality		- quality of compression (1 - 4)
========================
*/
void idEtcEncoder::CompressImageETC2_Quality( const byte *inBuf, byte *outBuf, int width, int height, bool punchAlpha, int quality ) {
	GenericCompress( inBuf, outBuf, width, height, quality, punchAlpha ? TEXTURE_TYPE_ETC2_PUNCHTHROUGH : TEXTURE_TYPE_ETC2_EAC );
}
