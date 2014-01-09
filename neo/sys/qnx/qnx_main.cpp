/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.
Copyright (C) 2014 Vincent Simonetti
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
#include <spawn.h>
#include <signal.h>
#include <process.h>

#include <bps/bps.h>
#include <bps/navigator_invoke.h>
#include <bps/deviceinfo.h>
#include <bps/battery.h>
#if BBNDK_VERSION_AT_LEAST(10, 2, 0)
#include <bps/removablemedia.h>
#endif
#include <bps/locale.h>

#include <clipboard/clipboard.h>

#include <sys/procmgr.h>
#include <sys/slog2.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/pps.h>

#include "../sys_local.h"
#include "qnx_local.h"
#include "../../renderer/tr_local.h"

idCVar QNXVars_t::sys_arch( "sys_arch", "", CVAR_SYSTEM | CVAR_INIT, "" );
idCVar QNXVars_t::sys_cpustring( "sys_cpustring", "detect", CVAR_SYSTEM | CVAR_INIT, "" );
#ifndef ID_RETAIL
idCVar QNXVars_t::qnx_errorAttachLogs( "qnx_errorAttachLogs", "1", CVAR_SYSTEM | CVAR_ARCHIVE | CVAR_INTEGER, "1 - attach slog2 files only, 2 - parse slog2 files before sending", 0, 2 );
#else
idCVar QNXVars_t::qnx_errorAttachLogs( "qnx_errorAttachLogs", "0", CVAR_SYSTEM | CVAR_ARCHIVE | CVAR_BOOL, "attach slog2 logs on error" );
#endif

QNXVars_t				qnx;

extern char 			*__progname;

static char				sys_cmdline[MAX_STRING_CHARS];

static sysMemoryStats_t	appLaunchMemoryStats;

static bool				hasFatalError = false;

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

		if ( !R_IsInitialized() ) {
			qnx.errorGraphics = true;

			// Graphics wasn't init, at least set it up
			void R_SetNewMode( const bool fullInit );
			R_SetNewMode( true );
		}

		// Only way for invoke to work is if there is a screen displayed,
		// which requires graphics to be init. and at least one swap buffers to have occurred.
		void GLimp_SwapBuffers();
		GLimp_SwapBuffers();

		// Get error log
		const char *errorLog = NULL;
		bool removeLog = false;
		if ( qnx.qnx_errorAttachLogs.GetBool() ) {
			idStr path;

#ifndef ID_RETAIL
			switch ( qnx.qnx_errorAttachLogs.GetInteger() ) {
			case 1: {
				path.Format( "/tmp/slogger2/%s.%d", __progname, getpid() );
				int fd = open( path.c_str(), O_RDONLY );
				if ( fd >= 0 ) {
					errorLog = Mem_CopyString( path.c_str() );
					close( fd );
				}
				break;
			}

			case 2:
#endif
				FILE *destFile = fopen( "logs/error_msg.txt", "w" );
				if ( destFile ) {
					if ( slog2_dump_logs_to_file( destFile, 0 ) == 0 ) {
						path.Format( "%s/logs/error_msg.txt", Sys_CWD() );
						errorLog = Mem_CopyString( path.c_str() );
						removeLog = true;
					}
					fclose( destFile );
				}
#ifndef ID_RETAIL
				break;
			}
#endif
		}

		// Invoke email card and wait for the user to quit
		int attempts = 0;
		bool cardOpened = false;
		while ( 1 ) {
			if ( ( ret = bps_get_event( &event, 250 ) ) == BPS_SUCCESS && event ) {
				if ( bps_event_get_domain( event ) == navigator_get_domain() ) {
					switch( bps_event_get_code( event ) ) {
					case NAVIGATOR_CHILD_CARD_CLOSED:
					case NAVIGATOR_EXIT:
						if ( errorLog ) {
							if ( removeLog ) {
								remove( errorLog );
							}
							Mem_Free( ( void* )errorLog );
						}
						common->Quit();
						break;
					}
				}
			}
			if ( !cardOpened && attempts < 2 ) {
				cardOpened = EmailCrashReport( text, errorLog );
				attempts++;
				if ( !cardOpened ) {
					common->Printf( "Attempt #%d, Email card failed to open, trying again: %s\n", attempts, strerror( errno ) );
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
void Sys_Launch( const char * path, idCmdArgs & args, void * data, unsigned int dataSize ) {

#if __SIZEOF_POINTER__ != __SIZEOF_INT__
#error Pointer is not the same size as a pointer, meaning the method of retrieving the invocation won't work
#endif

	// Purely an invoke call

	bool success = false;

	// Try invocation
	navigator_invoke_invocation_t *invoke = NULL;

	navigator_invoke_invocation_create( &invoke );
	{
		idStr ptr;
		ptr.Format( "%p", invoke );
		navigator_invoke_invocation_set_id( invoke, ptr.c_str() );
	}
	navigator_invoke_invocation_set_action( invoke, "bb.action.OPEN" );
	success = navigator_invoke_invocation_set_target( invoke, path ) == BPS_SUCCESS;
	if ( success ) {
		idStr metadata;
		metadata.Format( "{\"type\":\"doom.sys.launch\", \"doexit\":true, \"args\":\"%s\"}", args.Args() );
		navigator_invoke_invocation_set_metadata( invoke, metadata.c_str() );

		if ( data && dataSize > 0 ) {
			navigator_invoke_invocation_set_data( invoke, data, dataSize );
		}

		success = navigator_invoke_invocation_send( invoke ) == BPS_SUCCESS;
	}

	if ( !success ) {
		navigator_invoke_invocation_destroy( invoke );
	}
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
#ifdef USE_EXEC_APP_RESTART
idCmdArgs relaunchArgs;
#endif
void Sys_ReLaunch( void * data, const unsigned int dataSize ) {

#ifdef USE_EXEC_APP_RESTART
	char newCmd[MAX_PRINT_MSG];
	strcpy( newCmd, va( "\"%s\" %s", Sys_EXEPath(), (const char *)data ) );

	relaunchArgs.TokenizeString( newCmd, true );
	cmdSystem->AppendCommandText( "quit\n" );
#else
	if ( qnx.dontShowRelaunchDialog ) {
		// Save the arguments
		int fd = open( "data/relaunch.txt", O_WRONLY | O_CREAT, S_IRWXU );
		if ( fd >= 0 ) {
			write( fd, data, dataSize );
			close( fd );
		} else {
			common->Error( "Sys_ReLaunch: Could not save args\n" );
		}

		cmdSystem->AppendCommandText( "quit\n" );
	} else {
		if ( qnx.dialog ) {
			common->Error( "Sys_ReLaunch: Dialog already open\n" );
		}
		if ( dialog_create_alert( &qnx.dialog ) != BPS_SUCCESS ) {
			common->Error( "Sys_ReLaunch: Could not create alert\n" );
		}
		// Text: Game will exit. Please run again to apply changes
		dialog_set_alert_message_text( qnx.dialog, idLocalization::FindString( "#str_01000001" ) );
		// Button: Exit Game
		dialog_add_button( qnx.dialog, idLocalization::FindString( "#str_01975" ), true, NULL, true );
#if BBNDK_VERSION_AT_LEAST(10, 2, 0)
		// Setup checkbox if the dialog should be shown or not
		dialog_set_alert_checkbox_enabled( qnx.dialog, true );
		dialog_set_alert_checkbox_checked( qnx.dialog, false );
		// Text: Do not show this popup again.
		dialog_set_alert_checkbox_label( qnx.dialog, idLocalization::FindString( "#str_01000002" ) );
#endif

		// Save the arguments
		int fd = open( "data/relaunch.txt", O_WRONLY | O_CREAT, S_IRWXU );
		if ( fd >= 0 ) {
			write( fd, data, dataSize );
			close( fd );
		} else {
			common->Error( "Sys_ReLaunch: Could not save args\n" );
		}

		// Show dialog if no error occurred when saving args
		if ( dialog_show( qnx.dialog ) != BPS_SUCCESS ) {
			common->Error( "Sys_ReLaunch: Could not show alert\n" );
		}
		qnx.dialogType = QnxDialog_Relaunch;
	}
#endif
}

/*
==============
Sys_Quit
==============
*/
void Sys_Quit() {
	if ( qnx.errorGraphics ) {
		GLimp_Shutdown();
	}
#ifdef USE_EXEC_APP_RESTART
	int argc;
	const char * const * argvTmp = relaunchArgs.GetArgs( &argc );
	if ( argc > 0 /*&& qnx.canSpawn*/ ) {
		char ** argv = ( char ** )_alloca( sizeof( char ** ) * ( argc + 1 ) );
		memcpy( &argv[1], argvTmp, sizeof( char ** ) * argc );
		argv[argc + 1] = NULL;

		// If this fails, it simply exits
		execv( argv[0], argv );
	}
#endif
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
	// Don't use usleep because it has a limit of 1000000 microseconds (1000 milliseconds)

	struct timespec tm;
	tm.tv_sec = msec / 1000;
	tm.tv_nsec = ( msec * 1000000ULL ) % 1000000000;
	//nsec2timespec( &tm, msec * 1000000ULL );
	nanosleep( &tm, NULL );
}

/*
==============
Sys_ShowWindow
==============
*/
void Sys_ShowWindow( bool show ) {
	if ( show ) {
		common->Warning( "Sys_ShowWindow: Maximizing a window is not supported\n" );
	} else {
		// From QQnxWindow (QNX implementation of Qt)

		pps_encoder_t encoder;

		pps_encoder_initialize( &encoder, false );
		pps_encoder_add_string( &encoder, "msg", "minimizeWindow" );

		if ( navigator_raw_write( pps_encoder_buffer( &encoder ), pps_encoder_length( &encoder ) ) != BPS_SUCCESS ) {
			common->Warning( "Sys_ShowWindow: Failed to minimize window\n" );
		}

		pps_encoder_cleanup( &encoder );
	}
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
Sys_CWD
==============
*/
const char *Sys_CWD() {
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

	strcpy( savePath, Sys_CWD() );
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
	idStr str = Sys_CWD();
	str += "/shared/misc/appdata/doom3bfg";
	paths.Append( str );

	// Not temp/not backed-up cache (**UNTESTED**)
	str = Sys_CWD();
	str += "/cache";
	paths.Append( str );

	// App assets
	str = Sys_CWD();
	str += "/app/native/assets";
	paths.Append( str );
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

	exitCode = 0;

	/*
	if ( !qnx.canSpawn ) {
		common->Warning( "Sys_Exec not supported by process manager\n" );
		return false;
	}
	*/

	const char* cwd = Sys_CWD();
	if ( chdir( workingPath ) != 0 ) {
		return false;
	}

	int stdOutPipe[2];
	pipe( stdOutPipe );

	int stdInPipe[2];
	pipe( stdInPipe );
	close( stdInPipe[1] );

	if ( outputFn != NULL ) {
		outputFn( va( "^2Executing Process: ^7%s\n^2working path: ^7%s\n^2args: ^7%s\n", appPath, workingPath, args ) );
	} else {
		outputFn = ExecOutputFn;
	}

	// Get arguments if they exist
	char ** argv = NULL;
	if ( args ) {
		int argc;
		idCmdArgs cmdArgs( args, true );
		const char * const * argvTmp = cmdArgs.GetArgs( &argc );

		// Copy over app path
		argv = ( char ** )Mem_Alloc( sizeof( char ** ) * ( argc + 2 ), TAG_TEMP );
		memcpy( &argv[1], argvTmp, sizeof( char ** ) * argc );
		argv[0] = ( char * )appPath;
		argv[argc + 1] = NULL;
	}

	// Setup and spawn process
	posix_spawnattr_t spawnAtt;
	uint32_t spawnFlags;
	posix_spawnattr_init( &spawnAtt );
	posix_spawnattr_getxflags( &spawnAtt, &spawnFlags );
	spawnFlags |= POSIX_SPAWN_SEARCH_PATH;
	posix_spawnattr_getxflags( &spawnAtt, &spawnFlags );

	// Evaluated first to last
	posix_spawn_file_actions_t spawnFile;
	posix_spawn_file_actions_init( &spawnFile );
	posix_spawn_file_actions_addclose( &spawnFile, stdOutPipe[0] );
	posix_spawn_file_actions_adddup2( &spawnFile, stdInPipe[0], 0 );	//stdin
	posix_spawn_file_actions_adddup2( &spawnFile, stdOutPipe[1], 1 );	//stdout
	posix_spawn_file_actions_adddup2( &spawnFile, stdOutPipe[1], 2 );	//stderr
	posix_spawn_file_actions_addclose( &spawnFile, stdInPipe[0] );
	posix_spawn_file_actions_addclose( &spawnFile, stdOutPipe[1] );

	pid_t pid;
	int ret = posix_spawnp( &pid, appPath, &spawnFile, &spawnAtt, argv, NULL );

	// Reset back to original CWD
	chdir( cwd );

	posix_spawn_file_actions_destroy( &spawnFile );
	posix_spawnattr_destroy( &spawnAtt );

	if ( ret != EOK ) {
		char buf[1024];
		const char* errMsg = strerror( ret );

		sprintf( buf, "%d: %s", ret, errMsg );
		if ( outputFn != NULL ) {
			outputFn( buf );
		}

		close( stdOutPipe[0] );
		close( stdOutPipe[1] );
		close( stdInPipe[0] );

		if ( argv ) {
			Mem_Free( ( void* )argv );
		}
		return false;
	} else if ( waitMS >= 0 ) {	// if waitMS == -1, don't wait for process to exit
		int statVal;
		char buffer[ 4096 ];
		for ( ; ; ) {
			if ( waitMS == idSysSignal::WAIT_INFINITE ) {
				waitpid( pid, &statVal, 0 );
			} else {
				// Sleep for a period of time while periodically checking for process exit
				int end = Sys_Milliseconds() + waitMS;
				int time;
				while ( 1 ) {
					waitpid( pid, &statVal, WNOHANG );
					time = Sys_Milliseconds();
					if ( time >= end || WIFEXITED( statVal ) ) {
						break;
					}
					// Only sleep for a short period of time
					time = end - time;
					usleep( __min( 100, __max( time, 0 ) ) * 1000 );
				}
			}

			int bytesRead = read( stdOutPipe[0], buffer, sizeof( buffer ) - 3 );
			if ( bytesRead > 0 ) {
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

			if ( WIFEXITED( statVal ) ) {
				exitCode = WEXITSTATUS( statVal );
				break;
			}

			if ( workFn != NULL ) {
				if ( !workFn() ) {
					kill( pid, SIGKILL );
					break;
				}
			}
		}
	}

	if ( argv ) {
		Mem_Free( ( void* )argv );
	}

	close( stdOutPipe[0] );
	close( stdOutPipe[1] );
	close( stdInPipe[0] );
	return true;
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

	if ( navigator_request_events( NAVIGATOR_EXTENDED_DATA ) != BPS_SUCCESS )
		Sys_Error( "Couldn't request navigator events" );
	battery_request_events( 0 );
#ifdef USE_EVENT_UPDATE_SYS_LANG
	locale_request_events( 0 );
#endif
#ifndef USE_EXEC_APP_RESTART
	dialog_request_events( 0 );
#endif

	cmdSystem->AddCommand( "in_restart", Sys_In_Restart_f, CMD_FL_SYSTEM, "restarts the input system" );

	//
	// process permissions setup
	//
	qnx.canSpawn	= procmgr_ability( 0, PROCMGR_ADN_NONROOT | PROCMGR_AOP_ALLOW | PROCMGR_AID_SPAWN,			PROCMGR_AID_EOL );
	qnx.canNewApp	= procmgr_ability( 0, PROCMGR_ADN_NONROOT | PROCMGR_AOP_ALLOW | PROCMGR_AID_CHILD_NEWAPP,	PROCMGR_AID_EOL );
	qnx.canLockMem	= procmgr_ability( 0, PROCMGR_ADN_NONROOT | PROCMGR_AOP_ALLOW | PROCMGR_AID_MEM_LOCK,		PROCMGR_AID_EOL );

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

	if ( ( qnx.isSimulator = deviceinfo_details_is_simulator( devDetails ) ) ) {
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

	//
	// other info
	//
	qnx.personalPerimeter = idStr::Icmp( getenv( "PERIMETER" ), "personal" ) == 0;
	if ( !qnx.personalPerimeter ) {
		common->Printf( "%s is not running under personal perimeter. Some functions may be disabled\n", GAME_NAME );
	}

	//
	// Determine if dialog shouldn't be shown
	//
	int fd = open( "data/relaunch_dialog.txt", O_RDONLY );
	if ( fd >= 0 ) {
		char tmp;
		if ( read( fd, &tmp, sizeof( char ) ) > 0 ) {
			qnx.dontShowRelaunchDialog = tmp == 'T';
		}
		close( fd );
	}
}

/*
================
Sys_Shutdown
================
*/
void Sys_Shutdown() {
	Sys_ShutdownInput();
#ifndef USE_EXEC_APP_RESTART
	if ( qnx.dialog ) {
		dialog_destroy( qnx.dialog );
		qnx.dialog = NULL;
	}
	dialog_stop_events( 0 );
#endif
#ifdef USE_EVENT_UPDATE_SYS_LANG
	locale_stop_events( 0 );
#endif
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
bool EmailCrashReport( const char *messageText, const char *errorLog ) {
	static int lastEmailTime = 0;

	if ( Sys_Milliseconds() < lastEmailTime + 10000 ) {
		return false;
	}

	lastEmailTime = Sys_Milliseconds();

	idStr attachment;
	if ( errorLog ) {
		attachment.Format( ", \"attachment\":[\"file://%s\"]", errorLog );
	}

	idStr emailBody;
	emailBody.Format( SUPPORT_EMAIL_BODY, messageText );

	idStr format;
	format.Format( "data:json:{\"to\":\"%s\",\"subject\":\"%s\",\"body\":\"%s\"%s}\n",
			SUPPORT_EMAIL_ADDRESS, ( hasFatalError ? SUPPORT_EMAIL_SUBJECT_FATAL : SUPPORT_EMAIL_SUBJECT ), emailBody.c_str(), attachment.c_str() );

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

/*
====================
TestPermission
====================
*/
bool TestPermission( const char* permission ) {
	if ( idStr::Cmp( permission, "access_shared" ) == 0 ) {
		idStr str;
		str.Format( "%s/shared/misc/testofdoom.txt", Sys_CWD() );
		int file = open( str.c_str(), O_CREAT, S_IRWXU );
		bool success = file >= 0;
		if ( file >= 0 ) {
			close( file );
			remove( str.c_str() );
		}
		return success;
	}
	return false;
}

/*
====================
showRelaunchDialog_f
====================
*/
CONSOLE_COMMAND_SHIP( showRelaunchDialog, "Enable or disable game relaunch dialog", NULL ) {
	if ( args.Argc() < 2 || args.Argc() > 3 ) {
		common->Printf( "usage: showRelaunchDialog <show> [current value]\n" );
		return;
	}

	const char *arg = args.Argv( 1 );
	char show = arg && arg[0] == '1' ? 'F' : 'T';

	bool returnCurrentValue = args.Argc() > 2;

	int fd = open( "data/relaunch_dialog.txt", O_RDWR | O_CREAT, S_IRWXU );
	if ( fd >= 0 ) {
		qnx.dontShowRelaunchDialog = show == 'T';
		if ( returnCurrentValue ) {
			char tmp;
			if ( read( fd, &tmp, sizeof( char ) ) > 0 ) {
				common->Printf( "Show relaunch dialog: %s\n", ( tmp == 'F' ? "1" : "0" ) );
			} else {
				common->Warning( "Could not read relaunch dialog setting" );
			}
		}
		lseek( fd, 0, SEEK_SET );
		write( fd, &show, sizeof( char ) );
		close( fd );
	} else {
		common->Warning( "Could not get or change relaunch dialog setting" );
	}
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
	char* cmdLine = NULL;
	int fd = open( "data/relaunch.txt", O_RDONLY );
	if ( fd >= 0 ) {
		// Use the relaunch arguments
		lseek( fd, 0, SEEK_END );
		int dataLen = tell( fd );
		lseek( fd, 0, SEEK_SET );

		cmdLine = ( char* )Mem_Alloc( dataLen + 1, TAG_TEMP );
		int pos = read( fd, cmdLine, dataLen );
		if ( pos < 0 ) {
			Mem_Free( ( void* )cmdLine );
			cmdLine = NULL;
		} else {
			cmdLine[pos] = '\0';
		}
		close( fd );
		remove( "data/relaunch.txt" );
	}
	if ( cmdLine == NULL ) {
		// Use the system arguments
		idCmdArgs args;
		for ( int i = 0; i < argc; i++ ) {
			args.AppendArg( argv[i] );
		}
		idStr::Copynz( sys_cmdline, args.Args(), sizeof( sys_cmdline ) );
	}

	// get the initial time base
	Sys_Milliseconds();

//	Sys_FPU_EnableExceptions( TEST_FPU_EXCEPTIONS );
//	Sys_FPU_SetPrecision( FPU_PRECISION_DOUBLE_EXTENDED ); // ARM doesn't support this precision

	// test permissions
	qnx.permSharedFile = TestPermission( "access_shared" );

	if ( qnx.permSharedFile ) {
		// check for data, copy if it doesn't exist
		//TODO
	} else {
		// No shared access... this will error (need to setup cvar system for printf to work)

		//cmdSystem->Init();
		cvarSystem->Init();
		common->Printf( "No access to shared files, %s will probably error and crash.\n", GAME_NAME );
		cvarSystem->Shutdown();
		//cmdSystem->Shutdown();
	}

	// initialize system (done like this because cmdLine will just be parsed into argc and argv anyway, so if we have it, use it)
	if ( cmdLine ) {
		common->Init( 0, NULL, cmdLine );
		Mem_Free( ( void* )cmdLine );
	} else {
		if ( argc > 1 ) {
			common->Init( argc - 1, &argv[1], NULL );
		} else {
			common->Init( 0, NULL, NULL );
		}
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

	if ( doexit_spamguard ) {
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
Sys_StartProcess_Spawn
==================
*/
void Sys_StartProcess_Spawn( const char *path, bool doexit ) {
	idCmdArgs args( path, true );

	bool success = false;
	const char* target = args.Argv( 0 );

	if ( /*qnx.canSpawn && */ qnx.canNewApp ) {
		char ** argv = NULL;
		if ( args.Argc() > 1 ) {
			int argc;
			const char * const * argvTmp = args.GetArgs( &argc );

			// Copy over app path
			argv = ( char ** )Mem_Alloc( sizeof( char ** ) * ( argc + 1 ), TAG_TEMP );
			memcpy( argv, argvTmp, sizeof( char ** ) * argc );
			argv[argc + 1] = NULL;
		}

		posix_spawnattr_t spawnAtt;
		uint32_t spawnFlags;
		posix_spawnattr_init( &spawnAtt );
		posix_spawnattr_getxflags( &spawnAtt, &spawnFlags );
		spawnFlags |= POSIX_SPAWN_SEARCH_PATH | POSIX_SPAWN_NEWAPP;
		posix_spawnattr_getxflags( &spawnAtt, &spawnFlags );

		success = posix_spawnp( NULL, target, NULL, &spawnAtt, argv, NULL ) == EOK;

		posix_spawnattr_destroy( &spawnAtt );

		if ( argv ) {
			Mem_Free( ( void* )argv );
		}
	}

	if ( success ) {
		if ( doexit ) {
			cmdSystem->BufferCommandText( CMD_EXEC_APPEND, "quit\n" );
		}
	} else {
		common->Error( "Could not start process: '%s' ", path );
	}
}

/*
==================
idSysLocal::StartProcess
==================
*/
void idSysLocal::StartProcess( const char *exePath, bool doexit ) {

#if __SIZEOF_POINTER__ != __SIZEOF_INT__
#error Pointer is not the same size as a pointer, meaning the method of retrieving the invocation won't work
#endif

	// Try to invoke the process. If that doesn't work, then the BPS loop will catch it and try to spawn the process instead

	idCmdArgs args( exePath, true );
	if ( args.Argc() < 1 ) {
		// No args
		common->Warning( "StartProcess: exePath is empty\n" );
		if ( doexit ) {
			cmdSystem->BufferCommandText( CMD_EXEC_APPEND, "quit\n" );
		}
		return;
	}

	bool success = false;

	// Try invocation
	navigator_invoke_invocation_t *invoke = NULL;
	const char* target = args.Argv( 0 );

	navigator_invoke_invocation_create( &invoke );
	{
		idStr ptr;
		ptr.Format( "%p", invoke );
		navigator_invoke_invocation_set_id( invoke, ptr.c_str() );
	}
	navigator_invoke_invocation_set_action( invoke, "bb.action.OPEN" );
	success = navigator_invoke_invocation_set_target( invoke, target ) == BPS_SUCCESS;
	if ( success ) {
		idStr metadata;
		metadata.Format( "{\"type\":\"doom.sys_local.start_process\", \"doexit\":%s, \"path\":\"%s\"}", ( doexit ? "true" : "false" ), exePath );
		navigator_invoke_invocation_set_metadata( invoke, metadata.c_str() );

		if ( args.Argc() > 1 ) {
			const char *data = args.Args( 1 );
			navigator_invoke_invocation_set_data( invoke, data, strlen( data ) + 1 );
		}

		success = navigator_invoke_invocation_send( invoke ) == BPS_SUCCESS;
	}

	if ( !success ) {
		navigator_invoke_invocation_destroy( invoke );

		Sys_StartProcess_Spawn( exePath, doexit );
	}
}

/*
==================
Sys_SetFatalError
==================
*/
void Sys_SetFatalError( const char *error ) {
	slog2c( NULL, SLOG_CODE, SLOG2_CRITICAL, error );
	hasFatalError = true;
}

/*
================
Sys_UpdateLanguage
================
*/
extern idCVar sys_lang;
void Sys_UpdateLanguage( const char *language ) {
	sys_lang.ClearModified();

	int count = Sys_NumLangs();
	for ( int i = 0; i < count; i++ ) {
		if ( idStr::Icmp( language, Sys_LangCodes( i ) ) == 0 ) {
			sys_lang.SetString( Sys_Lang( i ) );
			break;
		}
	}

	if ( !sys_lang.IsModified() ) {
		sys_lang.SetString( Sys_DefaultLanguage() );
	}
}

/*
================
Sys_SetLanguageFromSystem
================
*/
void Sys_SetLanguageFromSystem() {
	char* language;
	char* country;
	if ( locale_get( &language, &country ) == BPS_SUCCESS ) {
		Sys_UpdateLanguage( language );

		bps_free( language );
		bps_free( country );
	} else {
		sys_lang.SetString( Sys_DefaultLanguage() );
	}
}
