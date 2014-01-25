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
#pragma hdrstop
#include "../idlib/precompiled.h"

/*
================================================================================================

	idBinaryImage

================================================================================================
*/

#include "tr_local.h"
#include "dxt/DXTCodec.h"
#ifdef GL_ES_VERSION_2_0
#include "etc/ETCCodec.h"
#endif
#include "color/ColorSpace.h"

idCVar image_highQualityCompression( "image_highQualityCompression", "0", CVAR_BOOL, "Use high quality (slow) compression" );

#ifdef GL_ES_VERSION_2_0
idCVar image_attemptETC1Encoding( "image_attemptETC1Encoding", "0", CVAR_BOOL, "Attempt to encode compressed images as ETC1. Valid only if alpha doesn't exist" );
idCVar image_attemptETC2PunchthroughEncoding( "image_attemptETC2PunchthroughEncoding", "0", CVAR_BOOL, "Attempt to encode compressed images as ETC2-Punchthrough. Valid only if alpha is \"off\" or \"on\"" );
#endif

/*
========================
idBinaryImage::Load2DFromMemory
========================
*/
void idBinaryImage::Load2DFromMemory( int width, int height, const byte * pic_const, int numLevels, textureFormat_t & textureFormat, textureColor_t & colorFormat, bool gammaMips ) {
	fileData.textureType = TT_2D;
	fileData.format = textureFormat;
	fileData.colorFormat = colorFormat;
	fileData.width = width;
	fileData.height = height;
	fileData.numLevels = numLevels;

	byte * pic = (byte *)Mem_Alloc( width * height * 4, TAG_TEMP );
	memcpy( pic, pic_const, width * height * 4 );

	if ( colorFormat == CFM_YCOCG_DXT5 ) {
		if ( textureFormat == FMT_DXT5 ) {
			// convert the image data to YCoCg and use the YCoCgDXT5 compressor
			idColorSpace::ConvertRGBToCoCg_Y( pic, pic, width, height );
		} else {
			// get within the YCoCg range while keeping it in RGB
			idColorSpace::ConvertRGBToCoCg_YInPlace( pic, pic, width, height );
		}
	} else if ( colorFormat == CFM_NORMAL_DXT5 ) {
		//XXX For OpenGL ES, would GL_COMPRESSED_RG11_EAC or GL_3DC_XY_AMD be better then a standard RGBA image?
		// Blah, HQ swizzles automatically, Fast doesn't
		if ( textureFormat != FMT_DXT5 || !image_highQualityCompression.GetBool() ) {
			for ( int i = 0; i < width * height; i++ ) {
				pic[i*4+3] = pic[i*4+0];
				pic[i*4+0] = 0;
				pic[i*4+2] = 0;
			}
		}
	} else if ( colorFormat == CFM_GREEN_ALPHA ) {
		for ( int i = 0; i < width * height; i++ ) {
			pic[i*4+1] = pic[i*4+3];
			pic[i*4+0] = 0;
			pic[i*4+2] = 0;
			pic[i*4+3] = 0;
		}
	}

	int	scaledWidth = width;
	int scaledHeight = height;
	images.SetNum( numLevels );
	for ( int level = 0; level < images.Num(); level++ ) {
		idBinaryImageData &img = images[ level ];

		// Images that are going to be DXT compressed and aren't multiples of 4 need to be
		// padded out before compressing.
		byte * dxtPic = pic;
		int	dxtWidth = 0;
		int	dxtHeight = 0;
#ifdef GL_ES_VERSION_2_0
		if ( textureFormat == FMT_DXT1 || textureFormat == FMT_DXT5 || textureFormat == FMT_ETC1 || textureFormat == FMT_ETC2_PUNCH || textureFormat == FMT_ETC2_ALPHA ) {
#else
		if ( textureFormat == FMT_DXT1 || textureFormat == FMT_DXT5 ) {
#endif
			if ( ( scaledWidth & 3 ) || ( scaledHeight & 3 ) ) {
				dxtWidth = ( scaledWidth + 3 ) & ~3;
				dxtHeight = ( scaledHeight + 3 ) & ~3;
				dxtPic = (byte *)Mem_ClearedAlloc( dxtWidth*4*dxtHeight, TAG_IMAGE );
				for ( int i = 0; i < scaledHeight; i++ ) {
					memcpy( dxtPic + i*dxtWidth*4, pic + i*scaledWidth*4, scaledWidth*4 );
				}
			} else {
				dxtPic = pic;
				dxtWidth = scaledWidth;
				dxtHeight = scaledHeight;
			}
		}

		img.level = level;
		img.destZ = 0;
		img.width = scaledWidth;
		img.height = scaledHeight;

		// compress data or convert floats as necessary
		if ( textureFormat == FMT_DXT1 ) {
			idDxtEncoder dxt;
			img.Alloc( dxtWidth * dxtHeight / 2 );
			if ( image_highQualityCompression.GetBool() ) {
				dxt.CompressImageDXT1HQ( dxtPic, img.data, dxtWidth, dxtHeight );
			} else {
				dxt.CompressImageDXT1Fast( dxtPic, img.data, dxtWidth, dxtHeight );
			}
		} else if ( textureFormat == FMT_DXT5 ) {
			idDxtEncoder dxt;
			img.Alloc( dxtWidth * dxtHeight );
			if ( colorFormat == CFM_NORMAL_DXT5 ) {
				if ( image_highQualityCompression.GetBool() ) {
					dxt.CompressNormalMapDXT5HQ( dxtPic, img.data, dxtWidth, dxtHeight );
				} else {
					dxt.CompressNormalMapDXT5Fast( dxtPic, img.data, dxtWidth, dxtHeight );
				}
			} else if ( colorFormat == CFM_YCOCG_DXT5 ) {
				if ( image_highQualityCompression.GetBool() ) {
					dxt.CompressYCoCgDXT5HQ( dxtPic, img.data, dxtWidth, dxtHeight );
				} else {
					dxt.CompressYCoCgDXT5Fast( dxtPic, img.data, dxtWidth, dxtHeight );
				}
			} else {
				fileData.colorFormat = colorFormat = CFM_DEFAULT;
				if ( image_highQualityCompression.GetBool() ) {
					dxt.CompressImageDXT5HQ( dxtPic, img.data, dxtWidth, dxtHeight );
				} else {
					dxt.CompressImageDXT5Fast( dxtPic, img.data, dxtWidth, dxtHeight );
				}
			}
#ifdef GL_ES_VERSION_2_0
		} else if ( textureFormat == FMT_ETC1 ) {
			img.Alloc( dxtWidth * dxtHeight / 2 );
			idEtcEncoder etc;
			if ( image_highQualityCompression.GetBool() ) {
				etc.CompressImageETC1HQ( dxtPic, img.data, dxtWidth, dxtHeight );
			} else {
				etc.CompressImageETC1Fast( dxtPic, img.data, dxtWidth, dxtHeight );
			}
		} else if ( textureFormat == FMT_ETC2_PUNCH ) {
			img.Alloc( dxtWidth * dxtHeight / 2 );
			idEtcEncoder etc;
			if ( image_highQualityCompression.GetBool() ) {
				etc.CompressImageETC2PunchAlphaHQ( dxtPic, img.data, dxtWidth, dxtHeight );
			} else {
				etc.CompressImageETC2PunchAlphaFast( dxtPic, img.data, dxtWidth, dxtHeight );
			}
		} else if ( textureFormat == FMT_ETC2_ALPHA ) {
			img.Alloc( dxtWidth * dxtHeight );
			idEtcEncoder etc;
			if ( !( colorFormat == CFM_NORMAL_DXT5 || colorFormat == CFM_YCOCG_DXT5 ) ) {
				// For any non-normal or YCoCg image, set it's format to default
				fileData.colorFormat = colorFormat = CFM_DEFAULT;
			}
			if ( image_highQualityCompression.GetBool() ) {
				etc.CompressImageETC2AlphaHQ( dxtPic, img.data, dxtWidth, dxtHeight );
			} else {
				etc.CompressImageETC2AlphaFast( dxtPic, img.data, dxtWidth, dxtHeight );
			}
#endif
		} else if ( textureFormat == FMT_LUM8 || textureFormat == FMT_INT8 ) {
			// LUM8 and INT8 just read the red channel
			img.Alloc( scaledWidth * scaledHeight );
			for ( int i = 0; i < img.dataSize; i++ ) {
				img.data[ i ] = pic[ i * 4 ];
			}
		} else if ( textureFormat == FMT_ALPHA ) {
			// ALPHA reads the alpha channel
			img.Alloc( scaledWidth * scaledHeight );
			for ( int i = 0; i < img.dataSize; i++ ) {
				img.data[ i ] = pic[ i * 4 + 3 ];
			}
		} else if ( textureFormat == FMT_L8A8 ) {
			// L8A8 reads the alpha and red channels
			img.Alloc( scaledWidth * scaledHeight * 2 );
			for ( int i = 0; i < img.dataSize / 2; i++ ) {
				img.data[ i * 2 + 0 ] = pic[ i * 4 + 0 ];
				img.data[ i * 2 + 1 ] = pic[ i * 4 + 3 ];
			}
		} else if ( textureFormat == FMT_RGB565 ) {
			img.Alloc( scaledWidth * scaledHeight * 2 );
			for ( int i = 0; i < img.dataSize / 2; i++ ) {
				unsigned short color = ( ( pic[ i * 4 + 0 ] >> 3 ) << 11 ) | ( ( pic[ i * 4 + 1 ] >> 2 ) << 5 ) | ( pic[ i * 4 + 2 ] >> 3 );
				img.data[ i * 2 + 0 ] = ( color >> 8 ) & 0xFF;
				img.data[ i * 2 + 1 ] = color & 0xFF;
			}
		} else {
			fileData.format = textureFormat = FMT_RGBA8;
			img.Alloc( scaledWidth * scaledHeight * 4 );
			for ( int i = 0; i < img.dataSize; i++ ) {
				img.data[ i ] = pic[ i ];
			}
		}

		// if we had to pad to quads, free the padded version
		if ( pic != dxtPic ) {
			Mem_Free( dxtPic );
			dxtPic = NULL;
		}

		// downsample for the next level
		byte * shrunk = NULL;
		if ( gammaMips ) {
			shrunk = R_MipMapWithGamma( pic, scaledWidth, scaledHeight );
		} else {
			shrunk = R_MipMap( pic, scaledWidth, scaledHeight );
		}
		Mem_Free( pic );
		pic = shrunk;

		scaledWidth = Max( 1, scaledWidth >> 1 );
		scaledHeight = Max( 1, scaledHeight >> 1 );
	}

	Mem_Free( pic );
}

/*
========================
PadImageTo4x4

DXT Compression requres a complete 4x4 block, even if the GPU will only be sampling
a subset of it, so pad to 4x4 with replicated texels to maximize compression.
========================
*/
static void PadImageTo4x4( const byte *src, int width, int height, byte dest[64] ) {
	// we probably will need to support this for non-square images, but I'll address
	// that when needed
	assert( width <= 4 && height <= 4 );
	assert( width > 0 && height > 0 );

	for ( int y = 0 ; y < 4 ; y++ ) {
		int	sy = y % height;
		for ( int x = 0 ; x < 4 ; x++ ) {
			int	sx = x % width;
			for ( int c = 0 ; c < 4 ; c++ ) {
				dest[(y*4+x)*4+c] = src[(sy*width+sx)*4+c];
			}
		}
	}
}

/*
========================
idBinaryImage::LoadCubeFromMemory
========================
*/
void idBinaryImage::LoadCubeFromMemory( int width, const byte * pics[6], int numLevels, textureFormat_t & textureFormat, bool gammaMips ) {
	fileData.textureType = TT_CUBIC;
	fileData.format = textureFormat;
	fileData.colorFormat = CFM_DEFAULT;
	fileData.height = fileData.width = width;
	fileData.numLevels = numLevels;

	images.SetNum( fileData.numLevels * 6 );

	for ( int side = 0; side < 6; side++ ) {
		const byte *orig = pics[side];
		const byte *pic = orig;
		int	scaledWidth = fileData.width;
		for ( int level = 0; level < fileData.numLevels; level++ ) {
			// compress data or convert floats as necessary
			idBinaryImageData &img = images[ level * 6 + side ];

			// handle padding blocks less than 4x4 for the DXT compressors
			ALIGN16( byte padBlock[64] );
			int		padSize;
			const byte *padSrc;
#ifdef GL_ES_VERSION_2_0
			if ( scaledWidth < 4 && ( textureFormat == FMT_DXT1 || textureFormat == FMT_DXT5 || textureFormat == FMT_ETC1 || textureFormat == FMT_ETC2_PUNCH || textureFormat == FMT_ETC2_ALPHA ) ) {
#else
			if ( scaledWidth < 4 && ( textureFormat == FMT_DXT1 || textureFormat == FMT_DXT5 ) ) {
#endif
				PadImageTo4x4( pic, scaledWidth, scaledWidth, padBlock );
				padSize = 4;
				padSrc = padBlock;
			} else {
				padSize = scaledWidth;
				padSrc = pic;
			}

			img.level = level;
			img.destZ = side;
			img.width = padSize;
			img.height = padSize;
			if ( textureFormat == FMT_DXT1 ) {
				img.Alloc( padSize * padSize / 2 );
				idDxtEncoder dxt;
				dxt.CompressImageDXT1Fast( padSrc, img.data, padSize, padSize );
			} else if ( textureFormat == FMT_DXT5 ) {
				img.Alloc( padSize * padSize );
				idDxtEncoder dxt;
				dxt.CompressImageDXT5Fast( padSrc, img.data, padSize, padSize );
#ifdef GL_ES_VERSION_2_0
			} else if ( textureFormat == FMT_ETC1 ) {
				img.Alloc( padSize * padSize / 2 );
				idEtcEncoder etc;
				etc.CompressImageETC1Fast( padSrc, img.data, padSize, padSize );
			} else if ( textureFormat == FMT_ETC2_PUNCH ) {
				img.Alloc( padSize * padSize / 2 );
				idEtcEncoder etc;
				etc.CompressImageETC2PunchAlphaFast( padSrc, img.data, padSize, padSize );
			} else if ( textureFormat == FMT_ETC2_ALPHA ) {
				img.Alloc( padSize * padSize );
				idEtcEncoder etc;
				etc.CompressImageETC2AlphaFast( padSrc, img.data, padSize, padSize );
#endif
			} else {
				fileData.format = textureFormat = FMT_RGBA8;
				img.Alloc( padSize * padSize * 4 );
				memcpy( img.data, pic, img.dataSize );
			}

			// downsample for the next level
			byte * shrunk = NULL;
			if ( gammaMips ) {
				shrunk = R_MipMapWithGamma( pic, scaledWidth, scaledWidth );
			} else {
				shrunk = R_MipMap( pic, scaledWidth, scaledWidth );
			}
			if ( pic != orig ) {
				Mem_Free( (void *)pic );
				pic = NULL;
			}
			pic = shrunk;

			scaledWidth = Max( 1, scaledWidth >> 1 );
		}
		if ( pic != orig ) {
			// free the down sampled version
			Mem_Free( (void *)pic );
			pic = NULL;
		}
	}
}

/*
========================
idBinaryImage::ConvertFormat
========================
*/
bool idBinaryImage::ConvertFormat( textureFormat_t desiredFormat ) {
	bool ret = false;
#ifdef GL_ES_VERSION_2_0
	if ( fileData.format != desiredFormat ) {
		if ( ( fileData.format == FMT_DXT1 || fileData.format == FMT_DXT5 ) && !glConfig.textureCompressionDXTAvailable ) {
			if ( desiredFormat == FMT_ETC1 || desiredFormat == FMT_ETC2_PUNCH || desiredFormat == FMT_ETC2_ALPHA || desiredFormat == FMT_RGBA8 ) {

				byte **newImgData = new (TAG_TEMP) byte *[images.Num()];

				// decode
				int level = 0;
				for ( ; level < images.Num(); level++ ) {
					idBinaryImageData &img = images[ level ];
					const byte * imgData = images[ level ].data;

					newImgData[level] = new (TAG_TEMP) byte[img.width * img.height * 4];

					if ( fileData.format == FMT_DXT1 ) {
						idDxtDecoder dxt;
						dxt.DecompressImageDXT1( imgData, newImgData[level], img.width, img.height );
					} else {
						if ( fileData.colorFormat == CFM_NORMAL_DXT5 ) {
							idDxtDecoder dxt;
							dxt.DecompressNormalMapDXT5( imgData, newImgData[level], img.width, img.height );
						} else if ( fileData.colorFormat == CFM_YCOCG_DXT5 ) {
							idDxtDecoder dxt;
							dxt.DecompressYCoCgDXT5( imgData, newImgData[level], img.width, img.height );
							idColorSpace::ConvertCoCg_YToRGB( newImgData[level], newImgData[level], img.width, img.height );
						} else {
							idDxtDecoder dxt;
							dxt.DecompressImageDXT5( imgData, newImgData[level], img.width, img.height );
						}
					}
				}
				// re-encode
				fileData.format = desiredFormat;
				for ( level = 0; level < images.Num(); level++ ) {
					idBinaryImageData &img = images[ level ];

					if ( desiredFormat == FMT_ETC1 ) {
						img.Alloc( img.width * img.height / 2 );
						idEtcEncoder etc;
						if ( image_highQualityCompression.GetBool() ) {
							etc.CompressImageETC1HQ( newImgData[level], img.data, img.width, img.height );
						} else {
							etc.CompressImageETC1Fast( newImgData[level], img.data, img.width, img.height );
						}
					} else if ( desiredFormat == FMT_ETC2_PUNCH ) {
						img.Alloc( img.width * img.height / 2 );
						idEtcEncoder etc;
						if ( image_highQualityCompression.GetBool() ) {
							etc.CompressImageETC2PunchAlphaHQ( newImgData[level], img.data, img.width, img.height );
						} else {
							etc.CompressImageETC2PunchAlphaFast( newImgData[level], img.data, img.width, img.height );
						}
					} else if ( desiredFormat == FMT_ETC2_ALPHA ) {
						img.Alloc( img.width * img.height );
						idEtcEncoder etc;
						if ( image_highQualityCompression.GetBool() ) {
							etc.CompressImageETC2AlphaHQ( newImgData[level], img.data, img.width, img.height );
						} else {
							etc.CompressImageETC2AlphaFast( newImgData[level], img.data, img.width, img.height );
						}
					} else {
						img.Alloc( img.width * img.height * 4 );
						memcpy( img.data, newImgData[level], img.width * img.height * 4 );
					}
					delete newImgData[level];
					newImgData[level] = NULL;
				}
				delete[] newImgData;
				ret = true;
			} else {
				idLib::Warning( "Image (%s) is not in a supported format, but the desired format was unexpected. Image may not load properly.\n", GetName() );
			}
		}
	} else {
		ret = true;
	}
#endif
	return ret;
}

/*
========================
idBinaryImage::OptimizeDesiredImageFormat2D
========================
*/
bool idBinaryImage::OptimizeDesiredImageFormat2D( int width, int height, const byte * pic_const, textureFormat_t & currentFormat, const textureColor_t & colorFormat ) {
#ifdef GL_ES_VERSION_2_0
	if ( colorFormat != CFM_NORMAL_DXT5 ) {
		if ( currentFormat != FMT_ETC1 && image_attemptETC1Encoding.GetBool() ) {
			if ( idEtcEncoder::CanEncodeAsETC1( pic_const, width, height ) ) {
				currentFormat = FMT_ETC1;
				return true;
			}
		} else if ( currentFormat != FMT_ETC2_PUNCH && image_attemptETC2PunchthroughEncoding.GetBool() ) {
			if ( idEtcEncoder::CanEncodeAsETC2_Punchthrough( pic_const, width, height ) ) {
				currentFormat = FMT_ETC2_PUNCH;
				return true;
			}
		}
	}
#endif
	return false;
}

/*
========================
idBinaryImage::OptimizeDesiredImageFormatCube
========================
*/
bool idBinaryImage::OptimizeDesiredImageFormatCube( int width, const byte * pics[6], textureFormat_t & currentFormat ) {
#ifdef GL_ES_VERSION_2_0
	if ( image_attemptETC1Encoding.GetBool() || image_attemptETC2PunchthroughEncoding.GetBool() ) {
		textureFormat_t format = currentFormat;
		textureFormat_t altFormat = FMT_NONE;
		for ( int i = 0; i < 6; i++ ) {
			if ( OptimizeDesiredImageFormat2D( width, width, pics[i], format, CFM_DEFAULT ) ) {
				if ( altFormat == FMT_NONE || format > altFormat ) {
					altFormat = format;
				}
				format = currentFormat;
			}
		}
		if ( altFormat != FMT_NONE ) {
			currentFormat = altFormat;
			return true;
		}
	}
#endif
	return false;
}

/*
========================
idBinaryImage::WriteGeneratedFile
========================
*/
ID_TIME_T idBinaryImage::WriteGeneratedFile( ID_TIME_T sourceFileTime ) {
	idStr binaryFileName;
	MakeGeneratedFileName( binaryFileName );
	idFileLocal file( fileSystem->OpenFileWrite( binaryFileName, "fs_basepath" ) );
	if ( file == NULL ) {
		idLib::Warning( "idBinaryImage: Could not open file '%s'", binaryFileName.c_str() );
		return FILE_NOT_FOUND_TIMESTAMP;
	}
	idLib::Printf( "Writing %s\n", binaryFileName.c_str() );

	fileData.headerMagic = BIMAGE_MAGIC;
	fileData.sourceFileTime = sourceFileTime;

	file->WriteBig( fileData.sourceFileTime );
	file->WriteBig( fileData.headerMagic );
	file->WriteBig( fileData.textureType );
	file->WriteBig( fileData.format );
	file->WriteBig( fileData.colorFormat );
	file->WriteBig( fileData.width );
	file->WriteBig( fileData.height );
	file->WriteBig( fileData.numLevels );

	for ( int i = 0; i < images.Num(); i++ ) {
		idBinaryImageData &img = images[ i ];
		file->WriteBig( img.level );
		file->WriteBig( img.destZ );
		file->WriteBig( img.width );
		file->WriteBig( img.height );
		file->WriteBig( img.dataSize );
		file->Write( img.data, img.dataSize );
	}
	return file->Timestamp();
}

/*
==========================
idBinaryImage::LoadFromGeneratedFile

Load the preprocessed image from the generated folder.
==========================
*/
ID_TIME_T idBinaryImage::LoadFromGeneratedFile( ID_TIME_T sourceFileTime ) {
	idStr binaryFileName;
	MakeGeneratedFileName( binaryFileName );
	idFileLocal bFile = fileSystem->OpenFileRead( binaryFileName );
	if ( bFile == NULL ) {
		return FILE_NOT_FOUND_TIMESTAMP;
	}
	if ( LoadFromGeneratedFile( bFile, sourceFileTime ) ) {
		return bFile->Timestamp();
	}
	return FILE_NOT_FOUND_TIMESTAMP;
}

/*
==========================
idBinaryImage::LoadFromGeneratedFile

Load the preprocessed image from the generated folder.
==========================
*/
bool idBinaryImage::LoadFromGeneratedFile( idFile * bFile, ID_TIME_T sourceFileTime ) {
	if ( bFile->Read( &fileData, sizeof( fileData ) ) <= 0 ) {
		return false;
	}
	idSwapClass<bimageFile_t> swap;
	swap.Big( fileData.sourceFileTime );
	swap.Big( fileData.headerMagic );
	swap.Big( fileData.textureType );
	swap.Big( fileData.format );
	swap.Big( fileData.colorFormat );
	swap.Big( fileData.width );
	swap.Big( fileData.height );
	swap.Big( fileData.numLevels );

	if ( BIMAGE_MAGIC != fileData.headerMagic ) {
		return false;
	}
	if ( fileData.sourceFileTime != sourceFileTime && !fileSystem->InProductionMode() ) {
		return false;
	}

	int numImages = fileData.numLevels;
	if ( fileData.textureType == TT_CUBIC ) {
		numImages *= 6;
	}

	images.SetNum( numImages );

	for ( int i = 0; i < numImages; i++ ) {
		idBinaryImageData &img = images[ i ];
		if ( bFile->Read( &img, sizeof( bimageImage_t ) ) <= 0 ) {
			return false;
		}
		idSwapClass<bimageImage_t> swap;
		swap.Big( img.level );
		swap.Big( img.destZ );
		swap.Big( img.width );
		swap.Big( img.height );
		swap.Big( img.dataSize );
		assert( img.level >= 0 && img.level < fileData.numLevels );
		assert( img.destZ == 0 || fileData.textureType == TT_CUBIC );
		assert( img.dataSize > 0 );
		// DXT images need to be padded to 4x4 block sizes, but the original image
		// sizes are still retained, so the stored data size may be larger than
		// just the multiplication of dimensions
		assert( img.dataSize >= img.width * img.height * BitsForFormat( (textureFormat_t)fileData.format ) / 8 );
		img.Alloc( img.dataSize );
		if ( img.data == NULL ) {
			return false;
		}

		if ( bFile->Read( img.data, img.dataSize ) <= 0 ) {
			return false;
		}
	}

	return true;
}

/*
==========================
idBinaryImage::MakeGeneratedFileName
==========================
*/
void idBinaryImage::MakeGeneratedFileName( idStr & gfn ) {
	GetGeneratedFileName( gfn, GetName() );
}

/*
==========================
idBinaryImage::GetGeneratedFileName
==========================
*/
void idBinaryImage::GetGeneratedFileName( idStr & gfn, const char *name ) {
	gfn.Format( "generated/images/%s.bimage", name );
	gfn.Replace( "(", "/" );
	gfn.Replace( ",", "/" );
	gfn.Replace( ")", "" );
	gfn.Replace( " ", "" );
}
