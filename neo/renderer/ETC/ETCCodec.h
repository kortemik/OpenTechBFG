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
#ifndef __ETCCODEC_H__
#define __ETCCODEC_H__

/*
================================================================================================
Contains the EtcEncoder declarations.
================================================================================================
*/

/*
================================================
idEtcEncoder encodes Images in a number of ETC formats. Raw input Images are assumed to be in
4-byte RGBA format. Raw input NormalMaps are assumed to be in 4-byte tangent-space NxNyNz format.

The supported formats are:
	* ETC1
	* ETC2 (punchthrough Alpha)
	* ETC2 (alpha)
================================================
*/
class idEtcEncoder {
public:
				idEtcEncoder() { srcPadding = dstPadding = 0; }
				~idEtcEncoder() {}

	void		SetSrcPadding( int pad ) { srcPadding = pad; }
	void		SetDstPadding( int pad ) { dstPadding = pad; }

	// ETC1 (no alpha)
	void		CompressImageETC1HQ( const byte *inBuf, byte *outBuf, int width, int height );
	void		CompressImageETC1Fast( const byte *inBuf, byte *outBuf, int width, int height );

	// ETC2 (punchthrough alpha)
	void		CompressImageETC2PunchAlphaHQ( const byte *inBuf, byte *outBuf, int width, int height );
	void		CompressImageETC2PunchAlphaFast( const byte *inBuf, byte *outBuf, int width, int height );

	// ETC2 (alpha)
	void		CompressImageETC2AlphaHQ( const byte *inBuf, byte *outBuf, int width, int height );
	void		CompressImageETC2AlphaFast( const byte *inBuf, byte *outBuf, int width, int height );

	// ETC1 (no alpha) with variable quality (1, fastest; 3 highest quality)
	void		CompressImageETC1_Quality( const byte *inBuf, byte *outBuf, int width, int height, int quality );

	// ETC1 (punchthrough/alpha) with variable quality (1, fastest; 3 highest quality)
	void		CompressImageETC2_Quality( const byte *inBuf, byte *outBuf, int width, int height, bool punchAlpha, int quality );

	static bool	CanEncodeAsETC1( const byte *inBuf, int width, int height );
	static bool	CanEncodeAsETC2_Punchthrough( const byte *inBuf, int width, int height );

private:
	enum AlphaSignificance {
		AlphaError,
		NoAlpha,
		Punchthrough,
		Alpha
	};

	int							width;
	int							height;
	byte *						outData;
	int							srcPadding;
	int							dstPadding;

	void						EmitByte( byte b );
	void						EmitUShort( unsigned short s );
	void						EmitUInt( unsigned int i );
	static AlphaSignificance	IsAlphaSignificant( const byte *inBuf, int width, int height );

	void						ExtractBlock( uint32 * outBlock, const uint32 * inBuf ) const;
	void						PadBlock( uint32 * outBuf, const uint32 * inBuf, int width, int height ) const;

	void						WriteTinyETC1Block( const uint32 * inBuf, int width, int height, int quality, bool dither );
	uint32						WriteETC1Block( const uint32 * inBuf, int quality, bool dither );
};

/*
========================
idEtcEncoder::CompressImageETC1HQ
========================
*/
ID_INLINE void idEtcEncoder::CompressImageETC1HQ( const byte *inBuf, byte *outBuf, int width, int height ) {
	CompressImageETC1_Quality( inBuf, outBuf, width, height, 3 );
}

/*
========================
idEtcEncoder::CompressImageETC1Fast
========================
*/
ID_INLINE void idEtcEncoder::CompressImageETC1Fast( const byte *inBuf, byte *outBuf, int width, int height ) {
	CompressImageETC1_Quality( inBuf, outBuf, width, height, 1 );
}

/*
========================
idEtcEncoder::CompressImageETC2PunchAlphaHQ
========================
*/
ID_INLINE void idEtcEncoder::CompressImageETC2PunchAlphaHQ( const byte *inBuf, byte *outBuf, int width, int height ) {
	CompressImageETC2_Quality( inBuf, outBuf, width, height, true, 3 );
}

/*
========================
idEtcEncoder::CompressImageETC2PunchAlphaFast
========================
*/
ID_INLINE void idEtcEncoder::CompressImageETC2PunchAlphaFast( const byte *inBuf, byte *outBuf, int width, int height ) {
	CompressImageETC2_Quality( inBuf, outBuf, width, height, true, 1 );
}

/*
========================
idEtcEncoder::CompressImageETC2AlphaHQ
========================
*/
ID_INLINE void idEtcEncoder::CompressImageETC2AlphaHQ( const byte *inBuf, byte *outBuf, int width, int height ) {
	CompressImageETC2_Quality( inBuf, outBuf, width, height, false, 3 );
}

/*
========================
idEtcEncoder::CompressImageETC2AlphaFast
========================
*/
ID_INLINE void idEtcEncoder::CompressImageETC2AlphaFast( const byte *inBuf, byte *outBuf, int width, int height ) {
	CompressImageETC2_Quality( inBuf, outBuf, width, height, false, 1 );
}

/*
========================
idEtcEncoder::CanEncodeAsETC1
========================
*/
ID_INLINE bool idEtcEncoder::CanEncodeAsETC1( const byte *inBuf, int width, int height ) {
	return IsAlphaSignificant( inBuf, width, height ) == NoAlpha;
}

/*
========================
idEtcEncoder::CanEncodeAsETC2_Punchthrough
========================
*/
ID_INLINE bool idEtcEncoder::CanEncodeAsETC2_Punchthrough( const byte *inBuf, int width, int height ) {
	return IsAlphaSignificant( inBuf, width, height ) == Punchthrough;
}

/*
========================
idEtcEncoder::EmitByte
========================
*/
ID_INLINE void idEtcEncoder::EmitByte( byte b ) {
	*outData = b;
	outData += 1;
}

/*
========================
idEtcEncoder::EmitUShort
========================
*/
ID_INLINE void idEtcEncoder::EmitUShort( unsigned short s ) {
	*((unsigned short *)outData) = s;
	outData += 2;
}

/*
========================
idEtcEncoder::EmitUInt
========================
*/
ID_INLINE void idEtcEncoder::EmitUInt( unsigned int i ) {
	*((unsigned int *)outData) = i;
	outData += 4;
}

#endif // !__ETCCODEC_H__
