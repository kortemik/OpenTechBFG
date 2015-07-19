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
Contains the EtcDecoder implementation.
================================================================================================
*/

#pragma hdrstop
#include "ETCCodec_local.h"
#include "ETCCodec.h"

/*
========================
idEtcDecoder::GenericDecompress
========================
*/
void idEtcDecoder::GenericDecompress( const byte *inBuf, byte *outBuf, int width, int height, int sourceType ) {
#ifdef _DEBUG
	switch ( sourceType ) {
	case TEXTURE_TYPE_ETC1:
	case TEXTURE_TYPE_ETC2_PUNCHTHROUGH:
	case TEXTURE_TYPE_ETC2_EAC:
		break;
	default:
		return;
	}
#endif

	texgen::Texture sourceTex;
	sourceTex.info = texgen::match_texture_type( sourceType );
	int block_width = sourceTex.info->block_width;
	int block_height = sourceTex.info->block_height;

	sourceTex.width = width;
	sourceTex.height = height;
	sourceTex.extended_width = ( ( width + block_width - 1 ) / block_width ) * block_width;
	sourceTex.extended_height = ( ( height + block_height - 1 ) / block_height ) * block_height;
	sourceTex.bits_per_block = sourceTex.info->internal_bits_per_block;
	sourceTex.type = sourceType;
	sourceTex.block_width = block_width;
	sourceTex.block_height = block_height;
	sourceTex.pixels = ( unsigned int * )inBuf;

	texgen::Options opt;
	texgen::init_options( &opt );

	texgen::Image destImage;
	if ( texgen::copy_image_to_uncompressed_texture( &opt, &destImage, TEXTURE_TYPE_UNCOMPRESSED_RGBA8, &sourceTex ) ) {
		memcpy( outBuf, destImage.pixels, destImage.width * destImage.height * sizeof( uint32 ) );
		texgen::destroy_image( &destImage );
	}
}

/*
========================
idEtcDecoder::DecompressImageETC1
========================
*/
void idEtcDecoder::DecompressImageETC1( const byte *inBuf, byte *outBuf, int width, int height ) {
	GenericDecompress( inBuf, outBuf, width, height, TEXTURE_TYPE_ETC1 );
}

/*
========================
idEtcDecoder::DecompressImageETC2PunchAlpha
========================
*/
void idEtcDecoder::DecompressImageETC2PunchAlpha( const byte *inBuf, byte *outBuf, int width, int height ) {
	GenericDecompress( inBuf, outBuf, width, height, TEXTURE_TYPE_ETC2_PUNCHTHROUGH );
}

/*
========================
idEtcDecoder::DecompressImageETC2Alpha
========================
*/
void idEtcDecoder::DecompressImageETC2Alpha( const byte *inBuf, byte *outBuf, int width, int height ) {
	GenericDecompress( inBuf, outBuf, width, height, TEXTURE_TYPE_ETC2_EAC );
}
