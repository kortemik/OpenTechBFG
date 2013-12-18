/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.
Copyright (C) 2013 Vincent Simonetti
Copyright (C) 2013 Robert Beckebans

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

#include <errno.h>
#include <float.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <dlfcn.h>
#include <dirent.h>
#include <fnmatch.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <bps/bps.h>
#include <bps/navigator_invoke.h>
#include <bps/deviceinfo.h>
#include <bps/battery.h>
#if BBNDK_VERSION_AT_LEAST(10, 2, 0)
#include <bps/removablemedia.h>
#endif

#include <clipboard/clipboard.h>

#include <sys/slog2.h>

#include "../sys_local.h"
#include "qnx_local.h"
#include "../../renderer/tr_local.h"

idCVar QNXVars_t::sys_arch( "sys_arch", "", CVAR_SYSTEM | CVAR_INIT, "" );
idCVar QNXVars_t::sys_cpustring( "sys_cpustring", "detect", CVAR_SYSTEM | CVAR_INIT, "" );
//XXX

QNXVars_t	qnx;

static char		sys_cmdline[MAX_STRING_CHARS];

static sysMemoryStats_t appLaunchMemoryStats;

/*
================
Sys_GetExeLaunchMemoryStatus
================
*/
void Sys_GetExeLaunchMemoryStatus( sysMemoryStats_t &stats ) {
	stats = appLaunchMemoryStats;
}

/*
=============
Sys_Error

Show the early console as an error dialog
=============
*/
void Sys_Error( const char *error, ... ) {
	va_list		argptr;
	char		text[4096];
	bps_event_t	*event = NULL;
	int 		ret;

	va_start( argptr, error );
	vsprintf( text, error, argptr );
	va_end( argptr);

	extern idCVar com_productionMode;
	if ( com_productionMode.GetInteger() == 0 ) {

		// Only way for invoke to work is if there is a screen displayed,
		// which requires graphics to be init. and at least one swap buffers to have occured.
		if ( R_IsInitialized() ) {
			void GLimp_SwapBuffers();
			GLimp_SwapBuffers();
		}

		// Invoke email card
		EmailCrashReport( text );

		// wait for the user to quit
		while ( 1 ) {
			if ( ( ret = bps_get_event( &event, 1 ) ) == BPS_SUCCESS && event ) {
				if ( bps_event_get_domain( event ) == navigator_get_domain() ) {
					switch( bps_event_get_code( event ) ) {
					case NAVIGATOR_CHILD_CARD_CLOSED:
					case NAVIGATOR_EXIT:
						common->Quit();
						break;
					}
				}
			}
		}
	}

	GLimp_Shutdown();

	exit( 1 );
}

/*
========================
Sys_Launch
========================
*/
void Sys_Launch( const char * path, idCmdArgs & args,  void * data, unsigned int dataSize ) {

	/* TODO
	TCHAR				szPathOrig[_MAX_PATH];
	STARTUPINFO			si;
	PROCESS_INFORMATION	pi;

	ZeroMemory( &si, sizeof(si) );
	si.cb = sizeof(si);

	strcpy( szPathOrig, va( "\"%s\" %s", Sys_EXEPath(), (const char *)data ) );

	if ( !CreateProcess( NULL, szPathOrig, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi ) ) {
		idLib::Error( "Could not start process: '%s' ", szPathOrig );
		return;
	}
	cmdSystem->AppendCommandText( "quit\n" );
	*/
}

/*
========================
Sys_GetCmdLine
========================
*/
const char * Sys_GetCmdLine() {
	return sys_cmdline;
}

/*
========================
Sys_ReLaunch
========================
*/
void Sys_ReLaunch( void * data, const unsigned int dataSize ) {
	/* TODO
	TCHAR				szPathOrig[MAX_PRINT_MSG];
	STARTUPINFO			si;
	PROCESS_INFORMATION	pi;

	ZeroMemory( &si, sizeof(si) );
	si.cb = sizeof(si);

	strcpy( szPathOrig, va( "\"%s\" %s", Sys_EXEPath(), (const char *)data ) );

	CloseHandle( hProcessMutex );

	if ( !CreateProcess( NULL, szPathOrig, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi ) ) {
		idLib::Error( "Could not start process: '%s' ", szPathOrig );
		return;
	}
	cmdSystem->AppendCommandText( "quit\n" );
	*/
}

/*
==============
Sys_Quit
==============
*/
void Sys_Quit() {
	exit( 0 );
}


/*
==============
Sys_Printf
==============
*/
#define MAXPRINTMSG 4096
void Sys_Printf( const char *fmt, ... ) {
	char		msg[MAXPRINTMSG];

	va_list argptr;
	va_start(argptr, fmt);
	idStr::vsnPrintf( msg, MAXPRINTMSG-1, fmt, argptr );
	va_end(argptr);
	msg[sizeof(msg)-1] = '\0';

	slog2c( NULL, SLOG_CODE, SLOG2_INFO, msg );
}

/*
==============
Sys_DebugPrintf
==============
*/
#define MAXPRINTMSG 4096
void Sys_DebugPrintf( const char *fmt, ... ) {
	char msg[MAXPRINTMSG];

	va_list argptr;
	va_start( argptr, fmt );
	idStr::vsnPrintf( msg, MAXPRINTMSG-1, fmt, argptr );
	msg[ sizeof(msg)-1 ] = '\0';
	va_end( argptr );

	slog2c( NULL, SLOG_CODE, SLOG2_DEBUG1, msg );
}

/*
==============
Sys_DebugVPrintf
==============
*/
void Sys_DebugVPrintf( const char *fmt, va_list arg ) {
	char msg[MAXPRINTMSG];

	idStr::vsnPrintf( msg, MAXPRINTMSG-1, fmt, arg );
	msg[ sizeof(msg)-1 ] = '\0';

	slog2c( NULL, SLOG_CODE, SLOG2_DEBUG1, msg );
}

/*
==============
Sys_Sleep
==============
*/
void Sys_Sleep( int msec ) {
	struct timespec tm;
	nsec2timespec( &tm, msec * 1000000 ); //XXX Should this be done by hand?
	nanosleep( &tm, NULL );
}

/*
==============
Sys_ShowWindow
==============
*/
void Sys_ShowWindow( bool show ) {
	//XXX ::ShowWindow( win32.hWnd, show ? SW_SHOW : SW_HIDE );
}

/*
==============
Sys_IsWindowVisible
==============
*/
bool Sys_IsWindowVisible() {
	return ( qnx.windowState == NAVIGATOR_WINDOW_FULLSCREEN );
}

/*
==================
Sys_ShowConsole
==================
*/
void Sys_ShowConsole( int visLevel, bool quitOnClose ) {
}

/*
==============
Sys_Mkdir
==============
*/
void Sys_Mkdir( const char *path ) {
	mkdir( path, S_IRWXU | S_IRGRP | S_IWGRP );
}

/*
=================
Sys_FileTimeStamp
=================
*/
ID_TIME_T Sys_FileTimeStamp( idFileHandle fp ) {
	struct stat st;
	fstat( fp, &st );
	return st.st_mtime;
}

/*
========================
Sys_Rmdir
========================
*/
bool Sys_Rmdir( const char *path ) {
	return rmdir( path ) == 0;
}

/*
========================
Sys_IsFileWritable
========================
*/
bool Sys_IsFileWritable( const char *path ) {
	struct stat st;
	if ( stat( path, &st ) == -1 ) {
		return true;
	}
	return ( st.st_mode & S_IWRITE ) != 0;
}

/*
========================
Sys_IsFolder
========================
*/
sysFolder_t Sys_IsFolder( const char *path ) {
	struct stat buffer;
	if ( stat( path, &buffer ) < 0 ) {
		return FOLDER_ERROR;
	}
	return ( buffer.st_mode & _S_IFDIR ) != 0 ? FOLDER_YES : FOLDER_NO;
}

/*
==============
Sys_Cwd
==============
*/
const char *Sys_Cwd() {
	static char cwd[MAX_OSPATH];

	getcwd( cwd, sizeof( cwd ) - 1 );
	cwd[MAX_OSPATH-1] = 0;

	return cwd;
}

/*
==============
Sys_DefaultBasePath
==============
*/
const char *Sys_DefaultBasePath() {
	static char basePath[ PATH_MAX ] = { 0 };
	if ( basePath[0] == '\0' ) {
		memset( basePath, 0, PATH_MAX );

#if BBNDK_VERSION_AT_LEAST(10, 2, 0)
		removablemedia_info_t *infoMain;
		removablemedia_info_t *info;
		if ( removablemedia_get_info( &infoMain ) == BPS_SUCCESS ) {
			info = infoMain;
			while ( info ) {
				if ( removablemedia_info_get_presence( info ) == REMOVABLEMEDIA_PRESENCE_INSERTED &&
						!removablemedia_info_is_read_only( info ) && !removablemedia_info_is_write_protected( info ) ) {
					strcpy( basePath, removablemedia_info_get_mount_path( info ) );
					strcat( basePath, "/appdata/doom3bfg" );
					break;
				}
				info = removablemedia_info_get_next( info );
			}
			removablemedia_free_info( &infoMain );
		}
#endif
		if( basePath[0] == '\0' ) {
			strcpy( basePath, getenv( "PERIMETER_HOME" ) );
			strcat( basePath, "/removable/sdcard/appdata/doom3bfg" );
		}
	}

	return basePath;
}

/*
==============
Sys_DefaultSavePath
==============
*/
const char *Sys_DefaultSavePath() {
	static char savePath[ PATH_MAX ];
	memset( savePath, 0, PATH_MAX );

	strcpy( savePath, Sys_Cwd() );
	strcat( savePath, SAVE_PATH );

	return savePath;
}

/*
==============
Sys_AdditionalSearchPaths
==============
*/
void Sys_AdditionalSearchPaths( idStrList & paths ) {

	// Shared storage
	idStr str = Sys_Cwd();
	str += "/shared/misc/appdata/doom3bfg";
	paths.AddUnique( str );

	// App assets
	str = Sys_Cwd();
	str += "/app/native/assets";
	paths.AddUnique( str );

	// Not temp/not backed-up cache (**UNTESTED**)
	str = Sys_Cwd();
	str += "/cache";
	paths.AddUnique( str );
}

/*
==============
Sys_EXEPath
==============
*/
const char *Sys_EXEPath() {
	static char exe[ MAX_OSPATH ] = { 0 };
	if ( exe[0] == '\0' ) {
		int fd = open( "/proc/self/exefile", O_RDONLY );
		int pos = read( fd, exe, MAX_OSPATH - 1 );
		close( fd );
		if ( pos > 0 ) {
			exe[pos] = '\0';
		}
	}
	return exe;
}

/*
==============
Sys_ListFiles
==============
*/
int Sys_ListFiles( const char *directory, const char *extension, idStrList &list ) {
	// (very) slight modification of RBDOOM3's function
	struct dirent* d;
	DIR* fdir;
	bool dironly = false;
	char search[MAX_OSPATH];
	struct stat st;

	if ( !extension ) {
		extension = "";
	}

	list.Clear();

	// DG: we use fnmatch for shell-style pattern matching
	// so the pattern should at least contain "*" to match everything,
	// the extension will be added behind that (if !dironly)
	idStr pattern( "*" );

	// passing a slash as extension will find directories
	if( extension[0] == '/' && extension[1] == 0 ) {
		dironly = true;
	} else {
		// so we have *<extension>, the same as in the windows code basically
		pattern += extension;
	}
	// DG end

	// NOTE: case sensitivity of directory path can screw us up here
	if ( ( fdir = opendir( directory ) ) == NULL ) {
		return -1;
	}

	// DG: use readdir_r instead of readdir for thread safety
	// the following lines are from the readdir_r manpage.. fscking ugly.
	int nameMax = pathconf( directory, _PC_NAME_MAX );
	if ( nameMax == -1 )
		nameMax = 255;
	int direntLen = offsetof( struct dirent, d_name ) + nameMax + 1;

	struct dirent* entry = ( struct dirent* )Mem_Alloc( direntLen, TAG_CRAP );

	if ( entry == NULL ) {
		common->Warning( "Sys_ListFiles: Mem_Alloc for entry failed!" );
		closedir( fdir );
		return 0;
	}

	while ( readdir_r( fdir, entry, &d ) == 0 && d != NULL ) {
		// DG end
		idStr::snPrintf( search, sizeof( search ), "%s/%s", directory, d->d_name );
		if ( stat( search, &st ) == -1 )
			continue;
		if ( !dironly ) {
			// DG: the original code didn't work because d3 bfg abuses the extension
			// to match whole filenames and patterns in the savegame-code, not just file extensions...
			// so just use fnmatch() which supports matching shell wildcard patterns ("*.foo" etc)
			// if we should ever need case insensitivity, use FNM_CASEFOLD as third flag
			if( fnmatch( pattern.c_str(), d->d_name, 0 ) != 0 )
				continue;
			// DG end
		}
		if ( ( dironly && !( st.st_mode & S_IFDIR ) ) ||
				( !dironly && ( st.st_mode & S_IFDIR ) ) )
			continue;

		list.Append( d->d_name );
	}

	closedir( fdir );
	Mem_Free( entry );

	return list.Num();
}

/*
================
Sys_GetClipboardData
================
*/
char *Sys_GetClipboardData() {
	char *data = NULL;
	char *cliptext;

	if ( is_clipboard_format_present( "text/plain" ) == 0 ) {
		int size;
		if ( ( size = get_clipboard_data( "text/plain", &cliptext ) ) > 0 ) {
			data = (char *)Mem_Alloc( size + 1, TAG_CRAP );
			strncpy( data, cliptext, size );
			data[size] = '\0';
			free( cliptext );

			strtok( data, "\n\r\b" );
		}
	}

	return data;
}

/*
================
Sys_SetClipboardData
================
*/
void Sys_SetClipboardData( const char *string ) {
	// Check if we can write
	if ( string == NULL || get_clipboard_can_write() != 0 ) {
		return;
	}

	// Remove current clipboard contents
	empty_clipboard();

	// Write the data to the clipboard
	set_clipboard_data( "text/plain", strlen( string ) + 1, string );
}

/*
========================
ExecOutputFn
========================
*/
static void ExecOutputFn( const char * text ) {
	idLib::Printf( text );
}


/*
========================
Sys_Exec

if waitMsec is INFINITE, completely block until the process exits
If waitMsec is -1, don't wait for the process to exit
Other waitMsec values will allow the workFn to be called at those intervals.
========================
*/
bool Sys_Exec(	const char * appPath, const char * workingPath, const char * args,
	execProcessWorkFunction_t workFn, execOutputFunction_t outputFn, const int waitMS,
	unsigned int & exitCode ) {
	/* TODO
		exitCode = 0;
		SECURITY_ATTRIBUTES secAttr;
		secAttr.nLength = sizeof( SECURITY_ATTRIBUTES );
		secAttr.bInheritHandle = TRUE;
		secAttr.lpSecurityDescriptor = NULL;

		HANDLE hStdOutRead;
		HANDLE hStdOutWrite;
		CreatePipe( &hStdOutRead, &hStdOutWrite, &secAttr, 0 );
		SetHandleInformation( hStdOutRead, HANDLE_FLAG_INHERIT, 0 );

		HANDLE hStdInRead;
		HANDLE hStdInWrite;
		CreatePipe( &hStdInRead, &hStdInWrite, &secAttr, 0 );
		SetHandleInformation( hStdInWrite, HANDLE_FLAG_INHERIT, 0 );

		STARTUPINFO si;
		memset( &si, 0, sizeof( si ) );
		si.cb = sizeof( si );
		si.hStdError = hStdOutWrite;
		si.hStdOutput = hStdOutWrite;
		si.hStdInput = hStdInRead;
		si.wShowWindow = FALSE;
		si.dwFlags |= STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;

		PROCESS_INFORMATION pi;
		memset ( &pi, 0, sizeof( pi ) );

		if ( outputFn != NULL ) {
			outputFn( va( "^2Executing Process: ^7%s\n^2working path: ^7%s\n^2args: ^7%s\n", appPath, workingPath, args ) );
		} else {
			outputFn = ExecOutputFn;
		}

		// we duplicate args here so we can concatenate the exe name and args into a single command line
		const char * imageName = appPath;
		char * cmdLine = NULL;
		{
			// if we have any args, we need to copy them to a new buffer because CreateProcess modifies
			// the command line buffer.
			if ( args != NULL ) {
				if ( appPath != NULL ) {
					int len = idStr::Length( args ) + idStr::Length( appPath ) + 1 /* for space * / + 1 /* for NULL terminator * / + 2 /* app quotes * /;
					cmdLine = (char*)Mem_Alloc( len, TAG_TEMP );
					// note that we're putting quotes around the appPath here because when AAS2.exe gets an app path with spaces
					// in the path "w:/zion/build/win32/Debug with Inlines/AAS2.exe" it gets more than one arg for the app name,
					// which it most certainly should not, so I am assuming this is a side effect of using CreateProcess.
					idStr::snPrintf( cmdLine, len, "\"%s\" %s", appPath, args );
				} else {
					int len = idStr::Length( args ) + 1;
					cmdLine = (char*)Mem_Alloc( len, TAG_TEMP );
					idStr::Copynz( cmdLine, args, len );
				}
				// the image name should always be NULL if we have command line arguments because it is already
				// prefixed to the command line.
				imageName = NULL;
			}
		}

		BOOL result = CreateProcess( imageName, (LPSTR)cmdLine, NULL, NULL, TRUE, 0, NULL, workingPath, &si, &pi );

		if ( result == FALSE ) {
			TCHAR szBuf[1024];
			LPVOID lpMsgBuf;
			DWORD dw = GetLastError();

			FormatMessage(
				FORMAT_MESSAGE_ALLOCATE_BUFFER |
				FORMAT_MESSAGE_FROM_SYSTEM,
				NULL,
				dw,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				(LPTSTR) &lpMsgBuf,
				0, NULL );

			wsprintf( szBuf, "%d: %s", dw, lpMsgBuf );
			if ( outputFn != NULL ) {
				outputFn( szBuf );
			}
			LocalFree( lpMsgBuf );
			if ( cmdLine != NULL ) {
				Mem_Free( cmdLine );
			}
			return false;
		} else if ( waitMS >= 0 ) {	// if waitMS == -1, don't wait for process to exit
			DWORD ec = 0;
			DWORD wait = 0;
			char buffer[ 4096 ];
			for ( ; ; ) {
				wait = WaitForSingleObject( pi.hProcess, waitMS );
				GetExitCodeProcess( pi.hProcess, &ec );

				DWORD bytesRead = 0;
				DWORD bytesAvail = 0;
				DWORD bytesLeft = 0;
				BOOL ok = PeekNamedPipe( hStdOutRead, NULL, 0, NULL, &bytesAvail, &bytesLeft );
				if ( ok && bytesAvail != 0 ) {
					ok = ReadFile( hStdOutRead, buffer, sizeof( buffer ) - 3, &bytesRead, NULL );
					if ( ok && bytesRead > 0 ) {
						buffer[ bytesRead ] = '\0';
						if ( outputFn != NULL ) {
							int length = 0;
							for ( int i = 0; buffer[i] != '\0'; i++ ) {
								if ( buffer[i] != '\r' ) {
									buffer[length++] = buffer[i];
								}
							}
							buffer[length++] = '\0';
							outputFn( buffer );
						}
					}
				}

				if ( ec != STILL_ACTIVE ) {
					exitCode = ec;
					break;
				}

				if ( workFn != NULL ) {
					if ( !workFn() ) {
						TerminateProcess( pi.hProcess, 0 );
						break;
					}
				}
			}
		}

		// this assumes that windows duplicates the command line string into the created process's
		// environment space.
		if ( cmdLine != NULL ) {
			Mem_Free( cmdLine );
		}

		CloseHandle( pi.hProcess );
		CloseHandle( pi.hThread );
		CloseHandle( hStdOutRead );
		CloseHandle( hStdOutWrite );
		CloseHandle( hStdInRead );
		CloseHandle( hStdInWrite );
		return true;
	*/
	return false;
}

/*
========================================================================

DLL Loading

========================================================================
*/

/*
=====================
Sys_DLL_Load
=====================
*/
int Sys_DLL_Load( const char *dllName ) {
	void* libHandle = dlopen( dllName, RTLD_LAZY );
	return (int)libHandle;
}

/*
=====================
Sys_DLL_GetProcAddress
=====================
*/
void *Sys_DLL_GetProcAddress( int dllHandle, const char *procName ) {
	return dlsym( (void*)dllHandle, procName );
}

/*
=====================
Sys_DLL_Unload
=====================
*/
void Sys_DLL_Unload( int dllHandle ) {
	if ( !dllHandle ) {
		return;
	}
	int errorCode;
	if ( ( errorCode = dlclose( (void*)dllHandle ) ) != 0 ) {
		const char* error = dlerror();
		Sys_Error( "Sys_DLL_Unload: dlclose failed - %s (%d)", error, errorCode );
	}
}

/*
=================
Sys_In_Restart_f

Restart the input subsystem
=================
*/
void Sys_In_Restart_f( const idCmdArgs &args ) {
	Sys_ShutdownInput();
	Sys_InitInput();
}

/*
================
Sys_AlreadyRunning

returns true if there is a copy of D3 running already
================
*/
bool Sys_AlreadyRunning() {
	return false;
}

extern char* __progname;

/*
================
Sys_Init

The cvar system must already be setup
================
*/
void Sys_Init() {

	bps_initialize();

	//
	// Navigator and event setup
	//
	navigator_rotation_lock( true );

	if ( navigator_request_events( 0 ) != BPS_SUCCESS )
		Sys_Error( "Couldn't request navigator events" );
	battery_request_events( 0 );

	cmdSystem->AddCommand( "in_restart", Sys_In_Restart_f, CMD_FL_SYSTEM, "restarts the input system" );

	//
	// QNX version
	//
	deviceinfo_details_t *devDetails;
	if ( deviceinfo_get_details( &devDetails ) != BPS_SUCCESS )
		Sys_Error( "Couldn't get device info" );

	qnx.sys_arch.SetString( deviceinfo_details_get_device_os( devDetails ) );

	//
	// CPU type
	//
	if ( !idStr::Icmp( qnx.sys_cpustring.GetString(), "detect" ) ) {
		idStr string;

#if BBNDK_VERSION_AT_LEAST( 10, 2, 0 )
		common->Printf( "%d MHz ", deviceinfo_details_get_processor_core_speed( devDetails, 0 ) );
#else
		//common->Printf( "%1.0f MHz ", Sys_ClockTicksPerSecond() / 1000000.0f ); //Number ends up being ridiculously large ("28991029248 MHz")
#endif

		qnx.cpuid = Sys_GetCPUId();

		string.Clear();

#ifdef ID_QNX_X86

		if ( qnx.cpuid & CPUID_AMD ) {
			string += "AMD CPU";
		} else if ( qnx.cpuid & CPUID_INTEL ) {
			string += "Intel CPU";
		} else if ( qnx.cpuid & CPUID_UNSUPPORTED ) {
			string += "unsupported CPU";
		} else {
			string += "generic CPU";
		}

		string += " with ";
		if ( qnx.cpuid & CPUID_MMX ) {
			string += "MMX & ";
		}
		if ( qnx.cpuid & CPUID_3DNOW ) {
			string += "3DNow! & ";
		}
		if ( qnx.cpuid & CPUID_SSE ) {
			string += "SSE & ";
		}
		if ( qnx.cpuid & CPUID_SSE2 ) {
			string += "SSE2 & ";
		}
		if ( qnx.cpuid & CPUID_SSE3 ) {
			string += "SSE3 & ";
		}
		if ( qnx.cpuid & CPUID_HTT ) {
			string += "HTT & ";
		}

#elif defined(ID_QNX_ARM)

		if ( qnx.cpuid & CPUID_TI ) {
			string += "Texas Instruments CPU";
		} else if ( qnx.cpuid & CPUID_QC ) {
			string += "Qualcomm CPU";
		} else if ( qnx.cpuid & CPUID_UNSUPPORTED ) {
			string += "unsupported CPU";
		} else {
			string += "generic CPU";
		}

		string += " with ";
		if ( qnx.cpuid & CPUID_VFP ) {
			string += "VFP & ";
		}
		if ( qnx.cpuid & CPUID_NEON ) {
			string += "NEON & ";
		}
		if ( qnx.cpuid & CPUID_WMMX2 ) {
			string += "WMMX2 & ";
		}

#else
#error Unknown CPU architecture
#endif
		string.StripTrailing( " & " );
		string.StripTrailing( " with " );
		qnx.sys_cpustring.SetString( string );
	} else {
		common->Printf( "forcing CPU type to " );
		idLexer src( qnx.sys_cpustring.GetString(), idStr::Length( qnx.sys_cpustring.GetString() ), "sys_cpustring" );
		idToken token;

		int id = CPUID_NONE;
		while( src.ReadToken( &token ) ) {
			if ( token.Icmp( "generic" ) == 0 ) {
				id |= CPUID_GENERIC;
				continue;
			}
#ifdef ID_QNX_X86

			if ( token.Icmp( "intel" ) == 0 ) {
				id |= CPUID_INTEL;
			} else if ( token.Icmp( "amd" ) == 0 ) {
				id |= CPUID_AMD;
			} else if ( token.Icmp( "mmx" ) == 0 ) {
				id |= CPUID_MMX;
			} else if ( token.Icmp( "3dnow" ) == 0 ) {
				id |= CPUID_3DNOW;
			} else if ( token.Icmp( "sse" ) == 0 ) {
				id |= CPUID_SSE;
			} else if ( token.Icmp( "sse2" ) == 0 ) {
				id |= CPUID_SSE2;
			} else if ( token.Icmp( "sse3" ) == 0 ) {
				id |= CPUID_SSE3;
			} else if ( token.Icmp( "htt" ) == 0 ) {
				id |= CPUID_HTT;
			}

#elif defined(ID_QNX_ARM)

			if ( token.Icmp( "ti" ) == 0 ) {
				id |= CPUID_TI;
			} else if ( token.Icmp( "qualcomm" ) == 0 ) {
				id |= CPUID_QC;
			} else if ( token.Icmp( "vfp" ) == 0 ) {
				id |= CPUID_VFP;
			} else if ( token.Icmp( "neon" ) == 0 ) {
				id |= CPUID_NEON;
			} else if ( token.Icmp( "wmmx2" ) == 0 ) {
				id |= CPUID_WMMX2;
			}

#else
#error Unknown CPU architecture
#endif
		}
		if ( id == CPUID_NONE ) {
			common->Printf( "WARNING: unknown sys_cpustring '%s'\n", qnx.sys_cpustring.GetString() );
			id = CPUID_GENERIC;
		}
		qnx.cpuid = (cpuid_t) id;
	}

	if ( deviceinfo_details_is_simulator( devDetails ) ) {
		common->Printf( "Running in Simulator\n" );
	}

	deviceinfo_free_details( &devDetails );

	common->Printf( "%s\n", qnx.sys_cpustring.GetString() );
	common->Printf( "%d MB System Memory\n", Sys_GetSystemRam() );
	//common->Printf( "%d MB Video Memory\n", Sys_GetVideoRam() );
#ifdef ID_QNX_X86
	if ( ( qnx.cpuid & CPUID_SSE2 ) == 0 ) {
		common->Error( "SSE2 not supported!" );
	}
#elif defined(ID_QNX_ARM)
	if ( ( qnx.cpuid & CPUID_NEON ) == 0 ) {
		common->Error( "NEON not supported!" );
	}
#else
#error Unknown CPU architecture
#endif
}

/*
================
Sys_Shutdown
================
*/
void Sys_Shutdown() {
	Sys_ShutdownInput();
	battery_stop_events( 0 );
	navigator_stop_events( 0 );
	bps_shutdown();
}

/*
================
Sys_GetProcessorId
================
*/
cpuid_t Sys_GetProcessorId() {
	return qnx.cpuid;
}

/*
================
Sys_GetProcessorString
================
*/
const char *Sys_GetProcessorString() {
	return qnx.sys_cpustring.GetString();
}

//=======================================================================

/*
====================
EmailCrashReport

  emailer originally from Raven/Quake 4
====================
*/
bool EmailCrashReport( const char* messageText ) {
	static int lastEmailTime = 0;

	if ( Sys_Milliseconds() < lastEmailTime + 10000 ) {
		return false;
	}

	lastEmailTime = Sys_Milliseconds();

	idStr format;
	format.Format( "data:json:{\"to\":\"%s\",\"subject\":\"%s\",\"body\":\"%s%s%s\"}\n",
			SUPPORT_EMAIL_ADDRESS, SUPPORT_EMAIL_SUBJECT, SUPPORT_EMAIL_BODY_PRE, messageText, SUPPORT_EMAIL_BODY_POST );

	navigator_invoke_invocation_t *invoke = NULL;

	navigator_invoke_invocation_create( &invoke );
	navigator_invoke_invocation_set_action( invoke, "bb.action.COMPOSE" );
	navigator_invoke_invocation_set_target( invoke, "sys.pim.uib.email.hybridcomposer" );
	navigator_invoke_invocation_set_type( invoke, "message/rfc822" );
	navigator_invoke_invocation_set_data( invoke, format.c_str(), format.Length() + 1 );

	bool ret = navigator_invoke_invocation_send( invoke ) == BPS_SUCCESS;

	navigator_invoke_invocation_destroy( invoke );

	return ret;
}

#define TEST_FPU_EXCEPTIONS	/*	FPU_EXCEPTION_INVALID_OPERATION |		*/	\
							/*	FPU_EXCEPTION_DENORMALIZED_OPERAND |	*/	\
							/*	FPU_EXCEPTION_DIVIDE_BY_ZERO |			*/	\
							/*	FPU_EXCEPTION_NUMERIC_OVERFLOW |		*/	\
							/*	FPU_EXCEPTION_NUMERIC_UNDERFLOW |		*/	\
							/*	FPU_EXCEPTION_INEXACT_RESULT |			*/	\
								0

/*
==================
main
==================
*/
int main( int argc, char** argv ) {

	// SLOG2 setup
	slog2_buffer_set_config_t buffer_config;

	// Setup log info
	buffer_config.buffer_set_name = __progname;
	buffer_config.num_buffers = SLOG_BUFFER_COUNT;
	buffer_config.verbosity_level = SLOG2_DEBUG1;

	// Setup each buffer
	buffer_config.buffer_config[0].buffer_name = "doom3";
	buffer_config.buffer_config[0].num_pages = 9;

	// Register
	if ( slog2_register( &buffer_config, qnx.logBuffers, 0 ) == -1 )
		Sys_Error( "Couldn't setup SLOG2" );

	// Set the default log
	slog2_set_default_buffer( qnx.logBuffers[0] );

	// setup memory
	Sys_SetPhysicalWorkMemory( 192 << 20, 1024 << 20 );

	Sys_GetCurrentMemoryStatus( appLaunchMemoryStats );

	// build command line
	{
		idCmdArgs args;
		for ( int i = 1; i < argc; i++ ) {
			args.AppendArg( argv[i] );
		}
		idStr::Copynz( sys_cmdline, args.Args( 0, -1 ), sizeof( sys_cmdline ) );
	}

	// get the initial time base
	Sys_Milliseconds();

//	Sys_FPU_EnableExceptions( TEST_FPU_EXCEPTIONS );
//	Sys_FPU_SetPrecision( FPU_PRECISION_DOUBLE_EXTENDED ); // ARM doesn't support this precision

	// check for data, copy if it doesn't exist
	//TODO (need to check that shared file access is allowed)

	// initialize system
	if ( argc > 1 ) {
		common->Init( argc - 1, &argv[1], NULL );
	} else {
		common->Init( 0, NULL, NULL );
	}

#if TEST_FPU_EXCEPTIONS != 0
	common->Printf( Sys_FPU_GetState() );
#endif

	// main game loop
	while( 1 ) {
		// set exceptions, even if some crappy syscall changes them!
		// Sys_FPU_EnableExceptions( TEST_FPU_EXCEPTIONS );

		// run the game
		common->Frame();
	}

	// never gets here
	return 0;
}

/*
==================
idSysLocal::OpenURL
==================
*/
void idSysLocal::OpenURL( const char *url, bool doexit ) {
	static bool doexit_spamguard = false;
	navigator_invoke_invocation_t* invoke = NULL;

	if (doexit_spamguard) {
		common->DPrintf( "OpenURL: already in an exit sequence, ignoring %s\n", url );
		return;
	}

	common->Printf("Open URL: %s\n", url);

	navigator_invoke_invocation_create( &invoke );
	navigator_invoke_invocation_set_action( invoke, "bb.action.OPEN" );
	navigator_invoke_invocation_set_uri( invoke, url );

	//This will open a card if it's not a browser URL. The card will be closed when the app exits
	int ret = navigator_invoke_invocation_send( invoke );

	navigator_invoke_invocation_destroy( invoke );

	if ( ret != BPS_SUCCESS ) {
		common->Error( "Could not open url: '%s' ", url );
		return;
	}

	if ( doexit ) {
		doexit_spamguard = true;
		cmdSystem->BufferCommandText( CMD_EXEC_APPEND, "quit\n" );
	}
}

/*
==================
idSysLocal::StartProcess
==================
*/
void idSysLocal::StartProcess( const char *exePath, bool doexit ) {
	/* TODO
	TCHAR				szPathOrig[_MAX_PATH];
	STARTUPINFO			si;
	PROCESS_INFORMATION	pi;

	ZeroMemory( &si, sizeof(si) );
	si.cb = sizeof(si);

	strncpy( szPathOrig, exePath, _MAX_PATH );

	if( !CreateProcess( NULL, szPathOrig, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi ) ) {
        common->Error( "Could not start process: '%s' ", szPathOrig );
	    return;
	}

	if ( doexit ) {
		cmdSystem->BufferCommandText( CMD_EXEC_APPEND, "quit\n" );
	}
	*/
}

/*
==================
Sys_SetFatalError
==================
*/
void Sys_SetFatalError( const char *error ) {
}

/*
================
Sys_SetLanguageFromSystem
================
*/
extern idCVar sys_lang;
void Sys_SetLanguageFromSystem() {
	sys_lang.SetString( Sys_DefaultLanguage() );
}
