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
#include "../../renderer/tr_local.h"

idCVar r_useGLES3( "r_useGLES3", "0", CVAR_INTEGER, "0 = OpenGL ES 3.0 if available, 1 = OpenGL ES 2.0, 2 = OpenGL 3.0", 0, 2 );

//
// function declaration
//
bool QGL_Init( const char *dllname );
void QGL_Shutdown();

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

	//TODO
	/*
	unsigned short table[3][256];
	int i;

	if ( !win32.hDC ) {
		return;
	}

	for ( i = 0; i < 256; i++ ) {
		table[0][i] = red[i];
		table[1][i] = green[i];
		table[2][i] = blue[i];
	}

	if ( !SetDeviceGammaRamp( win32.hDC, table ) ) {
		common->Printf( "WARNING: SetDeviceGammaRamp failed.\n" );
	}
	*/
}


/*
====================
PrintDevMode
====================
*/
static void PrintDevMode( DEVMODE & devmode ) {
	common->Printf( "          dmPosition.x        : %i\n", devmode.dmPosition.x );
	common->Printf( "          dmPosition.y        : %i\n", devmode.dmPosition.y );
	common->Printf( "          dmBitsPerPel        : %i\n", devmode.dmBitsPerPel );
	common->Printf( "          dmPelsWidth         : %i\n", devmode.dmPelsWidth );
	common->Printf( "          dmPelsHeight        : %i\n", devmode.dmPelsHeight );
	common->Printf( "          dmDisplayFixedOutput: %s\n", DMDFO( devmode.dmDisplayFixedOutput ) );
	common->Printf( "          dmDisplayFlags      : 0x%x\n", devmode.dmDisplayFlags );
	common->Printf( "          dmDisplayFrequency  : %i\n", devmode.dmDisplayFrequency );
}

/*
====================
DumpAllDisplayDevices
====================
*/
void DumpAllDisplayDevices() {
	//TODO
	common->Printf( "\n" );
	for ( int deviceNum = 0 ; ; deviceNum++ ) {
		DISPLAY_DEVICE	device = {};
		device.cb = sizeof( device );
		if ( !EnumDisplayDevices(
				0,			// lpDevice
				deviceNum,
				&device,
				0 /* dwFlags */ ) ) {
			break;
		}

		common->Printf( "display device: %i\n", deviceNum );
		common->Printf( "  DeviceName  : %s\n", device.DeviceName );
		common->Printf( "  DeviceString: %s\n", device.DeviceString );
		common->Printf( "  StateFlags  : 0x%x\n", device.StateFlags );
		common->Printf( "  DeviceID    : %s\n", device.DeviceID );
		common->Printf( "  DeviceKey   : %s\n", device.DeviceKey );

		for ( int monitorNum = 0 ; ; monitorNum++ ) {
			DISPLAY_DEVICE	monitor = {};
			monitor.cb = sizeof( monitor );
			if ( !EnumDisplayDevices(
					device.DeviceName,
					monitorNum,
					&monitor,
					0 /* dwFlags */ ) ) {
				break;
			}

			common->Printf( "      DeviceName  : %s\n", monitor.DeviceName );
			common->Printf( "      DeviceString: %s\n", monitor.DeviceString );
			common->Printf( "      StateFlags  : 0x%x\n", monitor.StateFlags );
			common->Printf( "      DeviceID    : %s\n", monitor.DeviceID );
			common->Printf( "      DeviceKey   : %s\n", monitor.DeviceKey );

			DEVMODE	currentDevmode = {};
			if ( !EnumDisplaySettings( device.DeviceName,ENUM_CURRENT_SETTINGS, &currentDevmode ) ) {
				common->Printf( "ERROR:  EnumDisplaySettings(ENUM_CURRENT_SETTINGS) failed!\n" );
			}
			common->Printf( "          -------------------\n" );
			common->Printf( "          ENUM_CURRENT_SETTINGS\n" );
			PrintDevMode( currentDevmode );

			DEVMODE	registryDevmode = {};
			if ( !EnumDisplaySettings( device.DeviceName,ENUM_REGISTRY_SETTINGS, &registryDevmode ) ) {
				common->Printf( "ERROR:  EnumDisplaySettings(ENUM_CURRENT_SETTINGS) failed!\n" );
			}
			common->Printf( "          -------------------\n" );
			common->Printf( "          ENUM_CURRENT_SETTINGS\n" );
			PrintDevMode( registryDevmode );

			for ( int modeNum = 0 ; ; modeNum++ ) {
				DEVMODE	devmode = {};

				if ( !EnumDisplaySettings( device.DeviceName,modeNum, &devmode ) ) {
					break;
				}

				if ( devmode.dmBitsPerPel != 32 ) {
					continue;
				}
				if ( devmode.dmDisplayFrequency < 60 ) {
					continue;
				}
				if ( devmode.dmPelsHeight < 720 ) {
					continue;
				}
				common->Printf( "          -------------------\n" );
				common->Printf( "          modeNum             : %i\n", modeNum );
				PrintDevMode( devmode );
			}
		}
	}
	common->Printf( "\n" );
}

/*
====================
R_GetModeListForDisplay
====================
*/
bool R_GetModeListForDisplay( const int requestedDisplayNum, idList<vidMode_t> & modeList ) {
	modeList.Clear();

	//TODO

#if 0

	bool	verbose = false;

	for ( int displayNum = requestedDisplayNum; ; displayNum++ ) {
		DISPLAY_DEVICE	device;
		device.cb = sizeof( device );
		if ( !EnumDisplayDevices(
				0,			// lpDevice
				displayNum,
				&device,
				0 /* dwFlags */ ) ) {
			return false;
		}

		// get the monitor for this display
		if ( ! (device.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP ) ) {
			continue;
		}

		DISPLAY_DEVICE	monitor;
		monitor.cb = sizeof( monitor );
		if ( !EnumDisplayDevices(
				device.DeviceName,
				0,
				&monitor,
				0 /* dwFlags */ ) ) {
			continue;
		}

		DEVMODE	devmode;
		devmode.dmSize = sizeof( devmode );

		if ( verbose ) {
			common->Printf( "display device: %i\n", displayNum );
			common->Printf( "  DeviceName  : %s\n", device.DeviceName );
			common->Printf( "  DeviceString: %s\n", device.DeviceString );
			common->Printf( "  StateFlags  : 0x%x\n", device.StateFlags );
			common->Printf( "  DeviceID    : %s\n", device.DeviceID );
			common->Printf( "  DeviceKey   : %s\n", device.DeviceKey );
			common->Printf( "      DeviceName  : %s\n", monitor.DeviceName );
			common->Printf( "      DeviceString: %s\n", monitor.DeviceString );
			common->Printf( "      StateFlags  : 0x%x\n", monitor.StateFlags );
			common->Printf( "      DeviceID    : %s\n", monitor.DeviceID );
			common->Printf( "      DeviceKey   : %s\n", monitor.DeviceKey );
		}

		for ( int modeNum = 0 ; ; modeNum++ ) {
			if ( !EnumDisplaySettings( device.DeviceName,modeNum, &devmode ) ) {
				break;
			}

			if ( devmode.dmBitsPerPel != 32 ) {
				continue;
			}
			if ( ( devmode.dmDisplayFrequency != 60 ) && ( devmode.dmDisplayFrequency != 120 ) ) {
				continue;
			}
			if ( devmode.dmPelsHeight < 720 ) {
				continue;
			}
			if ( verbose ) {
				common->Printf( "          -------------------\n" );
				common->Printf( "          modeNum             : %i\n", modeNum );
				common->Printf( "          dmPosition.x        : %i\n", devmode.dmPosition.x );
				common->Printf( "          dmPosition.y        : %i\n", devmode.dmPosition.y );
				common->Printf( "          dmBitsPerPel        : %i\n", devmode.dmBitsPerPel );
				common->Printf( "          dmPelsWidth         : %i\n", devmode.dmPelsWidth );
				common->Printf( "          dmPelsHeight        : %i\n", devmode.dmPelsHeight );
				common->Printf( "          dmDisplayFixedOutput: %s\n", DMDFO( devmode.dmDisplayFixedOutput ) );
				common->Printf( "          dmDisplayFlags      : 0x%x\n", devmode.dmDisplayFlags );
				common->Printf( "          dmDisplayFrequency  : %i\n", devmode.dmDisplayFrequency );
			}
			vidMode_t mode;
			mode.width = devmode.dmPelsWidth;
			mode.height = devmode.dmPelsHeight;
			mode.displayHz = devmode.dmDisplayFrequency;
			modeList.AddUnique( mode );
		}
		if ( modeList.Num() > 0 ) {

			class idSort_VidMode : public idSort_Quick< vidMode_t, idSort_VidMode > {
			public:
				int Compare( const vidMode_t & a, const vidMode_t & b ) const {
					int wd = a.width - b.width;
					int hd = a.height - b.height;
					int fd = a.displayHz - b.displayHz;
					return ( hd != 0 ) ? hd : ( wd != 0 ) ? wd : fd;
				}
			};

			// sort with lowest resolution first
			modeList.SortWithTemplate( idSort_VidMode() );

			return true;
		}
	}
	// Never gets here
#endif
}

/*
===================
QNXGLimp_CreateWindow
===================
*/
bool QNXGLimp_CreateWindow( int x, int y, int width, int height, int fullScreen ) {

	const int screenFormat = SCREEN_FORMAT_RGBA8888;
#ifdef ID_QNX_X86
	const int screenUsage = SCREEN_USAGE_OPENGL_ES2;
#else
	const int screenUsage = SCREEN_USAGE_DISPLAY | SCREEN_USAGE_OPENGL_ES2; // Physical device copy directly into physical display
#endif

	if ( screen_create_context( &qnx.screenCtx, 0 ) != 0 ) {
		common->Printf( "Could not create screen context\n" );
		return false;
	}

	if ( screen_create_window( &qnx.screenWin, qnx.screenCtx ) != 0 ) {
		common->Printf( "Could not create screen window\n" );
		return false;
	}

	if ( screen_set_window_property_iv( qnx.screenWin, SCREEN_PROPERTY_FORMAT, &screenFormat ) != 0 ) {
		common->Printf( "Could not set window format\n" );
		return false;
	}

	if ( screen_set_window_property_iv( qnx.screenWin, SCREEN_PROPERTY_USAGE, &screenUsage ) != 0 ) {
		common->Printf( "Could not set window usage\n" );
		return false;
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

	// Change display if determined
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
		screen_display_t display = displayList[ fullScreen - 1 ];
		Mem_Free( ( void* )displayList );

		if ( screen_set_window_property_pv( qnx.screenWin, SCREEN_PROPERTY_DISPLAY, ( void** )&display ) != 0 ) {
			common->Printf( "Could not set window size\n" );
			return false;
		}
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
bool QNXGLimp_CreateEGLConfig( int multisamples ) {
	EGLint eglConfigAttrs[] =
	{
		EGL_SAMPLE_BUFFERS,     ( ( multisamples >= 1 ) ? 1 : 0 ),
		EGL_SAMPLES,            multisamples,
		EGL_RED_SIZE,           8,
		EGL_GREEN_SIZE,         8,
		EGL_BLUE_SIZE,          8,
		EGL_ALPHA_SIZE,         8,
		EGL_DEPTH_SIZE,         24,
		EGL_STENCIL_SIZE,       8,
		EGL_SURFACE_TYPE,       EGL_WINDOW_BIT,
		EGL_RENDERABLE_TYPE,    EGL_OPENGL_ES2_BIT,
		EGL_NONE
	};

	EGLint eglConfigCount;
	bool success = true;

	if ( qeglChooseConfig( qnx.eglDisplay, eglConfigAttrs, &qnx.eglConfig, 1, &eglConfigCount ) != EGL_TRUE || eglConfigCount == 0 ) {
		success = false;
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
	}

	return success;
}

/*
===================
QNXGLimp_CreateEGLSurface
===================
*/
bool QNXGLimp_CreateEGLSurface( EGLNativeWindowType window, const int multisamples ) {
	common->Printf( "Initializing OpenGL driver\n" );

	EGLint eglContextAttrs[] =
	{
		EGL_CONTEXT_CLIENT_VERSION,    0,
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

	qnx.eglDisplay = qeglGetDisplay( EGL_DEFAULT_DISPLAY );
	if ( qnx.eglDisplay == EGL_NO_DISPLAY ) {
		common->Printf( "...^3Could not get EGL display^0\n");
		return false;
	}

	if ( qeglInitialize( qnx.eglDisplay, NULL, NULL ) != EGL_TRUE ) {
		common->Printf( "...^3Could not initialize EGL display^0\n");
		return false;
	}

	if ( !QNXGLimp_CreateEGLConfig( multisamples ) ) {
		common->Printf( "...^3Could not determine EGL config^0\n");
		return false;
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
			continue;
		}
		break;
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
===================
*/
bool GLimp_Init( glimpParms_t parms ) {
	const char	*driverName;

	// Some early checks to make sure supported params exist
	if ( parms.fullScreen == 0 ) {
		return false;
	}

	cmdSystem->AddCommand( "testSwapBuffers", GLimp_TestSwapBuffers, CMD_FL_SYSTEM, "Times swapbuffer options" );

	common->Printf( "Initializing OpenGL subsystem with multisamples:%i stereo:%i\n",
			parms.multiSamples, parms.stereo );

	// this will load the dll and set all our qgl* function pointers,
	// but doesn't create a window

	// r_glDriver is only intended for using instrumented OpenGL
	// so-s.  Normal users should never have to use it, and it is
	// not archived.
	driverName = r_glDriver.GetString()[0] ? r_glDriver.GetString() : "libGLESv2.so";
	if ( !QGL_Init( driverName ) ) {
		common->Printf( "^3GLimp_Init() could not load r_glDriver \"%s\"^0\n", driverName );
		return false;
	}

	if ( !QNXGLimp_CreateWindow( parms.x, parms.y, parms.width, parms.height, parms.fullScreen ) ) {
		GLimp_Shutdown();
		return false;
	}

	if ( !QNXGLimp_CreateEGLSurface( qnx.screenWin, parms.multiSamples ) ) {
		GLimp_Shutdown();
		return false;
	}

	r_swapInterval.SetModified();	// force a set next frame
	glConfig.swapControlTearAvailable = false;
	glConfig.stereoPixelFormatAvailable = false;

	//TODO
	glConfig.colorBits = win32.pfd.cColorBits;
	glConfig.depthBits = win32.pfd.cDepthBits;
	glConfig.stencilBits = win32.pfd.cStencilBits;

	//XXX Is rotation swapping needed for width/height and physicalScreenWidth?
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
			glConfig.physicalScreenWidthInCentimeters = size[0] / 10.0f; //MM to CM
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
	//TODO
	/*
	// Optionally ChangeDisplaySettings to get a different fullscreen resolution.
	if ( !GLW_ChangeDislaySettingsIfNeeded( parms ) ) {
		return false;
	}

	int x, y, w, h;
	if ( !GLW_GetWindowDimensions( parms, x, y, w, h ) ) {
		return false;
	}

	int exstyle;
	int stylebits;

	if ( parms.fullScreen ) {
		exstyle = WS_EX_TOPMOST;
		stylebits = WS_POPUP|WS_VISIBLE|WS_SYSMENU;
	} else {
		exstyle = 0;
		stylebits = WINDOW_STYLE|WS_SYSMENU;
	}

	SetWindowLong( win32.hWnd, GWL_STYLE, stylebits );
	SetWindowLong( win32.hWnd, GWL_EXSTYLE, exstyle );
	SetWindowPos( win32.hWnd, parms.fullScreen ? HWND_TOPMOST : HWND_NOTOPMOST, x, y, w, h, SWP_SHOWWINDOW );

	glConfig.isFullscreen = parms.fullScreen;
	glConfig.pixelAspect = 1.0f;	// FIXME: some monitor modes may be distorted

	glConfig.isFullscreen = parms.fullScreen;
	glConfig.nativeScreenWidth = parms.width;
	glConfig.nativeScreenHeight = parms.height;

	return true;
	*/
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

	//XXX cleanup render thread handle...

	// shutdown QGL subsystem
	QGL_Shutdown();
}

/*
=====================
GLimp_SwapBuffers
=====================
*/
void GLimp_SwapBuffers() {
	if ( r_swapInterval.IsModified() ) {
		r_swapInterval.ClearModified();

		int interval = 0;
		if ( r_swapInterval.GetInteger() == 1 ) {
			interval = ( glConfig.swapControlTearAvailable ) ? -1 : 1;
		} else if ( r_swapInterval.GetInteger() == 2 ) {
			interval = 1;
		}

		qeglSwapInterval( qnx.eglDisplay, interval );
	}

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
	//TODO
	/*
	if ( !qwglMakeCurrent( win32.hDC, win32.hGLRC ) ) {
		win32.wglErrors++;
	}
	*/
}

/*
===================
GLimp_DeactivateContext
===================
*/
void GLimp_DeactivateContext() {
	qglFinish();
	qeglMakeCurrent( qnx.eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT );
	//TODO
	/*
	qglFinish();
	if ( !qwglMakeCurrent( win32.hDC, NULL ) ) {
		win32.wglErrors++;
	}
	*/
}

/*
===================
GLimp_RenderThreadWrapper
===================
*/
static void GLimp_RenderThreadWrapper() {
	//TODO
	/*
	win32.glimpRenderThread();

	// unbind the context before we die
	qwglMakeCurrent( win32.hDC, NULL );
	*/
}

/*
=======================
GLimp_SpawnRenderThread

Returns false if the system only has a single processor
=======================
*/
bool GLimp_SpawnRenderThread( void (*function)() ) {
	//TODO
	/*
	SYSTEM_INFO info;

	// check number of processors
	GetSystemInfo( &info );
	if ( info.dwNumberOfProcessors < 2 ) {
		return false;
	}

	// create the IPC elements
	win32.renderCommandsEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
	win32.renderCompletedEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
	win32.renderActiveEvent = CreateEvent( NULL, TRUE, FALSE, NULL );

	win32.glimpRenderThread = function;

	win32.renderThreadHandle = CreateThread(
	   NULL,	// LPSECURITY_ATTRIBUTES lpsa,
	   0,		// DWORD cbStack,
	   (LPTHREAD_START_ROUTINE)GLimp_RenderThreadWrapper,	// LPTHREAD_START_ROUTINE lpStartAddr,
	   0,			// LPVOID lpvThreadParm,
	   0,			//   DWORD fdwCreate,
	   &win32.renderThreadId );

	if ( !win32.renderThreadHandle ) {
		common->Error( "GLimp_SpawnRenderThread: failed" );
	}

	SetThreadPriority( win32.renderThreadHandle, THREAD_PRIORITY_ABOVE_NORMAL );
#if 0
	// make sure they always run on different processors
	SetThreadAffinityMask( GetCurrentThread, 1 );
	SetThreadAffinityMask( win32.renderThreadHandle, 2 );
#endif

	return true;
	*/
}


//#define	DEBUG_PRINTS

/*
===================
GLimp_BackEndSleep
===================
*/
void *GLimp_BackEndSleep() {
	//TODO
	/*
	void	*data;

#ifdef DEBUG_PRINTS
OutputDebugString( "-->GLimp_BackEndSleep\n" );
#endif
	ResetEvent( win32.renderActiveEvent );

	// after this, the front end can exit GLimp_FrontEndSleep
	SetEvent( win32.renderCompletedEvent );

	WaitForSingleObject( win32.renderCommandsEvent, INFINITE );

	ResetEvent( win32.renderCompletedEvent );
	ResetEvent( win32.renderCommandsEvent );

	data = win32.smpData;

	// after this, the main thread can exit GLimp_WakeRenderer
	SetEvent( win32.renderActiveEvent );

#ifdef DEBUG_PRINTS
OutputDebugString( "<--GLimp_BackEndSleep\n" );
#endif
	return data;
	*/
}

/*
===================
GLimp_FrontEndSleep
===================
*/
void GLimp_FrontEndSleep() {
	//TODO
	/*
#ifdef DEBUG_PRINTS
OutputDebugString( "-->GLimp_FrontEndSleep\n" );
#endif
	WaitForSingleObject( win32.renderCompletedEvent, INFINITE );

#ifdef DEBUG_PRINTS
OutputDebugString( "<--GLimp_FrontEndSleep\n" );
#endif
	*/
}

//volatile bool	renderThreadActive;

/*
===================
GLimp_WakeBackEnd
===================
*/
void GLimp_WakeBackEnd( void *data ) {
	//TODO
	/*
	int		r;

#ifdef DEBUG_PRINTS
OutputDebugString( "-->GLimp_WakeBackEnd\n" );
#endif
	win32.smpData = data;

	if ( renderThreadActive ) {
		common->FatalError( "GLimp_WakeBackEnd: already active" );
	}

	r = WaitForSingleObject( win32.renderActiveEvent, 0 );
	if ( r == WAIT_OBJECT_0 ) {
		common->FatalError( "GLimp_WakeBackEnd: already signaled" );
	}

	r = WaitForSingleObject( win32.renderCommandsEvent, 0 );
	if ( r == WAIT_OBJECT_0 ) {
		common->FatalError( "GLimp_WakeBackEnd: commands already signaled" );
	}

	// after this, the renderer can continue through GLimp_RendererSleep
	SetEvent( win32.renderCommandsEvent );

	r = WaitForSingleObject( win32.renderActiveEvent, 5000 );

	if ( r == WAIT_TIMEOUT ) {
		common->FatalError( "GLimp_WakeBackEnd: WAIT_TIMEOUT" );
	}

#ifdef DEBUG_PRINTS
OutputDebugString( "<--GLimp_WakeBackEnd\n" );
#endif
	 */
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

	if ( !proc ) {
		common->Printf( "Couldn't find proc address for: %s\n", name );
	}

	return proc;
}
