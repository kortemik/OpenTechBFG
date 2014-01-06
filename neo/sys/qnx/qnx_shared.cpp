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

#include <sys/mman.h>
#include <sys/statvfs.h>
#include <sys/syspage.h>
#include <sys/resource.h>
#include <malloc.h>

/*
================
Sys_Milliseconds
================
*/
int Sys_Milliseconds() {
	static struct timespec sys_timeBase = { 0 };
	struct timespec ts;

	if ( sys_timeBase.tv_sec == 0 ) {
		clock_gettime( CLOCK_MONOTONIC, &sys_timeBase );
	}
	clock_gettime( CLOCK_MONOTONIC, &ts );
	return ( (1000.0 * ts.tv_sec) + (0.000001 * ts.tv_nsec) ) - ( (1000.0 * sys_timeBase.tv_sec) + (0.000001 * sys_timeBase.tv_nsec) );
}

/*
========================
Sys_Microseconds
========================
*/
uint64 Sys_Microseconds() {
	static uint64 ticksPerMicrosecondTimes1024 = 0;

	if ( ticksPerMicrosecondTimes1024 == 0 ) {
		ticksPerMicrosecondTimes1024 = ( (uint64)Sys_ClockTicksPerSecond() << 10 ) / 1000000;
		assert( ticksPerMicrosecondTimes1024 > 0 );
	}

	return ((uint64)( (int64)Sys_GetClockTicks() << 10 )) / ticksPerMicrosecondTimes1024;
}

/*
================
Sys_GetSystemRamInBytes
================
*/
int64 Sys_GetSystemRamInBytes() {
#if 1
	//Based off http://www.qssl.com/developers/docs/6.3.2/momentics/release_notes/rel_6.3.2.html (section "System page")
	char *str = SYSPAGE_ENTRY(strings)->data;
	struct asinfo_entry *as = SYSPAGE_ENTRY(asinfo);
	uint64_t total = 0;
	unsigned num;

	for(num = _syspage_ptr->asinfo.entry_size / sizeof(*as); num > 0; --num)
	{
		if(strcmp(&str[as->name], "ram") == 0)
		{
			total += as->end - as->start + 1;
		}
		++as;
	}
	return total;
#else
	//Usable, but more reading and writing then desired... requires libpps to be added
	int fd = open( "/pps/services/hw_info/inventory", O_RDONLY );
	lseek( fd, 0, SEEK_END );
	int size = tell( fd ) + 1;
	lseek( fd, 0, SEEK_SET );

	char* data = ( char* )calloc( size, sizeof( char ) );
	read( fd, data, size - 1 );
	close( fd );

	int ramSize = 0;
	pps_decoder_t dec;
	if ( pps_decoder_initialize( &dec, data ) == PPS_DECODER_OK ) {

		if( pps_decoder_push( &dec, "@inventory" ) == PPS_DECODER_OK ) {
			const char* ram;
			if( pps_decoder_get_string( &dec, "RAM_Size", &ram ) == PPS_DECODER_OK ) {
				ramSize = atoi( ram );
			}
		}

		pps_decoder_cleanup( &dec );
	}
	free( data );

	return ramSize;
#endif
}

/*
================
Sys_GetSystemRam

	returns amount of physical memory in MB
================
*/
int Sys_GetSystemRam() {
	return Sys_GetSystemRamInBytes() / ( 1024 * 1024 );
}

/*
================
Sys_GetDriveFreeSpace
returns in megabytes
================
*/
int Sys_GetDriveFreeSpace( const char *path ) {
	struct statvfs64 stv;
	int ret = 26;
	if ( statvfs64( path, &stv ) == 0 ) {
		ret = ( double )( stv.f_bavail * stv.f_frsize ) / ( 1024.0 * 1024.0 );
	}
	return ret;
}

/*
========================
Sys_GetDriveFreeSpaceInBytes
========================
*/
int64 Sys_GetDriveFreeSpaceInBytes( const char * path ) {
	struct statvfs64 stv;
	int64 ret = 1;
	if ( statvfs64( path, &stv ) == 0 ) {
		ret = stv.f_bavail * stv.f_frsize;
	}
	return ret;
}

/*
================
Sys_GetVideoRam
returns in megabytes
================
*/
int Sys_GetVideoRam() {
	return Sys_GetSystemRam(); // Video RAM is shared with system RAM
	//TODO: not sure I can get this. Worse-case: hardcode as it doesn't get used for anything other then console print-out
	/*
	unsigned int retSize = 64;

	CComPtr<IWbemLocator> spLoc = NULL;
	HRESULT hr = CoCreateInstance( CLSID_WbemLocator, 0, CLSCTX_SERVER, IID_IWbemLocator, ( LPVOID * ) &spLoc );
	if ( hr != S_OK || spLoc == NULL ) {
		return retSize;
	}

	CComBSTR bstrNamespace( _T( "\\\\.\\root\\CIMV2" ) );
	CComPtr<IWbemServices> spServices;

	// Connect to CIM
	hr = spLoc->ConnectServer( bstrNamespace, NULL, NULL, 0, NULL, 0, 0, &spServices );
	if ( hr != WBEM_S_NO_ERROR ) {
		return retSize;
	}

	// Switch the security level to IMPERSONATE so that provider will grant access to system-level objects.
	hr = CoSetProxyBlanket( spServices, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE );
	if ( hr != S_OK ) {
		return retSize;
	}

	// Get the vid controller
	CComPtr<IEnumWbemClassObject> spEnumInst = NULL;
	hr = spServices->CreateInstanceEnum( CComBSTR( "Win32_VideoController" ), WBEM_FLAG_SHALLOW, NULL, &spEnumInst );
	if ( hr != WBEM_S_NO_ERROR || spEnumInst == NULL ) {
		return retSize;
	}

	ULONG uNumOfInstances = 0;
	CComPtr<IWbemClassObject> spInstance = NULL;
	hr = spEnumInst->Next( 10000, 1, &spInstance, &uNumOfInstances );

	if ( hr == S_OK && spInstance ) {
		// Get properties from the object
		CComVariant varSize;
		hr = spInstance->Get( CComBSTR( _T( "AdapterRAM" ) ), 0, &varSize, 0, 0 );
		if ( hr == S_OK ) {
			retSize = varSize.intVal / ( 1024 * 1024 );
			if ( retSize == 0 ) {
				retSize = 64;
			}
		}
	}
	return retSize;
	*/
}

/*
================
Sys_GetCurrentMemoryStatus

	returns OS mem info
	all values are in kB except the memoryload
================
*/
void Sys_GetCurrentMemoryStatus( sysMemoryStats_t &stats ) {
#if __SIZEOF_POINTER__ != __SIZEOF_INT__
#error Pointer is not the same size as a pointer, meaning mallopt won't work
#endif

	struct malloc_stats _malloc_stats;
	mallopt( MALLOC_STATS, ( int )( &_malloc_stats ) );

	uint64_t totalPhysical = Sys_GetSystemRamInBytes();

	//XXX Is this correct and/or "complete"?
	uint64_t total = _malloc_stats.m_allocmem + _malloc_stats.m_small_allocmem; //Get to the amount of memory allocated

	stats.memoryLoad = ( int )( ( ( float )total / totalPhysical ) * 100 );

	// Adjust memory amount and availability in kb
	stats.totalPhysical = totalPhysical >> 10;
	stats.availPhysical = ( totalPhysical - total ) >> 10;

	// There is no such thing as a page file on QNX
	stats.availPageFile = 0;
	stats.totalPageFile = 0;

	// Virtual memory
	struct rlimit64 limit;
	if ( getrlimit64( RLIMIT_VMEM, &limit ) == 0 ) {
		// Limits can be very big, reduce it to fit int
		stats.totalVirtual = __min( limit.rlim_cur >> 10, INT_MAX );
		stats.availVirtual = __min( ( limit.rlim_cur - total ) >> 10, INT_MAX );
	} else {
		stats.totalVirtual = stats.totalPhysical;
		stats.availVirtual = stats.availPhysical;
	}
	stats.availExtendedVirtual = 0;
}

/*
================
Sys_LockMemory
================
*/
bool Sys_LockMemory( void *ptr, int bytes ) {
	if ( qnx.canLockMem ) {
		return ( mlock( ptr, (size_t)bytes ) == 0 );
	}
	return false;
}

/*
================
Sys_UnlockMemory
================
*/
bool Sys_UnlockMemory( void *ptr, int bytes ) {
	return ( munlock( ptr, (size_t)bytes ) == 0 );
}

/*
================
Sys_SetPhysicalWorkMemory
================
*/
void Sys_SetPhysicalWorkMemory( int minBytes, int maxBytes ) {
	if ( minBytes == -1 && maxBytes == -1 ) {
		//XXX Removes as many unused memory pages as possible
		return;
	}
	// Cannot set min process size, but can set max
	struct rlimit64 limit;
	if ( getrlimit64( RLIMIT_DATA, &limit ) == 0 ) {
		limit.rlim_cur = __min( maxBytes, limit.rlim_max );
		setrlimit64( RLIMIT_DATA, &limit );
	}
}


/*
===============================================================================

	Call stack

===============================================================================
*/


#define PROLOGUE_SIGNATURE 0x00EC8B55

/*
#include <dbghelp.h>

const int UNDECORATE_FLAGS =	UNDNAME_NO_MS_KEYWORDS |
								UNDNAME_NO_ACCESS_SPECIFIERS |
								UNDNAME_NO_FUNCTION_RETURNS |
								UNDNAME_NO_ALLOCATION_MODEL |
								UNDNAME_NO_ALLOCATION_LANGUAGE |
								UNDNAME_NO_MEMBER_TYPE;
*/

#if defined(_DEBUG) && 1

typedef struct symbol_s {
	int					address;
	char *				name;
	struct symbol_s *	next;
} symbol_t;

typedef struct module_s {
	int					address;
	char *				name;
	symbol_t *			symbols;
	struct module_s *	next;
} module_t;

module_t *modules;

/*
==================
SkipRestOfLine
==================
*/
void SkipRestOfLine( const char **ptr ) {
	while( (**ptr) != '\0' && (**ptr) != '\n' && (**ptr) != '\r' ) {
		(*ptr)++;
	}
	while( (**ptr) == '\n' || (**ptr) == '\r' ) {
		(*ptr)++;
	}
}

/*
==================
SkipWhiteSpace
==================
*/
void SkipWhiteSpace( const char **ptr ) {
	while( (**ptr) == ' ' ) {
		(*ptr)++;
	}
}

/*
==================
ParseHexNumber
==================
*/
int ParseHexNumber( const char **ptr ) {
	int n = 0;
	while( (**ptr) >= '0' && (**ptr) <= '9' || (**ptr) >= 'a' && (**ptr) <= 'f' ) {
		n <<= 4;
		if ( **ptr >= '0' && **ptr <= '9' ) {
			n |= ( (**ptr) - '0' );
		} else {
			n |= 10 + ( (**ptr) - 'a' );
		}
		(*ptr)++;
	}
	return n;
}

/*
==================
Sym_Init
==================
*/
void Sym_Init( long addr ) {
	//TODO
	/*
	TCHAR moduleName[MAX_STRING_CHARS];
	MEMORY_BASIC_INFORMATION mbi;

	VirtualQuery( (void*)addr, &mbi, sizeof(mbi) );

	GetModuleFileName( (HMODULE)mbi.AllocationBase, moduleName, sizeof( moduleName ) );

	char *ext = moduleName + strlen( moduleName );
	while( ext > moduleName && *ext != '.' ) {
		ext--;
	}
	if ( ext == moduleName ) {
		strcat( moduleName, ".map" );
	} else {
		strcpy( ext, ".map" );
	}

	module_t *module = (module_t *) malloc( sizeof( module_t ) );
	module->name = (char *) malloc( strlen( moduleName ) + 1 );
	strcpy( module->name, moduleName );
	module->address = (int)mbi.AllocationBase;
	module->symbols = NULL;
	module->next = modules;
	modules = module;

	FILE * fp = fopen( moduleName, "rb" );
	if ( fp == NULL ) {
		return;
	}

	int pos = ftell( fp );
	fseek( fp, 0, SEEK_END );
	int length = ftell( fp );
	fseek( fp, pos, SEEK_SET );

	char *text = (char *) malloc( length+1 );
	fread( text, 1, length, fp );
	text[length] = '\0';
	fclose( fp );

	const char *ptr = text;

	// skip up to " Address" on a new line
	while( *ptr != '\0' ) {
		SkipWhiteSpace( &ptr );
		if ( idStr::Cmpn( ptr, "Address", 7 ) == 0 ) {
			SkipRestOfLine( &ptr );
			break;
		}
		SkipRestOfLine( &ptr );
	}

	int symbolAddress;
	int symbolLength;
	char symbolName[MAX_STRING_CHARS];
	symbol_t *symbol;

	// parse symbols
	while( *ptr != '\0' ) {

		SkipWhiteSpace( &ptr );

		ParseHexNumber( &ptr );
		if ( *ptr == ':' ) {
			ptr++;
		} else {
			break;
		}
		ParseHexNumber( &ptr );

		SkipWhiteSpace( &ptr );

		// parse symbol name
		symbolLength = 0;
		while( *ptr != '\0' && *ptr != ' ' ) {
			symbolName[symbolLength++] = *ptr++;
			if ( symbolLength >= sizeof( symbolName ) - 1 ) {
				break;
			}
		}
		symbolName[symbolLength++] = '\0';

		SkipWhiteSpace( &ptr );

		// parse symbol address
		symbolAddress = ParseHexNumber( &ptr );

		SkipRestOfLine( &ptr );

		symbol = (symbol_t *) malloc( sizeof( symbol_t ) );
		symbol->name = (char *) malloc( symbolLength );
		strcpy( symbol->name, symbolName );
		symbol->address = symbolAddress;
		symbol->next = module->symbols;
		module->symbols = symbol;
	}

	free( text );
	*/
}

/*
==================
Sym_Shutdown
==================
*/
void Sym_Shutdown() {
	module_t *m;
	symbol_t *s;

	for ( m = modules; m != NULL; m = modules ) {
		modules = m->next;
		for ( s = m->symbols; s != NULL; s = m->symbols ) {
			m->symbols = s->next;
			free( s->name );
			free( s );
		}
		free( m->name );
		free( m );
	}
	modules = NULL;
}

/*
==================
Sym_GetFuncInfo
==================
*/
void Sym_GetFuncInfo( long addr, idStr &module, idStr &funcName ) {
	//TODO
	/*
	MEMORY_BASIC_INFORMATION mbi;
	module_t *m;
	symbol_t *s;

	VirtualQuery( (void*)addr, &mbi, sizeof(mbi) );

	for ( m = modules; m != NULL; m = m->next ) {
		if ( m->address == (int) mbi.AllocationBase ) {
			break;
		}
	}
	if ( !m ) {
		Sym_Init( addr );
		m = modules;
	}

	for ( s = m->symbols; s != NULL; s = s->next ) {
		if ( s->address == addr ) {

			char undName[MAX_STRING_CHARS];
			if ( UnDecorateSymbolName( s->name, undName, sizeof(undName), UNDECORATE_FLAGS ) ) {
				funcName = undName;
			} else {
				funcName = s->name;
			}
			for ( int i = 0; i < funcName.Length(); i++ ) {
				if ( funcName[i] == '(' ) {
					funcName.CapLength( i );
					break;
				}
			}
			module = m->name;
			return;
		}
	}

	sprintf( funcName, "0x%08x", addr );
	module = "";
	*/
}

#elif defined(_DEBUG)

/*
DWORD lastAllocationBase = -1;
HANDLE processHandle;
idStr lastModule;
*/

/*
==================
Sym_Init
==================
*/
void Sym_Init( long addr ) {
	//TODO
	/*
	TCHAR moduleName[MAX_STRING_CHARS];
	TCHAR modShortNameBuf[MAX_STRING_CHARS];
	MEMORY_BASIC_INFORMATION mbi;

	if ( lastAllocationBase != -1 ) {
		Sym_Shutdown();
	}

	VirtualQuery( (void*)addr, &mbi, sizeof(mbi) );

	GetModuleFileName( (HMODULE)mbi.AllocationBase, moduleName, sizeof( moduleName ) );
	_splitpath( moduleName, NULL, NULL, modShortNameBuf, NULL );
	lastModule = modShortNameBuf;

	processHandle = GetCurrentProcess();
	if ( !SymInitialize( processHandle, NULL, FALSE ) ) {
		return;
	}
	if ( !SymLoadModule( processHandle, NULL, moduleName, NULL, (DWORD)mbi.AllocationBase, 0 ) ) {
		SymCleanup( processHandle );
		return;
	}

	SymSetOptions( SymGetOptions() & ~SYMOPT_UNDNAME );

	lastAllocationBase = (DWORD) mbi.AllocationBase;
	*/
}

/*
==================
Sym_Shutdown
==================
*/
void Sym_Shutdown() {
	//TODO
	/*
	SymUnloadModule( GetCurrentProcess(), lastAllocationBase );
	SymCleanup( GetCurrentProcess() );
	lastAllocationBase = -1;
	*/
}

/*
==================
Sym_GetFuncInfo
==================
*/
void Sym_GetFuncInfo( long addr, idStr &module, idStr &funcName ) {
	//TODO
	/*
	MEMORY_BASIC_INFORMATION mbi;

	VirtualQuery( (void*)addr, &mbi, sizeof(mbi) );

	if ( (DWORD) mbi.AllocationBase != lastAllocationBase ) {
		Sym_Init( addr );
	}

	BYTE symbolBuffer[ sizeof(IMAGEHLP_SYMBOL) + MAX_STRING_CHARS ];
	PIMAGEHLP_SYMBOL pSymbol = (PIMAGEHLP_SYMBOL)&symbolBuffer[0];
	pSymbol->SizeOfStruct = sizeof(symbolBuffer);
	pSymbol->MaxNameLength = 1023;
	pSymbol->Address = 0;
	pSymbol->Flags = 0;
	pSymbol->Size =0;

	DWORD symDisplacement = 0;
	if ( SymGetSymFromAddr( processHandle, addr, &symDisplacement, pSymbol ) ) {
		// clean up name, throwing away decorations that don't affect uniqueness
	    char undName[MAX_STRING_CHARS];
		if ( UnDecorateSymbolName( pSymbol->Name, undName, sizeof(undName), UNDECORATE_FLAGS ) ) {
			funcName = undName;
		} else {
			funcName = pSymbol->Name;
		}
		module = lastModule;
	}
	else {
		LPVOID lpMsgBuf;
		FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
						NULL,
						GetLastError(),
						MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
						(LPTSTR) &lpMsgBuf,
						0,
						NULL
						);
		LocalFree( lpMsgBuf );

		// Couldn't retrieve symbol (no debug info?, can't load dbghelp.dll?)
		sprintf( funcName, "0x%08x", addr );
		module = "";
    }
    */
}

#else

/*
==================
Sym_Init
==================
*/
void Sym_Init( long addr ) {
}

/*
==================
Sym_Shutdown
==================
*/
void Sym_Shutdown() {
}

/*
==================
Sym_GetFuncInfo
==================
*/
void Sym_GetFuncInfo( long addr, idStr &module, idStr &funcName ) {
	module = "";
	sprintf( funcName, "0x%08x", addr );
}

#endif

/*
==================
GetFuncAddr
==================
*/
address_t GetFuncAddr( address_t midPtPtr ) {
	//TODO
	return 0;
	/*
	long temp;
	do {
		temp = (long)(*(long*)midPtPtr);
		if ( (temp&0x00FFFFFF) == PROLOGUE_SIGNATURE ) {
			break;
		}
		midPtPtr--;
	} while(true);

	return midPtPtr;
	*/
}

/*
==================
GetCallerAddr
==================
*/
address_t GetCallerAddr( long _ebp ) {
	//TODO
	return 0;
	/*
	long midPtPtr;
	long res = 0;

	__asm {
		mov		eax, _ebp
		mov		ecx, [eax]		// check for end of stack frames list
		test	ecx, ecx		// check for zero stack frame
		jz		label
		mov		eax, [eax+4]	// get the ret address
		test	eax, eax		// check for zero return address
		jz		label
		mov		midPtPtr, eax
	}
	res = GetFuncAddr( midPtPtr );
label:
	return res;
	*/
}

/*
==================
Sys_GetCallStack
==================
*/
void Sys_GetCallStack( address_t *callStack, const int callStackSize ) {
	//TODO
	/*
	 * "use /Oy option" <-- compiler flag for something...
	 *
#if 1 //def _DEBUG
	int i;
	long m_ebp;

	__asm {
		mov eax, ebp
		mov m_ebp, eax
	}
	// skip last two functions
	m_ebp = *((long*)m_ebp);
	m_ebp = *((long*)m_ebp);
	// list functions
	for ( i = 0; i < callStackSize; i++ ) {
		callStack[i] = GetCallerAddr( m_ebp );
		if ( callStack[i] == 0 ) {
			break;
		}
		m_ebp = *((long*)m_ebp);
	}
#else
	int i = 0;
#endif
	while( i < callStackSize ) {
		callStack[i++] = 0;
	}
	*/
}

/*
==================
Sys_GetCallStackStr
==================
*/
const char *Sys_GetCallStackStr( const address_t *callStack, const int callStackSize ) {
	static char string[MAX_STRING_CHARS*2];
	int index, i;
	idStr module, funcName;

	index = 0;
	for ( i = callStackSize-1; i >= 0; i-- ) {
		Sym_GetFuncInfo( callStack[i], module, funcName );
		index += sprintf( string+index, " -> %s", funcName.c_str() );
	}
	return string;
}

/*
==================
Sys_GetCallStackCurStr
==================
*/
const char *Sys_GetCallStackCurStr( int depth ) {
	address_t *callStack;

	callStack = (address_t *) _alloca( depth * sizeof( address_t ) );
	Sys_GetCallStack( callStack, depth );
	return Sys_GetCallStackStr( callStack, depth );
}

/*
==================
Sys_GetCallStackCurAddressStr
==================
*/
const char *Sys_GetCallStackCurAddressStr( int depth ) {
	static char string[MAX_STRING_CHARS*2];
	address_t *callStack;
	int index, i;

	callStack = (address_t *) _alloca( depth * sizeof( address_t ) );
	Sys_GetCallStack( callStack, depth );

	index = 0;
	for ( i = depth-1; i >= 0; i-- ) {
		index += sprintf( string+index, " -> 0x%08x", callStack[i] );
	}
	return string;
}

/*
==================
Sys_ShutdownSymbols
==================
*/
void Sys_ShutdownSymbols() {
	Sym_Shutdown();
}

/*
==================
Json_IsWhitespace
==================
*/
bool Json_IsWhitespace( char c ) {
	switch ( c ) {
	case ' ':
	case '\n':
	case '\t':
	case '\r':
	case '\v':
		return true;
	default:
		return false;
	}
}

/*
==================
Json_FindValueLen
==================
*/
int Json_FindKeyValueLen( const char *json, int start );
int Json_FindValueLen( const char *json, int start ) {
	const char* c = json + start;

	while ( *c ) {
		if ( *c == 'n' ) {
			// null
			if ( *( c + 1 ) == 'u' && *( c + 2 ) == 'l' && *( c + 3 ) == 'l' ) {
				return 4;
			} else {
				return -1;
			}
		} else if ( *c == 't' ) {
			// true
			if ( *( c + 1 ) == 'r' && *( c + 2 ) == 'u' && *( c + 3 ) == 'e' ) {
				return 4;
			} else {
				return -1;
			}
		} else if ( *c == 'f' ) {
			// false
			if ( *( c + 1 ) == 'a' && *( c + 2 ) == 'l' && *( c + 3 ) == 's' && *( c + 4 ) == 'e' ) {
				return 5;
			} else {
				return -1;
			}
		} else if ( idStr::CharIsNumeric( *c ) || ( ( *c == '-' || *c == '+' ) && idStr::CharIsNumeric( *( c + 1 ) ) ) ) {
			// number
			if ( *c == '-' ) {
				c++;
			}
			bool dot = false;
			while ( *c ) {
				if ( !idStr::CharIsNumeric( *c ) ) {
					if ( *c == ',' || *c == ']' || *c == '}' || Json_IsWhitespace( *c ) ) {
						return c - ( json + start );
					}
					if ( *c == '.' && !dot ) {
						dot = true;
						c++;
						continue;
					}
					return -1;
				}
				c++;
			}
		} else if ( *c == '"' ) {
			// string
			c++;

			// Nieve parsing... Valid: {"a":"b"} -> {"a":"{\"a\":\"b\"}"} -> {"a":"{\"a\":\"{\"a\":\"b\"}\"}"}... but so is {"a":"{{{{{"} so it would requires some manual parsing
			while ( *c && *c != '"' ) {
				c++;
			}
			if ( *c == '\0' ) {
				return -1;
			} else {
				return c - ( json + start + 1 );
			}
		} else if ( *c == '{' ) {
			// object
			c++;
			while ( *c ) {
				if ( *c == '}' ) {
					break;
				}
				if ( !Json_IsWhitespace( *c ) && ( *c ) != ',' ) {
					int len = Json_FindKeyValueLen( c, 0 );
					if ( len == -1 ) {
						return -1;
					}
					c += len;
				}
				if ( *c == '}' ) {
					break;
				}
				c++;
			}
			if ( *c == '\0' ) {
				return -1;
			} else {
				return c - ( json + start ) + 1;
			}
		} else if ( *c == '[' ) {
			// array
			c++;
			while ( *c ) {
				if ( *c == ']' ) {
					break;
				}
				if ( !Json_IsWhitespace( *c ) && ( *c ) != ',' ) {
					int len = Json_FindValueLen( c, 0 );
					if ( len == -1 ) {
						return -1;
					}
					c += len + ( *c == '"' ? 1 : 0 );
				}
				if ( *c == ']' ) {
					break;
				}
				c++;
			}
			if ( *c == '\0' ) {
				return -1;
			} else {
				return c - ( json + start ) + 1;
			}
		}
		c++;
	}

	return -1;
}

/*
==================
Json_FindKeyValueLen
==================
*/
int Json_FindKeyValueLen( const char *json, int start ) {
	const char* c = json + start;
	int state = 0;

	while ( *c ) {
		if ( *c == '}' ) {
			// End of object
			return -1;
		}

		if ( state == 0 ) {
			// Key
			if ( *c == '"' ) {
				int len = Json_FindValueLen( c, 0 );
				if ( len == -1 ||
						*( c + len + 1 ) != '"' ) {
					return -1;
				}
				c += len + 1;
				state = 1;
			}
		} else if ( state == 1 ) {
			// Looking for :
			if ( *c == ':' ) {
				state = 2;
			}
		} else if ( state == 2 ) {
			// Value
			if ( !Json_IsWhitespace( *c ) ) {
				int len = Json_FindValueLen( c, 0 );
				if ( len == -1 ) {
					return -1;
				}
				c += len + ( *c == '"' ? 1 : 0 );
				return c - ( json + start );
			}
		}
		c++;
	}

	return -1;
}

/*
==================
Json_FindKey
==================
*/
int Json_FindKey( const char *json, const char *key ) {
	const char* c = json;
	int state = 0;
	bool found = false;

	while ( *c ) {
		if ( *c == '}' ) {
			// End of object
			return -1;
		}

		// Key/value search
		if ( state == 0 ) {
			// Looking for key
			if ( *c == '"' ) {
				int len = Json_FindValueLen( c, 0 );
				if ( len == -1 ||
						*( c + len + 1 ) != '"' ) {
					return -1;
				}
				found = idStr::Cmpn( key, c + 1, len ) == 0;
				c += len + 1;
				state = 1;
			}
		} else if ( state == 1 ) {
			// Looking for :
			if ( *c == ':' ) {
				state = 2;
			}
		} else if ( state == 2 ) {
			// Skipping value
			if ( !Json_IsWhitespace( *c ) ) {
				int len = Json_FindValueLen( c, 0 );
				if ( len == -1 ) {
					return -1;
				}
				if ( found ) {
					return c - json + 1;
				}
				c += len + ( *c == '"' ? 1 : 0 );
				if ( *c != ',' ) {
					state = 3;
				} else {
					state = 0;
				}
			}
		} else if ( state == 3 ) {
			// Looking for ,
			if ( *c == ',' ) {
				state = 0;
			}
		}
		c++;
	}

	return -1;
}

/*
==================
Json_EscapeString
==================
*/
void Json_EscapeString( char *str ) {
	int s = 0;
	int d = 0;
	while ( str[s] ) {
		char c = str[s++];
		if ( str[s - 1] == '\\' ) {
			switch ( str[s++] ) {
#define ESCAPE_CHAR( sc, rc ) case (sc): c = (rc); break
			ESCAPE_CHAR( '\'', '\'' );
			ESCAPE_CHAR( '"', '"' );
			ESCAPE_CHAR( '?', '\?' );
			ESCAPE_CHAR( '\\', '\\' );
			ESCAPE_CHAR( '0', '\0' ); // Problem?
			ESCAPE_CHAR( 'a', '\a' );
			ESCAPE_CHAR( 'b', '\b' );
			ESCAPE_CHAR( 'f', '\f' );
			ESCAPE_CHAR( 'n', '\n' );
			ESCAPE_CHAR( 'r', '\r' );
			ESCAPE_CHAR( 't', '\t' );
			ESCAPE_CHAR( 'v', '\v' );
			// Don't worry about octal and hex?
#undef ESCAPE_CHAR
			}
		}
		str[d++] = c;
	}
	str[d] = '\0';
}

// Originally wanted to use PPS decoder, but along with having the same issue this JSON parser has (strings can't contain sub-strings), it doesn't return
// the full value desired. So {"test":{"test2":10}} with a request for "test" would result in a NULL (error) as opposed to {"test2":10} as this parser does.

/*
==================
Sys_ParseJSONObj
==================
*/
const char *Sys_ParseJSONObj( const char* json, const char* key, bool allocateMemory ) {
	static char text[2048];

	if ( !json || json[0] != '{' ) {
		return NULL;
	}

	int valueStart = Json_FindKey( json + 1, key );
	if ( valueStart == -1 ) {
		return NULL;
	}

	int len = Json_FindValueLen( json, valueStart );
	if ( len == -1 ) {
		return NULL;
	}
	bool isString = false;
	if ( json[valueStart] == '"' ) {
		valueStart++;
		isString = true;
	}

	char *dst = text;
	if ( allocateMemory ) {
		dst = ( char* )Mem_Alloc( len + 1, TAG_STRING );
		dst[0] = '\0';
	} else if ( ( unsigned int )( len + 1 ) > sizeof( text ) ) {
		return "";
	}

	// Copy data
	memcpy( dst, json + valueStart, len );
	dst[len] = '\0';
	if ( isString ) {
		Json_EscapeString( dst );
	}
	return dst;
}

/*
==================
Sys_ParseJSONArr
==================
*/
const char *Sys_ParseJSONArr( const char* json, unsigned int index, bool allocateMemory ) {
	static char text[2048];

	if ( !json || json[0] != '[' ) {
		return NULL;
	}

	int valueStart = 1;
	int len = -1;

	while ( json[valueStart] ) {
		if ( json[valueStart] == ']' ) {
			return NULL;
		}
		if ( !Json_IsWhitespace( json[valueStart] ) && ( json[valueStart] ) != ',' ) {
			len = Json_FindValueLen( json, valueStart );
			if ( len == -1 ) {
				return NULL;
			}
			if ( index == 0 ) {
				break;
			}
			index--;
			valueStart += len + ( json[valueStart] == '"' ? 1 : 0 );
		}
		if ( json[valueStart] == ']' ) {
			return NULL;
		}
		valueStart++;
	}

	bool isString = false;
	if ( json[valueStart] == '"' ) {
		valueStart++;
		isString = true;
	}

	char *dst = text;
	if ( allocateMemory ) {
		dst = ( char* )Mem_Alloc( len + 1, TAG_STRING );
		dst[0] = '\0';
	} else if ( ( unsigned int )( len + 1 ) > sizeof( text ) ) {
		return "";
	}

	// Copy data
	memcpy( dst, json + valueStart, len );
	dst[len] = '\0';
	if ( isString ) {
		Json_EscapeString( dst );
	}
	return dst;
}
