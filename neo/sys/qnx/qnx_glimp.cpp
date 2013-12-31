/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.
Copyright (C) 2013 Vincent Simonetti

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
#include "../../idlib/precompiled.h"

#include "qnx_local.h"
#include <sys/syspage.h>
#include "../../renderer/tr_local.h"
#include <EGL/eglext.h>

idCVar r_useGLES3( "r_useGLES3", "0", CVAR_RENDERER | CVAR_ARCHIVE | CVAR_INTEGER, "0 = OpenGL ES 3.0 if available, 1 = OpenGL ES 2.0, 2 = OpenGL 3.0", 0, 2 );

//
// function replacement
//

void ( APIENTRY * qglReadPixels_real )(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid* pixels);
void ( APIENTRY * qglCopyTexImage2D_real )(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
void ( APIENTRY * qglCopyTexSubImage2D_real )(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
void ( APIENTRY * qglCopyTexSubImage3D_real )(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);

// If in blit mode, produces a GL_INVALID_OPERATION error. Otherwise, switches to the correct read buffer if needed, executes the function, then switches back to the prior draw buffer
#define GL_READ_REPLACEMENT( x ) \
	if ( qnx.useBlit ) { \
		qglTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL ); \
	} else { \
		if ( qnx.readBuffer != qnx.drawBuffer ) { \
			int index = R_GetBufferIndex( qnx.readBuffer ); \
			qglBindFramebuffer( GL_FRAMEBUFFER, ( index == INT_MAX ? 0 : qnx.framebuffers[index] ) ); \
		} \
		x; \
		if ( qnx.readBuffer != qnx.drawBuffer ) { \
			int index = R_GetBufferIndex( qnx.drawBuffer ); \
			qglBindFramebuffer( GL_FRAMEBUFFER, ( index == INT_MAX ? 0 : qnx.framebuffers[index] ) ); \
		} \
	}

/*
=================
R_GetBufferIndex
=================
*/
int R_GetBufferIndex( GLenum value ) {
	switch ( value ) {
	case GL_FRONT:
	//case GL_LEFT:
	case GL_FRONT_LEFT:
		return INT_MAX;

	//case GL_RIGHT:
	case GL_FRONT_RIGHT:
		return 0;

	case GL_BACK:
	case GL_BACK_LEFT:
		return 1;

	case GL_BACK_RIGHT:
		return 2;
	}
	return -1;
}

/*
========================
GLimp_glReadBuffer
========================
*/
void GLimp_glReadBuffer( GLenum mode ) {
	if ( qnx.readBuffer != mode ) {
		int index = R_GetBufferIndex( mode );
		if ( index >= 0 ) {
			if ( qnx.useBlit ) {
				qglBindFramebuffer( GL_READ_FRAMEBUFFER_ANGLE, ( index == INT_MAX ? 0 : qnx.framebuffers[index] ) );
			}
			qnx.readBuffer = mode;
		} else {
			// Cause GL_INVALID_ENUM error
			GLint tmp;
			qglGetFramebufferAttachmentParameteriv( GL_RENDERBUFFER, GL_COLOR_ATTACHMENT0, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &tmp );
		}
	}
}

/*
========================
GLimp_glDrawBuffer
========================
*/
void GLimp_glDrawBuffer( GLenum mode ) {
	// Doesn't follow standard OpenGL call specification
	if ( qnx.drawBuffer != mode ) {
		int index = R_GetBufferIndex( mode );
		if ( index >= 0 ) {
			qglBindFramebuffer( ( qnx.useBlit ? GL_DRAW_FRAMEBUFFER_ANGLE : GL_FRAMEBUFFER ), ( index == INT_MAX ? 0 : qnx.framebuffers[index] ) );
			qnx.drawBuffer = mode;
		} else {
			// Cause GL_INVALID_ENUM error
			GLint tmp;
			qglGetFramebufferAttachmentParameteriv( GL_RENDERBUFFER, GL_COLOR_ATTACHMENT0, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &tmp );
		}
	}
}

/*
========================
GLimp_glReadPixels
========================
*/
void GLimp_glReadPixels( GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid* pixels ) {
	GL_READ_REPLACEMENT( qglReadPixels_real( x, y, width, height, format, type, pixels ) )
}

/*
========================
GLimp_glCopyTexImage2D
========================
*/
void GLimp_glCopyTexImage2D( GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border ) {
	GL_READ_REPLACEMENT( qglCopyTexImage2D_real( target, level, internalformat, x, y, width, height, border ) )
}

/*
========================
GLimp_glCopyTexSubImage2D
========================
*/
void GLimp_glCopyTexSubImage2D( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height ) {
	GL_READ_REPLACEMENT( qglCopyTexSubImage2D_real( target, level, xoffset, yoffset, x, y, width, height ) )
}

#ifdef GL_ES_VERSION_3_0

/*
========================
GLimp_glCopyTexSubImage3D
========================
*/
void GLimp_glCopyTexSubImage3D( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height ) {
	GL_READ_REPLACEMENT( qglCopyTexSubImage3D_real( target, level, xoffset, yoffset, zoffset, x, y, width, height ) )
}

#endif

//
// function declaration
//
bool QGL_Init( const char *dllname, const EGLFunctionReplacements_t & replacements );
void* QGL_GetSym( const char *function, bool egl );
void QGL_Shutdown();
void OGL_UpdateReplacements( const EGLFunctionReplacements_t & replacements );
bool R_CheckExtension( char *name );

PFNGLBLITFRAMEBUFFERANGLEPROC					qglBlitFramebufferANGLE;
PFNGLRENDERBUFFERSTORAGEMULTISAMPLEANGLEPROC	qglRenderbufferStorageMultisampleANGLE;
PFNGLFRAMEBUFFERTEXTURE2DMULTISAMPLEIMGPROC		qglFramebufferTexture2DMultisampleIMG;

#ifdef USE_GLES_MULTISAMPLE_FRAMEBUFFER
#define GL_RENDERBUFFER_STORAGE( samples, format, width, height ) qglRenderbufferStorageMultisampleANGLE( GL_RENDERBUFFER, ( samples ), ( format ), ( width ), ( height ) )
#else
#define GL_RENDERBUFFER_STORAGE( samples, format, width, height ) qglRenderbufferStorage( GL_RENDERBUFFER, ( format ), ( width ), ( height ) )
#endif

#ifdef USE_GLES_MULTISAMPLE_FRAMEBUFFER
#define GL_FRAMEBUFFER_TEXTURE2D( textureID, level, samples ) qglFramebufferTexture2DMultisampleIMG( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ( textureID ), ( level ), ( samples ) )
#else
#define GL_FRAMEBUFFER_TEXTURE2D( textureID, level, samples ) qglFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ( textureID ), ( level ) )
#endif

PFNGLDISCARDFRAMEBUFFEREXTPROC					qglDiscardFramebufferEXT;

/*
========================
GLimp_TestSwapBuffers
========================
*/
void GLimp_TestSwapBuffers( const idCmdArgs &args ) {
	idLib::Printf( "GLimp_TimeSwapBuffers\n" );
	static const int MAX_FRAMES = 5;
	uint64	timestamps[MAX_FRAMES];
	qglDisable( GL_SCISSOR_TEST );

	for ( int swapInterval = 2 ; swapInterval >= 0 ; swapInterval-- ) {
		qeglSwapInterval( qnx.eglDisplay, swapInterval );
		for ( int i = 0 ; i < MAX_FRAMES ; i++ ) {
			if ( i & 1 ) {
				qglClearColor( 0, 1, 0, 1 );
			} else {
				qglClearColor( 1, 0, 0, 1 );
			}
			qglClear( GL_COLOR_BUFFER_BIT );
			qeglSwapBuffers( qnx.eglDisplay, qnx.eglSurface );
			qglFinish();
			timestamps[i] = Sys_Microseconds();
		}

		idLib::Printf( "\nswapinterval %i\n", swapInterval );
		for ( int i = 1 ; i < MAX_FRAMES ; i++ ) {
			idLib::Printf( "%i microseconds\n", (int)(timestamps[i] - timestamps[i-1]) );
		}
	}
}

/*
========================
GLimp_SetGamma

The renderer calls this when the user adjusts r_gamma or r_brightness
========================
*/
void GLimp_SetGamma( unsigned short red[256], unsigned short green[256], unsigned short blue[256] ) {
	// XXX
	// The proper thing to do would be to change screen gamma and brightness by inverse calculation of gamma values... but when OpenGL is in use, those values seem to get
	// ignored by QNX. So it's not worth doing the calculation.

	// Note: Below is calculation and ranges (actual code is in RenderSystem_init's R_SetColorMappings). If the calculation is ever implemented and done,
	// and an error occurs, state which function failed (win32 uses SetDeviceGammaRamp, so the error is "common->Printf( "WARNING: SetDeviceGammaRamp failed.\n" );").

	/*
	gammaTable = 0xFFFF * ((j/255)^(1/gamma) + 0.5)

	j_min=0-128
	j_max=0-512

	gamma_min=0.5
	gamma_max=3

	gammaTable_gmin_jmin=0-0x4081
	gammaTable_gmin_jmax=0-0x4080D => 0-0xFFFF

	gammaTable_gmax_jmin=0-0xCB74
	gammaTable_gmax_jmax=0-0x142F6 => 0-0xFFFF
	 */
}

/*
====================
PrintDisplayModeFlags
====================
*/
static void PrintDisplayModeFlags( unsigned int flags ) {
	idStr flagString;
	if ( flags != SCREEN_MODE_PREFERRED ) {
		flagString.Format("0x%X", flags);
	} else {
		flagString = "Preferred";
	}
	common->Printf( "          flags       : %s\n", flagString.c_str() );
}

/*
====================
PrintDisplayMode
====================
*/
static void PrintDisplayMode( screen_display_mode_t & mode, bool rotate ) {
	//common->Printf( "          display index: %i\n", mode.index );
	common->Printf( "          width       : %i\n", ( rotate ? mode.height : mode.width ) );
	common->Printf( "          height      : %i\n", ( rotate ? mode.width : mode.height ) );
	common->Printf( "          refresh     : %i\n", mode.refresh );
	common->Printf( "          interlaced  : 0x%x\n", mode.interlaced ); //XXX Can this be broken down to what interlaced mode it's in?
	common->Printf( "          aspect_ratio: %ix%i\n", mode.aspect_ratio[0], mode.aspect_ratio[1] );
	PrintDisplayModeFlags( mode.flags );
}

/*
====================
PrintDisplayType
====================
*/
static void PrintDisplayType( int type ) {
	const char * dispType;
	switch ( type ) {
	case SCREEN_DISPLAY_TYPE_INTERNAL:			dispType = "Internal"; break;
	case SCREEN_DISPLAY_TYPE_COMPOSITE:			dispType = "Composite"; break;
	case SCREEN_DISPLAY_TYPE_SVIDEO:			dispType = "S-Video"; break;
	case SCREEN_DISPLAY_TYPE_COMPONENT_YPbPr:	dispType = "Component (YPbPr)"; break;
	case SCREEN_DISPLAY_TYPE_COMPONENT_RGB:		dispType = "Component (RGB)"; break;
	case SCREEN_DISPLAY_TYPE_COMPONENT_RGBHV:	dispType = "Component (RGBHV)"; break;
	case SCREEN_DISPLAY_TYPE_DVI:				dispType = "DVI"; break;
	case SCREEN_DISPLAY_TYPE_HDMI:				dispType = "HDMI"; break;
	case SCREEN_DISPLAY_TYPE_DISPLAYPORT:		dispType = "DisplayPort"; break;
	case SCREEN_DISPLAY_TYPE_OTHER:				dispType = "Other"; break;
	default:									dispType = "Unknown"; break;
	}
	common->Printf( "      Type              : %s\n", dispType );
}

/*
====================
PrintDisplayType
====================
*/
static void PrintDisplayTechnology( int tech ) {
	const char * dispTech;
	switch ( tech ) {
	case SCREEN_DISPLAY_TECHNOLOGY_CRT:		dispTech = "CRT"; break;
	case SCREEN_DISPLAY_TECHNOLOGY_LCD:		dispTech = "LCD"; break;
	case SCREEN_DISPLAY_TECHNOLOGY_PLASMA:	dispTech = "Plasma"; break;
	case SCREEN_DISPLAY_TECHNOLOGY_LED:		dispTech = "LED"; break;
	case SCREEN_DISPLAY_TECHNOLOGY_OLED:	dispTech = "OLED"; break;
	case SCREEN_DISPLAY_TECHNOLOGY_UNKNOWN:
	default:								dispTech = "Unknown"; break;
	}
	common->Printf( "      Technology        : %s\n", dispTech );
}

/*
====================
PrintDisplayTransparency
====================
*/
static void PrintDisplayTransparency( int trans ) {
	const char * transTech;
	switch ( trans ) {
	case SCREEN_TRANSPARENCY_SOURCE:		transTech = "Source"; break;
	case SCREEN_TRANSPARENCY_TEST:			transTech = "Test"; break;
	case SCREEN_TRANSPARENCY_SOURCE_COLOR:	transTech = "Source Color"; break;
	case SCREEN_TRANSPARENCY_SOURCE_OVER:	transTech = "Source Over"; break;
	case SCREEN_TRANSPARENCY_NONE:			transTech = "None"; break;
	case SCREEN_TRANSPARENCY_DISCARD:		transTech = "Discard"; break;
	default:								transTech = "Unknown"; break;
	}
	common->Printf( "      Transparency      : %s\n", transTech );
}

/*
====================
PrintDisplayPowerMode
====================
*/
static void PrintDisplayPowerMode( int power ) {
	const char * powerTech;
	switch ( power ) {
	case SCREEN_POWER_MODE_OFF:			powerTech = "Off"; break;
	case SCREEN_POWER_MODE_SUSPEND:		powerTech = "Suspend"; break;
	case SCREEN_POWER_MODE_LIMITED_USE:	powerTech = "Limited Use"; break;
	case SCREEN_POWER_MODE_ON:			powerTech = "On"; break;
	default:							powerTech = "Unknown"; break;
	}
	common->Printf( "      Power Mode        : %s\n", powerTech );
}

/*
====================
DumpAllDisplayDevices
====================
*/
void DumpAllDisplayDevices() {
	common->Printf( "\n" );

	int displayCount = 1;
	if ( screen_get_context_property_iv( qnx.screenCtx, SCREEN_PROPERTY_DISPLAY_COUNT, &displayCount ) != 0 ) {
		common->Printf( "ERROR:  screen_get_context_property_iv(..., SCREEN_PROPERTY_DISPLAY_COUNT, ...) failed!\n\n" );
		return;
	}

	screen_display_t* displayList = ( screen_display_t* )Mem_Alloc( sizeof( screen_display_t ) * displayCount, TAG_TEMP );
	if ( screen_get_context_property_pv( qnx.screenCtx, SCREEN_PROPERTY_DISPLAYS, ( void** )displayList ) != 0 ) {
		Mem_Free( displayList );
		common->Printf( "ERROR:  screen_get_context_property_pv(..., SCREEN_PROPERTY_DISPLAYS, ...) failed!\n\n" );
		return;
	}

	int rotation = atoi( getenv( "ORIENTATION" ) );

	screen_display_t display;
	screen_display_mode_t * modes;
	char buffer[1024];
	int vals[2];
	int nativeRes[2];
	int64 longVal;
	for ( int displayNum = 0; displayNum < displayCount; displayNum++ ) {
		display = displayList[displayNum];

		// General display stats
		common->Printf( "display device: %i\n", displayNum );
		screen_get_display_property_cv( display, SCREEN_PROPERTY_ID_STRING, sizeof(buffer) - 1, buffer );
		common->Printf( "  ID     : %s\n", buffer );
		screen_get_display_property_cv( display, SCREEN_PROPERTY_VENDOR, sizeof(buffer) - 1, buffer );
		common->Printf( "  Vendor : %s\n", buffer );
		screen_get_display_property_cv( display, SCREEN_PROPERTY_PRODUCT, sizeof(buffer) - 1, buffer );
		common->Printf( "  Product: %s\n", buffer );

		// Detailed display stats
		screen_get_display_property_iv( display, SCREEN_PROPERTY_TYPE, vals );
		PrintDisplayType( vals[0] );
		screen_get_display_property_iv( display, SCREEN_PROPERTY_TECHNOLOGY, vals );
		PrintDisplayTechnology( vals[0] );
		screen_get_display_property_iv( display, SCREEN_PROPERTY_ATTACHED, vals );
		common->Printf( "      Attached          : %s\n", ( vals[0] ? "True" : "False" ) );
		screen_get_display_property_iv( display, SCREEN_PROPERTY_DETACHABLE, vals );
		common->Printf( "      Detachable        : %s\n", ( vals[0] ? "True" : "False" ) );
		screen_get_display_property_iv( display, SCREEN_PROPERTY_ROTATION, vals );
		common->Printf( "      Rotation          : %i\n", vals[0] );
		screen_get_display_property_iv( display, SCREEN_PROPERTY_DPI, vals );
		common->Printf( "      DPI               : %i\n", vals[0] );
		screen_get_display_property_iv( display, SCREEN_PROPERTY_NATIVE_RESOLUTION, nativeRes );
		common->Printf( "      Native Resolution : %ix%i\n", nativeRes[0], nativeRes[1] );
		screen_get_display_property_iv( display, SCREEN_PROPERTY_PHYSICAL_SIZE, vals );
		common->Printf( "      Physical Size     : %ix%i\n", vals[0], vals[1] );
		screen_get_display_property_iv( display, SCREEN_PROPERTY_SIZE, vals );
		common->Printf( "      Size              : %ix%i\n", vals[0], vals[1] );
		screen_get_display_property_iv( display, SCREEN_PROPERTY_TRANSPARENCY, vals );
		PrintDisplayTransparency( vals[0] );
		screen_get_display_property_iv( display, SCREEN_PROPERTY_GAMMA, vals );
		common->Printf( "      Gamma             : %i\n", vals[0] );
		screen_get_display_property_iv( display, SCREEN_PROPERTY_INTENSITY, vals );
		common->Printf( "      Intensity         : %i\n", vals[0] );
		screen_get_display_property_iv( display, SCREEN_PROPERTY_IDLE_STATE, vals );
		common->Printf( "      Idle              : %s\n", ( vals[0] ? "True" : "False" ) );
		screen_get_display_property_llv( display, SCREEN_PROPERTY_IDLE_TIMEOUT, &longVal );
		common->Printf( "      Idle Timeout      : %lld\n", longVal );
		screen_get_display_property_iv( display, SCREEN_PROPERTY_KEEP_AWAKES, vals );
		common->Printf( "      Keep Awake        : %s\n", ( vals[0] ? "True" : "False" ) );
		screen_get_display_property_iv( display, SCREEN_PROPERTY_MIRROR_MODE, vals );
		common->Printf( "      Mirror Mode       : %s\n", ( vals[0] ? "True" : "False" ) );
		screen_get_display_property_iv( display, SCREEN_PROPERTY_POWER_MODE, vals );
		PrintDisplayPowerMode( vals[0] );
		screen_get_display_property_iv( display, SCREEN_PROPERTY_PROTECTION_ENABLE, vals );
		common->Printf( "      Protection Enabled: %s\n", ( vals[0] ? "True" : "False" ) );

		// Modes
		screen_get_display_property_iv( display, SCREEN_PROPERTY_MODE_COUNT, vals );
		modes = ( screen_display_mode_t* )Mem_Alloc( sizeof( screen_display_mode_t ) * vals[0], TAG_TEMP );
		if ( screen_get_display_modes( display, vals[0], modes ) == 0 ) {
			for ( int mode = 0; mode < vals[0]; mode++ ) {
				screen_display_mode_t & displayMode = modes[mode];

				if ( displayMode.refresh < 30 ) {
					continue;
				}

				bool rotate = false;
				int width = displayMode.width;
				int height = displayMode.height;
				if ( displayNum == 0 ) {
					// May have to rotate display based on device orientation
					if ( ( rotation == 0 ) || ( rotation == 180 ) ) {
						rotate = ( ( displayMode.width > displayMode.height ) && ( nativeRes[0] < nativeRes[1] ) ) ||
								( ( displayMode.width < displayMode.height ) && ( nativeRes[0] > nativeRes[1] ) );
					} else if ( ( rotation == 90 ) || ( rotation == 270 ) ) {
						rotate = ( ( displayMode.width > displayMode.height ) && ( nativeRes[0] > nativeRes[1] ) ) ||
								( ( displayMode.width < displayMode.height ) && ( nativeRes[0] < nativeRes[1] ) );
					}
				}
				if ( rotate ) {
					int tmp = width;
					width = height;
					height = tmp;
				}

				if ( height < 720 ) {
					continue;
				}

				common->Printf( "          -------------------\n" );
				common->Printf( "          modeNum     : %i\n", mode );
				PrintDisplayMode( displayMode, rotate );
			}
		}
		Mem_Free( ( void* )modes );
	}

	Mem_Free( ( void* )displayList );

	common->Printf( "\n" );
}

class idSort_VidMode : public idSort_Quick< vidMode_t, idSort_VidMode > {
public:
	int Compare( const vidMode_t & a, const vidMode_t & b ) const {
		int wd = a.width - b.width;
		int hd = a.height - b.height;
		int fd = a.displayHz - b.displayHz;
		return ( hd != 0 ) ? hd : ( wd != 0 ) ? wd : fd;
	}
};

/*
====================
R_GetModeListForDisplay
====================
*/
bool R_GetModeListForDisplay( const int requestedDisplayNum, idList<vidMode_t> & modeList ) {
#define CLEANUP_TMP_CONTEXT() \
	if ( !hasContext ) { \
		screen_destroy_context( qnx.screenCtx ); \
		qnx.screenCtx = NULL; \
	}

	modeList.Clear();

	bool	verbose = false;
	bool	hasContext = qnx.screenCtx != NULL;

	int rotation = atoi( getenv( "ORIENTATION" ) );
	if ( requestedDisplayNum == 0 &&
			!( rotation == 0 || rotation == 90 || rotation == 180 || rotation == 270 ) ) {
		return false;
	}

	if ( !hasContext ) {
		// Temporarily create a context
		if ( screen_create_context( &qnx.screenCtx, 0 ) != 0 ) {
			return false;
		}
	}

	int displayCount = 1;
	if ( screen_get_context_property_iv( qnx.screenCtx, SCREEN_PROPERTY_DISPLAY_COUNT, &displayCount ) != 0 ) {
		CLEANUP_TMP_CONTEXT()
		return false;
	}

	if ( requestedDisplayNum >= displayCount ) {
		CLEANUP_TMP_CONTEXT()
		return false;
	}

	screen_display_t* displayList = ( screen_display_t* )Mem_Alloc( sizeof( screen_display_t ) * displayCount, TAG_TEMP );
	if ( screen_get_context_property_pv( qnx.screenCtx, SCREEN_PROPERTY_DISPLAYS, ( void** )displayList ) != 0 ) {
		Mem_Free( displayList );
		CLEANUP_TMP_CONTEXT()
		return false;
	}

	screen_display_t display;
	char buffer[1024];
	int vals[2];
	int nativeRes[2];
	screen_display_mode_t * modes;
	for ( int displayNum = requestedDisplayNum; ; displayNum++ ) {
		if ( displayNum >= displayCount ) {
			Mem_Free( displayList );
			CLEANUP_TMP_CONTEXT()
			return false;
		}

		display = displayList[displayNum];

		// Only get displays that are still attached
		if ( screen_get_display_property_iv( display, SCREEN_PROPERTY_ATTACHED, vals ) != 0 || !vals[0] ) {
			continue;
		}

		screen_get_display_property_iv( display, SCREEN_PROPERTY_NATIVE_RESOLUTION, nativeRes );

		if ( verbose ) {
			common->Printf( "display device: %i\n", displayNum );
			screen_get_display_property_cv( display, SCREEN_PROPERTY_ID_STRING, sizeof(buffer) - 1, buffer );
			common->Printf( "  ID     : %s\n", buffer );
			screen_get_display_property_cv( display, SCREEN_PROPERTY_VENDOR, sizeof(buffer) - 1, buffer );
			common->Printf( "  Vendor : %s\n", buffer );
			screen_get_display_property_cv( display, SCREEN_PROPERTY_PRODUCT, sizeof(buffer) - 1, buffer );
			common->Printf( "  Product: %s\n", buffer );
			screen_get_display_property_iv( display, SCREEN_PROPERTY_TYPE, vals );
			PrintDisplayType( vals[0] );
			screen_get_display_property_iv( display, SCREEN_PROPERTY_TECHNOLOGY, vals );
			PrintDisplayTechnology( vals[0] );
			screen_get_display_property_iv( display, SCREEN_PROPERTY_ROTATION, vals );
			common->Printf( "      Rotation          : %i\n", vals[0] );
			screen_get_display_property_iv( display, SCREEN_PROPERTY_DPI, vals );
			common->Printf( "      DPI               : %i\n", vals[0] );
			common->Printf( "      Native Resolution : %ix%i\n", nativeRes[0], nativeRes[1] );
			screen_get_display_property_iv( display, SCREEN_PROPERTY_PHYSICAL_SIZE, vals );
			common->Printf( "      Physical Size     : %ix%i\n", vals[0], vals[1] );
		}

		screen_get_display_property_iv( display, SCREEN_PROPERTY_MODE_COUNT, vals );
		modes = ( screen_display_mode_t* )Mem_Alloc( sizeof( screen_display_mode_t ) * vals[0], TAG_TEMP );
		if ( screen_get_display_modes( display, vals[0], modes ) == 0 ) {
			for ( int mode = 0; mode < vals[0]; mode++ ) {
				screen_display_mode_t & displayMode = modes[mode];

				// The internal display lists it's refresh rate at 59...
				if ( !( displayMode.refresh == 59 || displayMode.refresh == 60 || displayMode.refresh == 120 ) ) {
					continue;
				}

				bool rotate = false;
				int width = displayMode.width;
				int height = displayMode.height;
				if ( displayNum == 0 ) {
					// May have to rotate display based on device orientation
					if ( ( rotation == 0 ) || ( rotation == 180 ) ) {
						rotate = ( ( displayMode.width > displayMode.height ) && ( nativeRes[0] < nativeRes[1] ) ) ||
								( ( displayMode.width < displayMode.height ) && ( nativeRes[0] > nativeRes[1] ) );
					} else if ( ( rotation == 90 ) || ( rotation == 270 ) ) {
						rotate = ( ( displayMode.width > displayMode.height ) && ( nativeRes[0] > nativeRes[1] ) ) ||
								( ( displayMode.width < displayMode.height ) && ( nativeRes[0] < nativeRes[1] ) );
					}
				}
				if ( rotate ) {
					int tmp = width;
					width = height;
					height = tmp;
				}

				if ( height < 720 ) {
					continue;
				}

				if ( verbose ) {
					common->Printf( "          -------------------\n" );
					common->Printf( "          modeNum     : %i\n", mode );
					PrintDisplayMode( displayMode, rotate );
				}

				vidMode_t mode;
				mode.width = width;
				mode.height = height;
				mode.displayHz = displayMode.refresh;
				modeList.AddUnique( mode );
			}
		}
		Mem_Free( ( void* )modes );

		if ( modeList.Num() > 0 ) {
			break;
		}
	}

	Mem_Free( ( void* )displayList );

	CLEANUP_TMP_CONTEXT()

	if ( modeList.Num() > 0 ) {

		// sort with lowest resolution first
		modeList.SortWithTemplate( idSort_VidMode() );

		return true;
	}

	return false;

#undef CLEANUP_TMP_CONTEXT
}

/*
=================
R_BuildFramebuffers
=================
*/
void R_BuildFramebuffers( bool useSRGB, GLsizei samples, int renderbufferOffset ) {
	for ( int i = 0; i < FRAMEBUFFER_COUNT; i++ ) {
		qglBindFramebuffer( GL_FRAMEBUFFER, qnx.framebuffers[i] );

		// color
		if ( qnx.useBlit ) {
			qglBindRenderbuffer( GL_RENDERBUFFER, qnx.renderbuffers[i * renderbufferOffset + 0] );
			GL_RENDERBUFFER_STORAGE( samples, ( useSRGB ? GL_SRGB8_ALPHA8_EXT : GL_RGBA8_FB ), glConfig.nativeScreenWidth, glConfig.nativeScreenHeight );
			qglFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, qnx.renderbuffers[i * renderbufferOffset + 0] );
		} else {
			qglBindTexture( GL_TEXTURE_2D, qnx.fbTextures[i] );

			qglTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
			qglTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
			qglTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
			qglTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );

#ifdef GL_ES_VERSION_3_0
			GLenum internalFormat = ( useSRGB ? ( glConfig.glVersion >= 3.0f ? GL_SRGB8_ALPHA8 : GL_SRGB_ALPHA_EXT ) : GL_RGBA8 );
#else
			GLenum internalFormat = ( useSRGB ? GL_SRGB_ALPHA_EXT : GL_RGBA8 );
#endif
			qglTexImage2D( GL_TEXTURE_2D, 0, internalFormat, glConfig.nativeScreenWidth, glConfig.nativeScreenHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL );

			GL_FRAMEBUFFER_TEXTURE2D( qnx.fbTextures[i], 0, samples );
		}

		if ( qnx.packedFramebufferSupported ) {
			// Depth and stencil
			qglBindRenderbuffer( GL_RENDERBUFFER, qnx.renderbuffers[i * renderbufferOffset + 1] );
			GL_RENDERBUFFER_STORAGE( samples, GL_DEPTH24_STENCIL8_OES, glConfig.nativeScreenWidth, glConfig.nativeScreenHeight );
			qglFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, qnx.renderbuffers[i * renderbufferOffset + 1] );
			qglFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, qnx.renderbuffers[i * renderbufferOffset + 1] );
		} else {
			// Depth
			qglBindRenderbuffer( GL_RENDERBUFFER, qnx.renderbuffers[i * renderbufferOffset + 1] );
			GL_RENDERBUFFER_STORAGE( samples, GL_DEPTH_COMPONENT24_OES, glConfig.nativeScreenWidth, glConfig.nativeScreenHeight );
			qglFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, qnx.renderbuffers[i * renderbufferOffset + 1] );

			// Stencil
			qglBindRenderbuffer( GL_RENDERBUFFER, qnx.renderbuffers[i * renderbufferOffset + 2] );
			GL_RENDERBUFFER_STORAGE( samples, GL_STENCIL_INDEX8, glConfig.nativeScreenWidth, glConfig.nativeScreenHeight );
			qglFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, qnx.renderbuffers[i * renderbufferOffset + 2] );
		}

		// check for completeness
		GLenum status = qglCheckFramebufferStatus( GL_FRAMEBUFFER );
		if ( status != GL_FRAMEBUFFER_COMPLETE ) {
			const char *error;
			switch ( status ) {
			case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
				error = "Incomplete Attachment";
				break;
			case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS:
				error = "Incomplete Dimensions";
				break;
			case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
				error = "Missing Attachment";
				break;
			case GL_FRAMEBUFFER_UNSUPPORTED:
				error = "Unsupported";
				break;
			case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE_ANGLE:
			case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE_IMG:
				error = "Incomplete Multisample";
				break;
			default:
				error = "Unknown status";
				break;
			}
			idLib::Error( "Framebuffer status isn't complete: %s", error );
		}
	}
}

/*
=================
R_UpdateFramebuffers
=================
*/
void R_UpdateFramebuffers() {
	extern idCVar r_useSRGB;
	bool useSRGB = glConfig.sRGBFramebufferAvailable && ( r_useSRGB.GetInteger() == 1 || r_useSRGB.GetInteger() == 2 );
	GLsizei samples = r_multiSamples.GetInteger();
	samples = __min( __max( samples, 0 ), glConfig.maxMultisamples );

	GLint tmp;
	if ( qnx.useBlit ) {
		qglBindRenderbuffer( GL_RENDERBUFFER, qnx.renderbuffers[0] ); // First renderbuffer is a color one
#ifdef USE_GLES_MULTISAMPLE_FRAMEBUFFER
		qglGetRenderbufferParameteriv( GL_RENDERBUFFER, GL_RENDERBUFFER_SAMPLES_ANGLE, &tmp );
		if ( samples == tmp )
#endif
		{
			qglGetRenderbufferParameteriv( GL_RENDERBUFFER, GL_RENDERBUFFER_INTERNAL_FORMAT, &tmp );
			if ( !glConfig.sRGBFramebufferAvailable || tmp == GL_SRGB8_ALPHA8_EXT ) {
				// Nothing to do, all values are the same
				return;
			}
		}
	}
#ifdef USE_GLES_MULTISAMPLE_FRAMEBUFFER
	// There is no way to get texture format on PVR (can get it on QCOM...), so only check texture samples. If they are the same, and sRGB isn't supported
	// then return as format won't change. Otherwise, if the samples are different or sRGB is supported then we regenerate everything.
	else {
		qglGetFramebufferAttachmentParameteriv( GL_RENDERBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_SAMPLES_IMG, &tmp );
		if ( samples == tmp && !glConfig.sRGBFramebufferAvailable )
		{
			return;
		}
	}
#endif

	// According to http://www.opengl.org/wiki/Renderbuffer_Object, it is better to delete the renderbuffer and recreate it then it is to reallocate it
	int renderbufferOffset = qnx.packedFramebufferSupported ? RENDERBUFFER_PER_FRAMEBUFFER_PACKED : RENDERBUFFER_PER_FRAMEBUFFER;
	qglDeleteRenderbuffers( FRAMEBUFFER_COUNT * renderbufferOffset, qnx.renderbuffers );
	qglGenRenderbuffers( FRAMEBUFFER_COUNT * renderbufferOffset, qnx.renderbuffers );
	if ( !qnx.useBlit ) {
		qglDeleteTextures( FRAMEBUFFER_COUNT, qnx.fbTextures );
		qglGenTextures( FRAMEBUFFER_COUNT, qnx.fbTextures );
	}

	R_BuildFramebuffers( useSRGB, samples, renderbufferOffset );
}

/*
=================
R_SetupFramebuffers
=================
*/
void R_SetupFramebuffers() {

	// gen framebuffers
	qglGenFramebuffers( FRAMEBUFFER_COUNT, qnx.framebuffers );

	// setup parameters
	qnx.readBuffer = GL_FRONT;
	qnx.drawBuffer = GL_FRONT;

	// quick extension check
	qnx.discardFramebuffersSupported = R_CheckExtension( "GL_EXT_discard_framebuffer" );
	if ( qnx.discardFramebuffersSupported ) {
		qglDiscardFramebufferEXT = (PFNGLDISCARDFRAMEBUFFEREXTPROC)GLimp_ExtensionPointer( "glDiscardFramebufferEXT" );
	}

	qnx.blitSupported = ( glConfig.glVersion >= 3.0f || R_CheckExtension( "GL_ANGLE_framebuffer_blit" ) );
	if ( qnx.blitSupported ) {
		if ( glConfig.glVersion >= 3.0f ) {
			qglBlitFramebufferANGLE = (PFNGLBLITFRAMEBUFFERANGLEPROC)GLimp_ExtensionPointer( "glBlitFramebuffer" );
		} else {
			qglBlitFramebufferANGLE = (PFNGLBLITFRAMEBUFFERANGLEPROC)GLimp_ExtensionPointer( "glBlitFramebufferANGLE" );
		}
	}

	qnx.multisampleFramebufferSupported = ( glConfig.glVersion >= 3.0f || R_CheckExtension( "GL_ANGLE_framebuffer_multisample" ) );
	if ( qnx.multisampleFramebufferSupported ) {
		if ( glConfig.glVersion >= 3.0f ) {
			qglRenderbufferStorageMultisampleANGLE = (PFNGLRENDERBUFFERSTORAGEMULTISAMPLEANGLEPROC)GLimp_ExtensionPointer( "glRenderbufferStorageMultisample" );
		} else {
			qglRenderbufferStorageMultisampleANGLE = (PFNGLRENDERBUFFERSTORAGEMULTISAMPLEANGLEPROC)GLimp_ExtensionPointer( "glRenderbufferStorageMultisampleANGLE" );
		}
	}

	qnx.multisampleFramebufferTextureSupported = R_CheckExtension( "GL_IMG_multisampled_render_to_texture" );
	if ( qnx.multisampleFramebufferTextureSupported ) {
		qglRenderbufferStorageMultisampleANGLE = (PFNGLRENDERBUFFERSTORAGEMULTISAMPLEANGLEPROC)GLimp_ExtensionPointer( "glRenderbufferStorageMultisampleIMG" );
		qglFramebufferTexture2DMultisampleIMG = (PFNGLFRAMEBUFFERTEXTURE2DMULTISAMPLEIMGPROC)GLimp_ExtensionPointer( "glFramebufferTexture2DMultisampleIMG" );
	}

	qnx.packedFramebufferSupported = ( glConfig.glVersion >= 3.0f || R_CheckExtension( "GL_OES_packed_depth_stencil" ) );

	qnx.depth24FramebufferSupported = ( glConfig.glVersion >= 3.0f || R_CheckExtension( "GL_OES_depth24") || R_CheckExtension( "GL_OES_required_internalformat" ) );

	// check for errors
	if ( !qnx.blitSupported && qnx.multisampleFramebufferSupported ) {
		idLib::Error( "Framebuffer blit not available" );
	}
	if ( !qnx.multisampleFramebufferSupported && !qnx.multisampleFramebufferTextureSupported ) {
		idLib::Error( "Framebuffer multisample not available" );
	}
	if ( !( qnx.packedFramebufferSupported || qnx.depth24FramebufferSupported ) ) {
		idLib::Error( "24 bit depth renderbuffer not available" );
	}

#ifdef USE_GLES_MULTISAMPLE_FRAMEBUFFER
	// If on a PVR GPU, even if blit is supported, the color buffer won't be multisampled. So we can't use blit.
	qnx.useBlit = qnx.blitSupported && !qnx.multisampleFramebufferTextureSupported;
#else
	// If blit is supported, just use standard framebuffers
	qnx.useBlit = qnx.blitSupported;
#endif

	// generate renderbuffers
	int renderbufferOffset = qnx.packedFramebufferSupported ? RENDERBUFFER_PER_FRAMEBUFFER_PACKED : RENDERBUFFER_PER_FRAMEBUFFER;
	qglGenRenderbuffers( FRAMEBUFFER_COUNT * renderbufferOffset, qnx.renderbuffers );

	if ( !qnx.useBlit ) {
		qglGenTextures( FRAMEBUFFER_COUNT, qnx.fbTextures );
	}

	// setup framebuffers
	GLint maxSamples;
	if ( qnx.multisampleFramebufferTextureSupported ) {
		qglGetIntegerv( GL_MAX_SAMPLES_IMG, &maxSamples );
	} else {
		qglGetIntegerv( GL_MAX_SAMPLES_ANGLE, &maxSamples );
	}
	glConfig.maxMultisamples = maxSamples;

	extern idCVar r_useSRGB;
	bool useSRGB = glConfig.sRGBFramebufferAvailable && ( r_useSRGB.GetInteger() == 1 || r_useSRGB.GetInteger() == 2 );
	GLsizei samples = r_multiSamples.GetInteger();
	samples = __min( __max( samples, 0 ), glConfig.maxMultisamples );

	R_BuildFramebuffers( useSRGB, samples, renderbufferOffset );
}

/*
=================
R_ShutdownFramebuffers
=================
*/
void R_ShutdownFramebuffers() {
	if ( !qnx.useBlit ) {
		qglDeleteTextures( FRAMEBUFFER_COUNT, qnx.fbTextures );
	}
	qglDeleteRenderbuffers( ( qnx.packedFramebufferSupported ? RENDERBUFFER_PER_FRAMEBUFFER_PACKED : RENDERBUFFER_PER_FRAMEBUFFER ), qnx.renderbuffers );
	qglDeleteFramebuffers( FRAMEBUFFER_COUNT, qnx.framebuffers );
}

/*
=================
R_UpdateGLVersion

Acts like a "check and swap" function. If blit is enabled, check if "real" functions are already set. If not, then set them.
If blit is not enabled, check if "fake" functions are already set. If not, then set them.
=================
*/
void R_UpdateGLESVersion() {
	bool update = false;
	EGLFunctionReplacements_t replacements = {
		GLimp_glReadBuffer,			// glReadBufferImpl
		GLimp_glDrawBuffer,			// glDrawBufferImpl
		qglReadPixels_real,			// glReadPixelsImpl
		qglCopyTexImage2D_real,		// glCopyTexImage2DImpl
		qglCopyTexSubImage2D_real,	// glCopyTexSubImage2DImpl
#ifdef GL_ES_VERSION_3_0
		qglCopyTexSubImage3D_real	// glCopyTexSubImage3DImpl
#endif
	};

	// Update functions
	if ( qnx.useBlit ) {
		update = qglReadPixels_real != NULL;

		qglReadPixels_real = NULL;
		qglCopyTexImage2D_real = NULL;
		qglCopyTexSubImage2D_real = NULL;
#ifdef GL_ES_VERSION_3_0
		qglCopyTexSubImage3D_real = NULL;
#endif
	} else {
		update = qglReadPixels_real == NULL;

		qglReadPixels_real = qglReadPixels;
		qglCopyTexImage2D_real = qglCopyTexImage2D;
		qglCopyTexSubImage2D_real = qglCopyTexSubImage2D;
#ifdef GL_ES_VERSION_3_0
		qglCopyTexSubImage3D_real = qglCopyTexSubImage3D;
#endif

		replacements.glReadPixelsImpl = GLimp_glReadPixels;
		replacements.glCopyTexImage2DImpl = GLimp_glCopyTexImage2D;
		replacements.glCopyTexSubImage2DImpl = GLimp_glCopyTexSubImage2D;
#ifdef GL_ES_VERSION_3_0
		replacements.glCopyTexSubImage3DImpl = GLimp_glCopyTexSubImage3D;
#endif
	}

	if ( update ) {
		OGL_UpdateReplacements( replacements );
	}
}

/*
=================
EGL_CheckExtension
=================
*/
ID_FORCE_INLINE bool EGL_CheckExtension( const char *name ) {
	return strstr( glConfig.egl_extensions_string, name ) != NULL;
}

/*
===================
QNXGLimp_DowngradeScreenUsage
===================
*/
bool QNXGLimp_DowngradeScreenUsage() {
	int screenUsage;
	if ( screen_get_window_property_iv( qnx.screenWin, SCREEN_PROPERTY_USAGE, &screenUsage ) != 0 ) {
		return false;
	}
	if ( screenUsage & SCREEN_USAGE_OPENGL_ES2 ) {
		// already GLES 2.0
		return true;
	}
#if defined(GL_ES_VERSION_3_0) && BBNDK_VERSION_AT_LEAST(10, 2, 0)
	if ( !( screenUsage & SCREEN_USAGE_OPENGL_ES3 ) ) {
		// not GLES 3.0?
		return false;
	}
	if ( screen_destroy_window_buffers( qnx.screenWin ) != 0 ) {
		return false;
	}
	// switch to GLES 2.0 usage and recreate buffers
	screenUsage &= ~SCREEN_USAGE_OPENGL_ES3;
	screenUsage |= SCREEN_USAGE_OPENGL_ES2;
	screen_set_window_property_iv( qnx.screenWin, SCREEN_PROPERTY_USAGE, &screenUsage );
	if ( screen_create_window_buffers( qnx.screenWin, 2 ) != 0 ) {
		return false;
	}
#endif
	return true;
}

/*
===================
QNXGLimp_WindowGroup
===================
*/
const char *QNXGLimp_WindowGroup()
{
    static char windowGroup[32] = { 0 };

    if ( !windowGroup[0] ) {
    	idStr::snPrintf( windowGroup, sizeof( windowGroup ), "doom_window_group_%d", getpid() );
    }

    return windowGroup;
}

/*
===================
QNXGLimp_CreateWindow
===================
*/
bool QNXGLimp_CreateWindow( int x, int y, int width, int height, int fullScreen ) {

	const int screenFormat = SCREEN_FORMAT_RGBA8888;
	const int screenIdleMode = SCREEN_IDLE_MODE_KEEP_AWAKE;

#ifdef ID_QNX_X86
	int screenUsage = 0;
#else
	int screenUsage = SCREEN_USAGE_DISPLAY; // Physical device copy directly into physical display
#endif
#if defined(GL_ES_VERSION_3_0) && BBNDK_VERSION_AT_LEAST(10, 2, 0)
	screenUsage |= ( ( r_useGLES3.GetInteger() != 1 ) ? SCREEN_USAGE_OPENGL_ES3 : SCREEN_USAGE_OPENGL_ES2 );
#else
	screenUsage |= SCREEN_USAGE_OPENGL_ES2;
#endif

	if ( screen_create_context( &qnx.screenCtx, 0 ) != 0 ) {
		common->Printf( "Could not create screen context\n" );
		return false;
	}

	if ( screen_create_window( &qnx.screenWin, qnx.screenCtx ) != 0 ) {
		common->Printf( "Could not create screen window\n" );
		return false;
	}

	if ( screen_create_window_group( qnx.screenWin, QNXGLimp_WindowGroup() ) != 0 ) {
		common->Printf( "Could not create window group\n" );
		return false;
	}

	if ( screen_set_window_property_iv( qnx.screenWin, SCREEN_PROPERTY_FORMAT, &screenFormat ) != 0 ) {
		common->Printf( "Could not set window format\n" );
		return false;
	}

	if ( screen_set_window_property_iv( qnx.screenWin, SCREEN_PROPERTY_USAGE, &screenUsage ) != 0 ) {
#if defined(GL_ES_VERSION_3_0) && BBNDK_VERSION_AT_LEAST(10, 2, 0)
		// Try to fallback to GLES 2.0 for usage expectations
#ifdef ID_QNX_X86
		screenUsage = SCREEN_USAGE_OPENGL_ES2;
#else
		screenUsage = SCREEN_USAGE_DISPLAY | SCREEN_USAGE_OPENGL_ES2;
#endif
		if ( r_useGLES3.GetInteger() != 0 || screen_set_window_property_iv( qnx.screenWin, SCREEN_PROPERTY_USAGE, &screenUsage ) != 0 ) {
			common->Printf( "Could not set window usage\n" );
			return false;
		}
#else
		common->Printf( "Could not set window usage\n" );
		return false;
#endif // defined(GL_ES_VERSION_3_0) && BBNDK_VERSION_AT_LEAST(10, 2, 0)
	}

	if ( screen_set_window_property_iv( qnx.screenWin, SCREEN_PROPERTY_IDLE_MODE, &screenIdleMode ) != 0 ) {
		common->Printf( "Could not set window idle mode\n" );
		return false;
	}

	if ( r_debugContext.GetBool() ) {
		// Setup debug info (just a readout)
		int debugFlags = SCREEN_DEBUG_GRAPH_FPS;
		if ( r_debugContext.GetInteger() >= 2 ) {
			debugFlags |= SCREEN_DEBUG_GRAPH_CPU_TIME | SCREEN_DEBUG_GRAPH_GPU_TIME;
		}
		if ( r_debugContext.GetInteger() >= 3 ) {
			debugFlags = SCREEN_DEBUG_STATISTICS; // Note: We replace the value since it's not supposed to be a flag
		}
		if ( screen_set_window_property_iv( qnx.screenWin, SCREEN_PROPERTY_DEBUG, &debugFlags ) != 0 ) {
			common->Printf( "Could not set window debug flags\n" );
		}
	}

	const int position[2] = {x, y};
	if ( screen_set_window_property_iv( qnx.screenWin, SCREEN_PROPERTY_POSITION, position ) != 0 ) {
		common->Printf( "Could not set window position\n" );
		return false;
	}

	const int size[2] = {width, height};
	if ( screen_set_window_property_iv( qnx.screenWin, SCREEN_PROPERTY_BUFFER_SIZE, size ) != 0 ) {
		common->Printf( "Could not set window size\n" );
		return false;
	}

	int rotation = atoi( getenv( "ORIENTATION" ) );
	if ( screen_set_window_property_iv( qnx.screenWin, SCREEN_PROPERTY_ROTATION, &rotation ) != 0 ) {
		common->Printf( "Could not set window rotation\n" );
		return false;
	}

	// Change display if determined
	screen_display_t display;
	if ( fullScreen > 1 ) {
		int displayCount = 1;
		if ( screen_get_context_property_iv( qnx.screenCtx, SCREEN_PROPERTY_DISPLAY_COUNT, &displayCount ) != 0 ) {
			common->Printf( "Could not get display count\n" );
			return false;
		}

		if ( ( fullScreen - 1 ) >= displayCount ) {
			common->Printf( "fullScreen index does not correlate to a display\n" );
			return false;
		}

		screen_display_t* displayList = ( screen_display_t* )Mem_Alloc( sizeof( screen_display_t ) * displayCount, TAG_TEMP );
		if ( screen_get_context_property_pv( qnx.screenCtx, SCREEN_PROPERTY_DISPLAYS, ( void** )displayList ) != 0 ) {
			Mem_Free( displayList );
			common->Printf( "Could not get displays\n" );
			return false;
		}
		display = displayList[ fullScreen - 1 ];
		Mem_Free( ( void* )displayList );

		if ( screen_set_window_property_pv( qnx.screenWin, SCREEN_PROPERTY_DISPLAY, ( void** )&display ) != 0 ) {
			common->Printf( "Could not set window display\n" );
			return false;
		}
	}

	if ( screen_get_window_property_pv( qnx.screenWin, SCREEN_PROPERTY_DISPLAY, ( void** )&display ) == 0 ) {
		screen_get_display_property_iv( display, SCREEN_PROPERTY_ID, &qnx.screenDisplayID );
	}

	if ( screen_create_window_buffers( qnx.screenWin, 2 ) != 0 ) {
		common->Printf( "Could not set window usage\n" );
		return false;
	}

	return true;
}

/*
===================
QNXGLimp_CreateEGLConfig
===================
*/
bool QNXGLimp_CreateEGLConfig( int multisamples, bool gles3bit ) {
	EGLint eglConfigAttrs[] =
	{
#ifdef USE_GLES_MULTISAMPLE_EGL
		EGL_SAMPLE_BUFFERS,     ( ( multisamples >= 1 ) ? 1 : 0 ),
		EGL_SAMPLES,            multisamples,
#endif
		EGL_RED_SIZE,           8,
		EGL_GREEN_SIZE,         8,
		EGL_BLUE_SIZE,          8,
		EGL_ALPHA_SIZE,         8,
		EGL_DEPTH_SIZE,         24,
		EGL_STENCIL_SIZE,       8,
		EGL_SURFACE_TYPE,       EGL_WINDOW_BIT,
#ifdef EGL_KHR_create_context
		EGL_RENDERABLE_TYPE,    ( gles3bit ? EGL_OPENGL_ES3_BIT_KHR : EGL_OPENGL_ES2_BIT ),
#else
		EGL_RENDERABLE_TYPE,    EGL_OPENGL_ES2_BIT,
#endif
		EGL_NONE
	};

	EGLint eglConfigCount;
	bool success = true;

	if ( qeglChooseConfig( qnx.eglDisplay, eglConfigAttrs, &qnx.eglConfig, 1, &eglConfigCount ) != EGL_TRUE || eglConfigCount == 0 ) {
		success = false;
#if 0 // While useful, if the specified number of samples isn't supported then we want it to re-pick a config instead of saying one config while having another set
		while (multisamples) {
			// Try lowering the MSAA sample count until we find a supported config
			multisamples /= 2;
			eglConfigAttrs[1] = ( ( multisamples >= 1 ) ? 1 : 0 );
			eglConfigAttrs[3] = multisamples;

			if ( qeglChooseConfig( qnx.eglDisplay, eglConfigAttrs, &qnx.eglConfig, 1, &eglConfigCount ) == EGL_TRUE && eglConfigCount > 0 ) {
				success = true;
				break;
			}
		}
#endif
	}

	return success;
}

/*
===================
QNXGLimp_CreateEGLSurface
===================
*/
bool QNXGLimp_CreateEGLSurface( EGLNativeWindowType window, const int multisamples, bool createContextAvaliable, bool contextPriorityAvaliable ) {
	common->Printf( "Initializing OpenGL driver\n" );

	bool resetEGLVersion = false;

	EGLint eglContextAttrs[] =
	{
		EGL_CONTEXT_CLIENT_VERSION,	0,
		EGL_NONE,					EGL_NONE, // EGL_CONTEXT_FLAGS_KHR,				EGL_CONTEXT_OPENGL_DEBUG_BIT_KHR
		EGL_NONE,					EGL_NONE, // EGL_CONTEXT_PRIORITY_LEVEL_IMG,	EGL_CONTEXT_PRIORITY_HIGH_IMG
		EGL_NONE
	};
#ifdef GL_ES_VERSION_3_0
	eglContextAttrs[1] = 3;
	if ( r_useGLES3.GetInteger() == 1 ) {
		eglContextAttrs[1] = 2;
	}
#else
	eglContextAttrs[1] = 2;
	if ( r_useGLES3.GetInteger() == 2 ) {
		// Compiled with OpenGL ES 2.0, 3.0 is not available
		return false;
	}
#endif

	// Additional attrib. if supported
	int additionalAttribIndex = 2;
#ifdef EGL_KHR_create_context
	if ( r_debugContext.GetBool() && createContextAvaliable ) {
		eglContextAttrs[additionalAttribIndex + 0] = EGL_CONTEXT_FLAGS_KHR;
		eglContextAttrs[additionalAttribIndex + 1] = EGL_CONTEXT_OPENGL_DEBUG_BIT_KHR;
		additionalAttribIndex += 2;
	}
#endif
#ifdef EGL_IMG_context_priority
	if ( contextPriorityAvaliable ) {
		eglContextAttrs[additionalAttribIndex + 0] = EGL_CONTEXT_PRIORITY_LEVEL_IMG;
		eglContextAttrs[additionalAttribIndex + 1] = EGL_CONTEXT_PRIORITY_HIGH_IMG;
		additionalAttribIndex += 2;
	}
#endif

	if ( qeglInitialize( qnx.eglDisplay, NULL, NULL ) != EGL_TRUE ) {
		common->Printf( "...^3Could not initialize EGL display^0\n");
		return false;
	}

	if ( !QNXGLimp_CreateEGLConfig( multisamples, createContextAvaliable && r_useGLES3.GetInteger() != 1 ) ) {
		if ( r_useGLES3.GetInteger() != 0 || !QNXGLimp_CreateEGLConfig( multisamples, false ) ) {
			common->Printf( "...^3Could not determine EGL config^0\n");
			return false;
		}
	}

	for ( int config = 0; config < 2; config++ ) {
		qnx.eglContext = qeglCreateContext( qnx.eglDisplay, qnx.eglConfig, EGL_NO_CONTEXT, eglContextAttrs );
		if ( qnx.eglContext == EGL_NO_CONTEXT ) {
			// If we already tried to create the context or we only want a specific version of GLES, then fail
			if ( config == 1 || r_useGLES3.GetInteger() != 0 ) {
				common->Printf( "...^3Could not create EGL context^0\n");
				return false;
			}
			eglContextAttrs[1] = 2; // Switch to OpenGL ES 2.0
			resetEGLVersion = true;
			continue;
		}
		break;
	}

	if ( resetEGLVersion && !QNXGLimp_DowngradeScreenUsage() ) {
		common->Printf( "...^3EGL context version mismatch with screen usage, could not change screen usage^0\n");
		return false;
	}

	qnx.eglSurface = qeglCreateWindowSurface( qnx.eglDisplay, qnx.eglConfig, window, NULL );
	if ( qnx.eglSurface == EGL_NO_SURFACE ) {
		common->Printf( "...^3Could not create EGL window surface^0\n");
		return false;
	}

	common->Printf( "...making context current: " );
	if ( qeglMakeCurrent( qnx.eglDisplay, qnx.eglSurface, qnx.eglSurface, qnx.eglContext ) != EGL_TRUE ) {
		common->Printf( "^3failed^0\n");
		return false;
	}
	common->Printf( "succeeded\n" );

	return true;
}

/*
===================
GLimp_Init

This is the platform specific OpenGL initialization function.  It
is responsible for loading OpenGL, initializing it,
creating a window of the appropriate size, doing
fullscreen manipulations, etc.  Its overall responsibility is
to make sure that a functional OpenGL subsystem is operating
when it returns to the ref.

If there is any failure, the renderer will revert back to safe
parameters and try again.

We assume params is based on device rotation.
===================
*/
bool GLimp_Init( glimpParms_t parms ) {
	const char	*driverName;

	// Check if already init
	if ( qnx.screenCtx != NULL ) {
		return false;
	}

	// Some early checks to make sure supported params exist
	if ( parms.x != 0 || parms.y != 0 || parms.fullScreen <= 0 ) {
		return false;
	}

	cmdSystem->AddCommand( "testSwapBuffers", GLimp_TestSwapBuffers, CMD_FL_SYSTEM, "Times swapbuffer options" );

	common->Printf( "Initializing OpenGL subsystem with multisamples:%i stereo:%i\n", parms.multiSamples, parms.stereo );

	// this will load the dll and set all our qgl* function pointers,
	// but doesn't create a window

	const EGLFunctionReplacements_t replacements = {
		GLimp_glReadBuffer,	// glReadBufferImpl
		GLimp_glDrawBuffer,	// glDrawBufferImpl
		NULL,				// glReadPixelsImpl
		NULL,				// glCopyTexImage2DImpl
		NULL,				// glCopyTexSubImage2DImpl
		NULL				// glCopyTexSubImage3DImpl
	};

	// r_glDriver is only intended for using instrumented OpenGL
	// so-s.  Normal users should never have to use it, and it is
	// not archived.
	driverName = r_glDriver.GetString()[0] ? r_glDriver.GetString() : "libGLESv2.so";
	if ( !QGL_Init( driverName, replacements ) ) {
		common->Printf( "^3GLimp_Init() could not load r_glDriver \"%s\"^0\n", driverName );
		return false;
	}

	qnx.eglDisplay = qeglGetDisplay( EGL_DEFAULT_DISPLAY );
	if ( qnx.eglDisplay == EGL_NO_DISPLAY ) {
		common->Printf( "...^3Could not get EGL display^0\n");
		return false;
	}

	glConfig.egl_extensions_string = qeglQueryString( qnx.eglDisplay, EGL_EXTENSIONS );

	if ( !QNXGLimp_CreateWindow( parms.x, parms.y, parms.width, parms.height, parms.fullScreen ) ) {
		GLimp_Shutdown();
		return false;
	}

	if ( !QNXGLimp_CreateEGLSurface( qnx.screenWin, parms.multiSamples,
			EGL_CheckExtension( "EGL_KHR_create_context" ), EGL_CheckExtension( "EGL_IMG_context_priority" ) ) ) {
		GLimp_Shutdown();
		return false;
	}

	r_swapInterval.SetModified();				// force a set next frame
	glConfig.swapControlTearAvailable = false;
	glConfig.stereoPixelFormatAvailable = true;	// It's all emulated by glimp, so yes we can support this

	qeglGetConfigAttrib( qnx.eglDisplay, qnx.eglConfig, EGL_BUFFER_SIZE, &glConfig.colorBits );
	qeglGetConfigAttrib( qnx.eglDisplay, qnx.eglConfig, EGL_DEPTH_SIZE, &glConfig.depthBits );
	qeglGetConfigAttrib( qnx.eglDisplay, qnx.eglConfig, EGL_STENCIL_SIZE, &glConfig.stencilBits );

	// We won't swap width and height as the params are expected to be based on rotation
	glConfig.isFullscreen = parms.fullScreen;
	glConfig.isStereoPixelFormat = parms.stereo;
	glConfig.nativeScreenWidth = parms.width;
	glConfig.nativeScreenHeight = parms.height;
	glConfig.multisamples = parms.multiSamples;

	glConfig.pixelAspect = 1.0f;	// FIXME: some monitor modes may be distorted
									// should side-by-side stereo modes be consider aspect 0.5?

	screen_display_t display;
	glConfig.physicalScreenWidthInCentimeters = 100.0f;
	if ( screen_get_window_property_pv( qnx.screenWin, SCREEN_PROPERTY_DISPLAY, ( void** )&display ) == 0 ) {
		int size[2];
		if ( screen_get_display_property_iv( display, SCREEN_PROPERTY_PHYSICAL_SIZE, size ) == 0 ) {
			int index = 0;
			if ( ( ( parms.width > parms.height ) && ( size[0] < size[1] ) ) ||
					( ( parms.width < parms.height ) && ( size[0] > size[1] ) ) ) {
				index = 1;
			}
			glConfig.physicalScreenWidthInCentimeters = size[index] * 0.1f; //MM to CM
		}
	}

	// check logging
	GLimp_EnableLogging( ( r_logFile.GetInteger() != 0 ) );

	return true;
}

/*
===================
GLimp_SetScreenParms

Sets up the screen based on passed parms..
===================
*/
bool GLimp_SetScreenParms( glimpParms_t parms ) {
	// Some early checks to make sure supported params exist
	if ( parms.x != 0 || parms.y != 0 || parms.fullScreen <= 0 ) {
		return false;
	}

	// Destroy old surface, change params, then recreate surface
	if ( qeglMakeCurrent( qnx.eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT ) != EGL_TRUE ) {
		common->Printf( "Could not disable screen\n");
		return false;
	}

	if ( qeglDestroySurface( qnx.eglDisplay, qnx.eglSurface ) != EGL_TRUE ) {
		common->Printf( "Could not destroy EGL surface\n");
		return false;
	}

	const int position[2] = {parms.x, parms.y};
	if ( screen_set_window_property_iv( qnx.screenWin, SCREEN_PROPERTY_POSITION, position ) != 0 ) {
		common->Printf( "Could not set window position\n" );
		return false;
	}

	const int size[2] = {parms.width, parms.height};
	if ( screen_set_window_property_iv( qnx.screenWin, SCREEN_PROPERTY_BUFFER_SIZE, size ) != 0 ) {
		common->Printf( "Could not set window size\n" );
		return false;
	}

	qnx.eglSurface = qeglCreateWindowSurface( qnx.eglDisplay, qnx.eglConfig, qnx.screenWin, NULL );
	if ( qnx.eglSurface == EGL_NO_SURFACE ) {
		common->Printf( "Could not recreate EGL window surface\n");
		return false;
	}

	if ( qeglMakeCurrent( qnx.eglDisplay, qnx.eglSurface, qnx.eglSurface, qnx.eglContext ) != EGL_TRUE ) {
		common->Printf( "Could not make screen current\n");
		return false;
	}

	glConfig.nativeScreenWidth = parms.width;
	glConfig.nativeScreenHeight = parms.height;

	r_swapInterval.SetModified();

	// Get displays
	int displayCount = 1;
	if ( screen_get_context_property_iv( qnx.screenCtx, SCREEN_PROPERTY_DISPLAY_COUNT, &displayCount ) != 0 ) {
		common->Printf( "Could not get display count\n" );
		return false;
	}

	if ( ( parms.fullScreen - 1 ) >= displayCount ) {
		common->Printf( "fullScreen index does not correlate to a display\n" );
		return false;
	}

	screen_display_t* displayList = ( screen_display_t* )Mem_Alloc( sizeof( screen_display_t ) * displayCount, TAG_TEMP );
	if ( screen_get_context_property_pv( qnx.screenCtx, SCREEN_PROPERTY_DISPLAYS, ( void** )displayList ) != 0 ) {
		Mem_Free( displayList );
		common->Printf( "Could not get displays\n" );
		return false;
	}
	screen_display_t display = displayList[ parms.fullScreen - 1 ];
	Mem_Free( ( void* )displayList );

	int displayId;
	screen_get_display_property_iv( display, SCREEN_PROPERTY_ID, &displayId );
	if ( displayId != qnx.screenDisplayID && screen_set_window_property_pv( qnx.screenWin, SCREEN_PROPERTY_DISPLAY, ( void** )&display ) != 0 ) {
		common->Printf( "Could not set window display\n" );
		return false;
	}
	qnx.screenDisplayID = displayId;

	glConfig.isFullscreen = parms.fullScreen;
	glConfig.pixelAspect = 1.0f;	// FIXME: some monitor modes may be distorted

	return true;
}

/*
===================
GLimp_Shutdown

This routine does all OS specific shutdown procedures for the OpenGL
subsystem.
===================
*/
void GLimp_Shutdown() {
	const char *success[] = { "failed", "success" };
	int retVal;

	common->Printf( "Shutting down OpenGL subsystem\n" );

	qnx.eglConfig = NULL;

	if ( qnx.eglDisplay != EGL_NO_DISPLAY ) {
		R_ShutdownFramebuffers();

		retVal = qeglMakeCurrent( qnx.eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT );
		common->Printf( "...qeglMakeCurrent( display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT ): %s\n", success[retVal] );
	}

	if ( qnx.eglSurface != EGL_NO_SURFACE ) {
		retVal = qeglDestroySurface( qnx.eglDisplay, qnx.eglSurface );
		common->Printf( "...destroying surface: %s\n", success[retVal] );
		qnx.eglSurface = EGL_NO_SURFACE;
	}

	if ( qnx.eglContext != EGL_NO_CONTEXT ) {
		retVal = qeglDestroyContext( qnx.eglDisplay, qnx.eglContext );
		common->Printf( "...destroying context: %s\n", success[retVal] );
		qnx.eglContext = EGL_NO_CONTEXT;
	}

	if ( qnx.eglDisplay != EGL_NO_DISPLAY ) {
		retVal = qeglTerminate( qnx.eglDisplay );
		common->Printf( "...terminating display: %s\n", success[retVal] );
		qnx.eglDisplay = EGL_NO_DISPLAY;
	}

	if ( qnx.screenWin ) {
		retVal = screen_destroy_window( qnx.screenWin ) + 1;
		common->Printf( "...destroying window: %s\n", success[retVal] );
		qnx.screenWin = NULL;
	}

	if ( qnx.screenCtx ) {
		retVal = screen_destroy_context( qnx.screenCtx ) + 1;
		common->Printf( "...destroying window context: %s\n", success[retVal] );
		qnx.screenCtx = NULL;
	}

	if ( qnx.renderThread ) {
		common->Printf( "...closing smp thread\n" );
		Sys_DestroyThread( qnx.renderThread );
		qnx.renderThread = ( uintptr_t )NULL;
	}

	if ( qnx.renderCommandsEvent ) {
		common->Printf( "...destroying smp signals\n" );
		Sys_SignalDestroy( qnx.renderCommandsEvent );
		Sys_SignalDestroy( qnx.renderCompletedEvent );
		Sys_SignalDestroy( qnx.renderActiveEvent );
		qnx.renderCommandsEvent = NULL;
		qnx.renderCompletedEvent = NULL;
		qnx.renderActiveEvent = NULL;
	}

	// shutdown QGL subsystem
	QGL_Shutdown();
}

/*
=====================
GLimp_SwapBuffers
=====================
*/
void GLimp_SwapBuffers() {

	// Setup swap interval
	if ( r_swapInterval.IsModified() ) {
		r_swapInterval.ClearModified();

		int interval = 0;
		if ( r_swapInterval.GetInteger() != 0 ) {
			interval = 1;
		}

		qeglSwapInterval( qnx.eglDisplay, interval );
	}

	// Only if framebuffers have been init should framebuffer processing be executed
	if ( qnx.framebuffers[0] != 0 ) {
#ifdef GL_EXT_discard_framebuffer
		static const GLenum discardAttachments[3] = { GL_COLOR_EXT, GL_DEPTH_EXT, GL_STENCIL_EXT };

		if ( qnx.discardFramebuffersSupported ) {

			// Discard front-right buffer (not needed in current implementation)
			qglBindFramebuffer( GL_FRAMEBUFFER, qnx.framebuffers[0] );
			qglDiscardFramebufferEXT( GL_FRAMEBUFFER, 3, discardAttachments );

			// Swap front-right with back-right
			int tmp = qnx.framebuffers[0];
			qnx.framebuffers[0] = qnx.framebuffers[2];
			qnx.framebuffers[2] = tmp;

			int renderbufferOffset = qnx.packedFramebufferSupported ? RENDERBUFFER_PER_FRAMEBUFFER_PACKED : RENDERBUFFER_PER_FRAMEBUFFER;
			tmp = qnx.renderbuffers[0 * renderbufferOffset + 0];
			qnx.renderbuffers[0 * renderbufferOffset + 0] = qnx.renderbuffers[2 * renderbufferOffset + 0];
			qnx.renderbuffers[2 * renderbufferOffset + 0] = tmp;

			tmp = qnx.renderbuffers[0 * renderbufferOffset + 1];
			qnx.renderbuffers[0 * renderbufferOffset + 1] = qnx.renderbuffers[2 * renderbufferOffset + 1];
			qnx.renderbuffers[2 * renderbufferOffset + 1] = tmp;

			if ( renderbufferOffset == RENDERBUFFER_PER_FRAMEBUFFER ) {
				tmp = qnx.renderbuffers[0 * renderbufferOffset + 2];
				qnx.renderbuffers[0 * renderbufferOffset + 2] = qnx.renderbuffers[2 * renderbufferOffset + 2];
				qnx.renderbuffers[2 * renderbufferOffset + 2] = tmp;
			}
		}
#endif

		int curDraw = R_GetBufferIndex( qnx.drawBuffer );
		if ( qnx.useBlit ) {
			int curRead = R_GetBufferIndex( qnx.readBuffer );
			if ( curRead != 1 ) {
				qglBindFramebuffer( GL_READ_FRAMEBUFFER_ANGLE, qnx.framebuffers[1] ); // BACK_LEFT framebuffer
			}
			if ( curDraw != INT_MAX ) {
				qglBindFramebuffer( GL_DRAW_FRAMEBUFFER_ANGLE, 0 );
			}

			qglBlitFramebufferANGLE( 0, 0, glConfig.nativeScreenWidth, glConfig.nativeScreenHeight, 0, 0, glConfig.nativeScreenWidth, glConfig.nativeScreenHeight,
					GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, GL_NEAREST );

			if ( curRead == curDraw ) {
				qglBindFramebuffer( GL_FRAMEBUFFER, ( curRead == INT_MAX ? 0 : qnx.framebuffers[curRead] ) );
			} else {
				if ( curRead != 1 ) {
					qglBindFramebuffer( GL_READ_FRAMEBUFFER_ANGLE, ( curRead == INT_MAX ? 0 : qnx.framebuffers[curRead] ) );
				}
				if ( curDraw != INT_MAX ) {
					qglBindFramebuffer( GL_DRAW_FRAMEBUFFER_ANGLE, qnx.framebuffers[curDraw] );
				}
			}
		} else {
			if ( curDraw != INT_MAX ) {
				qglBindFramebuffer( GL_FRAMEBUFFER, 0 );
			}

			//TODO: switch shaders, render texture, go back to prior shaders

			if ( curDraw != INT_MAX ) {
				qglBindFramebuffer( GL_FRAMEBUFFER, qnx.framebuffers[curDraw] );
			}
		}
	}

	// Swap EGL buffers
	qeglSwapBuffers( qnx.eglDisplay, qnx.eglSurface );
}

/*
===========================================================

SMP acceleration

===========================================================
*/

/*
===================
GLimp_ActivateContext
===================
*/
void GLimp_ActivateContext() {
	qeglMakeCurrent( qnx.eglDisplay, qnx.eglSurface, qnx.eglSurface, qnx.eglContext );
}

/*
===================
GLimp_DeactivateContext
===================
*/
void GLimp_DeactivateContext() {
	qglFinish();
	qeglMakeCurrent( qnx.eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT );
}

/*
===================
GLimp_RenderThreadWrapper
===================
*/
static unsigned int GLimp_RenderThreadWrapper( void* args ) {
	void (*renderThread)() = ( void (*)() )args;

	renderThread();

	// unbind the context before we die
	return qeglMakeCurrent( qnx.eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT );
}

/*
=======================
GLimp_SpawnRenderThread

Returns false if the system only has a single processor
=======================
*/
bool GLimp_SpawnRenderThread( void (*function)() ) {
	// check number of processors
	if ( _syspage_ptr->num_cpu < 2 ) {
		return false;
	}

	//What if the values already exist? The Windows version doesn't do anything (though it doesn't seem to cleanup it's events either...)

	// create the IPC elements
	Sys_SignalCreate( qnx.renderCommandsEvent, true );
	Sys_SignalCreate( qnx.renderCompletedEvent, true );
	Sys_SignalCreate( qnx.renderActiveEvent, true );

	qnx.renderThread = Sys_CreateThread( GLimp_RenderThreadWrapper, ( void* )function, THREAD_ABOVE_NORMAL, "qnx_renderThread", CORE_ANY );

	return true;
}


//#define	DEBUG_PRINTS

/*
===================
GLimp_BackEndSleep
===================
*/
void *GLimp_BackEndSleep() {
	void	*data;

#ifdef DEBUG_PRINTS
	slog2c( NULL, SLOG_CODE, SLOG2_DEBUG2, "-->GLimp_BackEndSleep\n" );
#endif
	Sys_SignalClear( qnx.renderActiveEvent );

	// after this, the front end can exit GLimp_FrontEndSleep
	Sys_SignalRaise( qnx.renderCompletedEvent );

	Sys_SignalWait( qnx.renderCommandsEvent, idSysSignal::WAIT_INFINITE );

	Sys_SignalClear( qnx.renderCompletedEvent );
	Sys_SignalClear( qnx.renderCommandsEvent );

	data = qnx.smpData;

	// after this, the main thread can exit GLimp_WakeRenderer
	Sys_SignalRaise( qnx.renderActiveEvent );

#ifdef DEBUG_PRINTS
	slog2c( NULL, SLOG_CODE, SLOG2_DEBUG2, "<--GLimp_BackEndSleep\n" );
#endif
	return data;
}

/*
===================
GLimp_FrontEndSleep
===================
*/
void GLimp_FrontEndSleep() {
#ifdef DEBUG_PRINTS
	slog2c( NULL, SLOG_CODE, SLOG2_DEBUG2, "-->GLimp_FrontEndSleep\n" );
#endif
	Sys_SignalWait( qnx.renderCompletedEvent, idSysSignal::WAIT_INFINITE );

#ifdef DEBUG_PRINTS
	slog2c( NULL, SLOG_CODE, SLOG2_DEBUG2, "<--GLimp_FrontEndSleep\n" );
#endif
}

volatile bool	renderThreadActive;

// We can use Sys_SignalWait, but it will simply return false if something goes wrong
int Sys_SignalWait_ErrorReturn( signalHandle_t & handle, int timeout );

/*
===================
GLimp_WakeBackEnd
===================
*/
void GLimp_WakeBackEnd( void *data ) {
	int		r;

#ifdef DEBUG_PRINTS
	slog2c( NULL, SLOG_CODE, SLOG2_DEBUG2, "-->GLimp_WakeBackEnd\n" );
#endif

	qnx.smpData = data;

	if ( renderThreadActive ) {
		common->FatalError( "GLimp_WakeBackEnd: already active" );
	}

	r = Sys_SignalWait_ErrorReturn( qnx.renderActiveEvent, 0 );
	if ( r == EOK ) {
		common->FatalError( "GLimp_WakeBackEnd: already signaled" );
	}

	r = Sys_SignalWait_ErrorReturn( qnx.renderCommandsEvent, 0 );
	if ( r == EOK ) {
		common->FatalError( "GLimp_WakeBackEnd: commands already signaled" );
	}

	// after this, the renderer can continue through GLimp_RendererSleep
	Sys_SignalRaise( qnx.renderCommandsEvent );

	r = Sys_SignalWait_ErrorReturn( qnx.renderActiveEvent, 5000 );

	if ( r == ETIMEDOUT ) {
		common->FatalError( "GLimp_WakeBackEnd: WAIT_TIMEOUT" );
	}

#ifdef DEBUG_PRINTS
	slog2c( NULL, SLOG_CODE, SLOG2_DEBUG2, "<--GLimp_WakeBackEnd\n" );
#endif
}

/*
===================
GLimp_ExtensionPointer

Returns a function pointer for an OpenGL extension entry point
===================
*/
GLExtension_t GLimp_ExtensionPointer( const char *name ) {
	void	(*proc)();

	proc = (GLExtension_t)qeglGetProcAddress( name );

	if ( !proc && !EGL_CheckExtension( "EGL_KHR_get_all_proc_addresses" ) ) {
		proc = (GLExtension_t)QGL_GetSym( name, ( name != NULL && name[0] == 'e' ) );
	}
	if ( !proc ) {
		common->Printf( "Couldn't find proc address for: %s\n", name );
	}

	return proc;
}
