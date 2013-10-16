/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.

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

#include "../../idlib/precompiled.h"

#include "qnx_local.h"

#include <sys/neutrino.h>
#include <sys/syspage.h>
#include <inttypes.h>
#include <fpstatus.h>

/*
==============================================================

	Clock ticks

==============================================================
*/

/*
================
Sys_GetClockTicks
================
*/
double Sys_GetClockTicks() {
	return (double)ClockCycles();
}

/*
================
Sys_ClockTicksPerSecond
================
*/
double Sys_ClockTicksPerSecond() {
	return (double)(SYSPAGE_ENTRY(qtime)->cycles_per_sec);
}


/*
==============================================================

	CPU

==============================================================
*/

#ifdef ID_QNX_X86
//TODO: switch to using QNX functions

#define _REG_EAX		0
#define _REG_EBX		1
#define _REG_ECX		2
#define _REG_EDX		3

/*
================
CPUID
================
*/
static void CPUID( int func, unsigned regs[4] ) {
	unsigned regEAX, regEBX, regECX, regEDX;

	__asm pusha
	__asm mov eax, func
	__asm __emit 00fh
	__asm __emit 0a2h
	__asm mov regEAX, eax
	__asm mov regEBX, ebx
	__asm mov regECX, ecx
	__asm mov regEDX, edx
	__asm popa

	regs[_REG_EAX] = regEAX;
	regs[_REG_EBX] = regEBX;
	regs[_REG_ECX] = regECX;
	regs[_REG_EDX] = regEDX;
}


/*
================
IsAMD
================
*/
static bool IsAMD() {
	char pstring[16];
	char processorString[13];

	// get name of processor
	CPUID( 0, ( unsigned int * ) pstring );
	processorString[0] = pstring[4];
	processorString[1] = pstring[5];
	processorString[2] = pstring[6];
	processorString[3] = pstring[7];
	processorString[4] = pstring[12];
	processorString[5] = pstring[13];
	processorString[6] = pstring[14];
	processorString[7] = pstring[15];
	processorString[8] = pstring[8];
	processorString[9] = pstring[9];
	processorString[10] = pstring[10];
	processorString[11] = pstring[11];
	processorString[12] = 0;

	if ( strcmp( processorString, "AuthenticAMD" ) == 0 ) {
		return true;
	}
	return false;
}

/*
================
Has3DNow
================
*/
static bool Has3DNow() {
	unsigned regs[4];

	// check AMD-specific functions
	CPUID( 0x80000000, regs );
	if ( regs[_REG_EAX] < 0x80000000 ) {
		return false;
	}

	// bit 31 of EDX denotes 3DNow! support
	CPUID( 0x80000001, regs );
	if ( regs[_REG_EDX] & ( 1 << 31 ) ) {
		return true;
	}

	return false;
}

/*
================
HasSSE3
================
*/
static bool HasSSE3() {
	unsigned regs[4];

	// get CPU feature bits
	CPUID( 1, regs );

	// bit 0 of ECX denotes SSE3 existence
	if ( regs[_REG_ECX] & ( 1 << 0 ) ) {
		return true;
	}
	return false;
}

/*
================
HasHTT
================
*/
static bool HasHTT() {
	unsigned regs[4];
	int logicalNum, physicalNum, HTStatusFlag;

	// get CPU feature bits
	CPUID( 1, regs );

	// bit 28 of EDX denotes HTT existence
	if ( !( regs[_REG_EDX] & ( 1 << 28 ) ) ) {
		return false;
	}

	HTStatusFlag = CPUCount( logicalNum, physicalNum );
	if ( HTStatusFlag != HT_ENABLED ) {
		return false;
	}
	return true;
}

/*
================
HasHTT
================
*/
static bool HasDAZ() {
	__declspec(align(16)) unsigned char FXSaveArea[512];
	unsigned char *FXArea = FXSaveArea;
	DWORD dwMask = 0;
	unsigned regs[4];

	// get CPU feature bits
	CPUID( 1, regs );

	// bit 24 of EDX denotes support for FXSAVE
	if ( !( regs[_REG_EDX] & ( 1 << 24 ) ) ) {
		return false;
	}

	memset( FXArea, 0, sizeof( FXSaveArea ) );

	__asm {
		mov		eax, FXArea
		FXSAVE	[eax]
	}

	dwMask = *(DWORD *)&FXArea[28];						// Read the MXCSR Mask
	return ( ( dwMask & ( 1 << 6 ) ) == ( 1 << 6 ) );	// Return if the DAZ bit is set
}
#endif

/*
================================================================================================

	CPU

================================================================================================
*/

/*
========================
Sys_GetCPUCacheSize
========================
*/
void Sys_GetCPUCacheSize( int level, int & count, int & size, int & lineSize ) {
	assert( level >= 1 && level <= 3 );

	count = 0;
	size = 0;
	lineSize = -1;

	//Iterate through the cpus
	unsigned int cpuCount = _syspage_ptr->num_cpu;
	for ( unsigned int i = 0; i < cpuCount; i++ ) {
		unsigned int cacheIndexI = SYSPAGE_ENTRY(cpuinfo)[i].ins_cache;
		unsigned int cacheIndexD = SYSPAGE_ENTRY(cpuinfo)[i].data_cache;

		//Get the correct cache level for the CPU
		int tLevel = level;
		while( tLevel != 1 ) {
			if( cacheIndexI != CACHE_LIST_END ) {
				cacheIndexI = SYSPAGE_ENTRY(cacheattr)[cacheIndexI].next;
			}
			if( cacheIndexD != CACHE_LIST_END ) {
				cacheIndexD = SYSPAGE_ENTRY(cacheattr)[cacheIndexD].next;
			}
			tLevel--;
		}

		//Get the smallest cache size info for the CPU
		int cCount = 0;
		unsigned int ls;
		unsigned int nl;
		if( cacheIndexI != CACHE_LIST_END ) {
			cCount = 1;
			ls = SYSPAGE_ENTRY(cacheattr)[cacheIndexI].line_size;
			nl = SYSPAGE_ENTRY(cacheattr)[cacheIndexI].num_lines;
		}
		if( cacheIndexI != cacheIndexD && cacheIndexD != CACHE_LIST_END ) {
			cCount++;
			unsigned int lsn = SYSPAGE_ENTRY(cacheattr)[cacheIndexD].line_size;
			unsigned int nln = SYSPAGE_ENTRY(cacheattr)[cacheIndexD].num_lines;
			if( ls > lsn ) {
				ls = lsn;
			}
			if( nl > nln ) {
				nl = nln;
			}
		}

		//Determine if the outer CPU info should be updated
		if( cCount > 0 ) {
			count += cCount;
			if( ((unsigned int)lineSize) > ls ) {
				lineSize = ls;
				size = ls * nl;
			}
		}
	}

	if( lineSize == -1) {
		lineSize = 0;
	}
}

/*
========================
Sys_CPUCount

numLogicalCPUCores	- the number of logical CPU per core
numPhysicalCPUCores	- the total number of cores per package
numCPUPackages		- the total number of packages (physical processors)
========================
*/
void Sys_CPUCount( int & numLogicalCPUCores, int & numPhysicalCPUCores, int & numCPUPackages ) {
	numPhysicalCPUCores = _syspage_ptr->num_cpu;
	numLogicalCPUCores = numPhysicalCPUCores; //XXX Don't know if this can be queried.
	numCPUPackages = 1; //XXX Don't know if this can be queried.
}

/*
================
Sys_GetCPUId
================
*/
cpuid_t Sys_GetCPUId() { //XXX Required
	int flags;

	unsigned int cpuFlags = SYSPAGE_ENTRY(cpuinfo)->flags;

#ifdef ID_QNX_ARM
	unsigned int cpuType = SYSPAGE_ENTRY(cpuinfo)->cpu;

	// check CPU manufacturer
	switch( ( cpuType & 0xFF000000 ) >> 24 ) {
		case 'A':	//ARM
		case 'D':	//Digital Equipment Corporation
		case 'M':	//Motorola/Freescale
		case 'V':	//Marvell
		case 'i':	//Intel
			flags = CPUID_GENERIC; //Nothing is really unsupported...
			break;
		case 'Q':	//Qualcomm
			flags = CPUID_QC;
			break;
		case 'T':	//Texas Instruments
			flags = CPUID_TI;
			break;
		}

	// check for VFP support
	if ( ( cpuFlags & CPU_FLAG_FPU ) != 0 ) {
		flags |= CPUID_VFP | CPUID_FTZ;
	}

	// check for NEON
	if ( ( cpuFlags & ARM_CPU_FLAG_NEON ) != 0 ) {
		flags |= CPUID_NEON;
		flags &= ~CPUID_FTZ; //NEON always does FTZ if NEON is used. If NEON is supported, we will use that anyway.
	}

	// check for iWMMX2
	if ( ( cpuFlags & ARM_CPU_FLAG_WMMX2 ) != 0 ) {
		flags |= CPUID_WMMX2;
	}
#else
	// verify we're at least a Pentium or 486 with CPUID support
	if ( ( cpuFlags & X86_CPU_CPUID ) == 0 ) {
		return CPUID_UNSUPPORTED;
	}

	// check for an AMD
	if ( IsAMD() ) {
		flags = CPUID_AMD;
	} else {
		flags = CPUID_INTEL;
	}

	// check for Multi Media Extensions
	if ( ( cpuFlags & X86_CPU_MMX ) != 0 ) {
		flags |= CPUID_MMX;
	}

	// check for 3DNow!
	if ( Has3DNow() ) {
		flags |= CPUID_3DNOW;
	}

	// check for Streaming SIMD Extensions
	if ( ( cpuFlags & X86_CPU_SIMD ) != 0 ) {
		flags |= CPUID_SSE | CPUID_FTZ;
	}

	// check for Streaming SIMD Extensions 2
	if ( ( cpuFlags & X86_CPU_SSE2 ) != 0 ) {
		flags |= CPUID_SSE2;
	}

	// check for Streaming SIMD Extensions 3 aka Prescott's New Instructions
	if ( HasSSE3() ) {
		flags |= CPUID_SSE3;
	}

	// check for Hyper-Threading Technology
	if ( HasHTT() ) {
		flags |= CPUID_HTT;
	}

	// check for Conditional Move (CMOV) and fast floating point comparison (FCOMI) instructions
	if ( ( cpuFlags & X86_CPU_CMOV ) != 0 ) {
		flags |= CPUID_CMOV;
	}

	// check for Denormals-Are-Zero mode
	if ( HasDAZ() ) {
		flags |= CPUID_DAZ;
	}
#endif

	return (cpuid_t)flags;
}


/*
===============================================================================

	FPU

===============================================================================
*/

typedef struct bitFlag_s {
	char *		name;
	int			bit;
} bitFlag_t;

static byte fpuState[128], *statePtr = fpuState;
static char fpuString[2048];
static bitFlag_t controlWordFlags[] = {
	{ "Invalid operation", 0 },
	{ "Denormalized operand", 1 },
	{ "Divide-by-zero", 2 },
	{ "Numeric overflow", 3 },
	{ "Numeric underflow", 4 },
	{ "Inexact result (precision)", 5 },
	{ "Infinity control", 12 },
	{ "", 0 }
};
static char *precisionControlField[] = {
	"Single Precision (24-bits)",
	"Reserved",
	"Double Precision (53-bits)",
	"Double Extended Precision (64-bits)"
};
static char *roundingControlField[] = {
	"Round to nearest",
	"Round down",
	"Round up",
	"Round toward zero"
};
static bitFlag_t statusWordFlags[] = {
	{ "Invalid operation", 0 },
	{ "Denormalized operand", 1 },
	{ "Divide-by-zero", 2 },
	{ "Numeric overflow", 3 },
	{ "Numeric underflow", 4 },
	{ "Inexact result (precision)", 5 },
	{ "Stack fault", 6 },
	{ "Error summary status", 7 },
	{ "FPU busy", 15 },
	{ "", 0 }
};

/*
===============
Sys_FPU_PrintStateFlags
===============
*/
int Sys_FPU_PrintStateFlags( char *ptr, int ctrl, int stat, int tags, int inof, int inse, int opof, int opse ) {
	int i, length = 0;

	length += sprintf( ptr+length,	"CTRL = %08x\n"
									"STAT = %08x\n"
									"TAGS = %08x\n"
									"INOF = %08x\n"
									"INSE = %08x\n"
									"OPOF = %08x\n"
									"OPSE = %08x\n"
									"\n",
									ctrl, stat, tags, inof, inse, opof, opse );

	length += sprintf( ptr+length, "Control Word:\n" );
	for ( i = 0; controlWordFlags[i].name[0]; i++ ) {
		length += sprintf( ptr+length, "  %-30s = %s\n", controlWordFlags[i].name, ( ctrl & ( 1 << controlWordFlags[i].bit ) ) ? "true" : "false" );
	}
	length += sprintf( ptr+length, "  %-30s = %s\n", "Precision control", precisionControlField[(ctrl>>8)&3] );
	length += sprintf( ptr+length, "  %-30s = %s\n", "Rounding control", roundingControlField[(ctrl>>10)&3] );

	length += sprintf( ptr+length, "Status Word:\n" );
	for ( i = 0; statusWordFlags[i].name[0]; i++ ) {
		ptr += sprintf( ptr+length, "  %-30s = %s\n", statusWordFlags[i].name, ( stat & ( 1 << statusWordFlags[i].bit ) ) ? "true" : "false" );
	}
	length += sprintf( ptr+length, "  %-30s = %d%d%d%d\n", "Condition code", (stat>>8)&1, (stat>>9)&1, (stat>>10)&1, (stat>>14)&1 );
	length += sprintf( ptr+length, "  %-30s = %d\n", "Top of stack pointer", (stat>>11)&7 );

	return length;
}

/*
===============
Sys_FPU_StackIsEmpty
===============
*/
bool Sys_FPU_StackIsEmpty() { //XXX Required
	__asm {
		mov			eax, statePtr
		fnstenv		[eax]
		mov			eax, [eax+8]
		xor			eax, 0xFFFFFFFF
		and			eax, 0x0000FFFF
		jz			empty
	}
	return false;
empty:
	return true;
}

/*
===============
Sys_FPU_ClearStack
===============
*/
void Sys_FPU_ClearStack() { //XXX Required
	__asm {
		mov			eax, statePtr
		fnstenv		[eax]
		mov			eax, [eax+8]
		xor			eax, 0xFFFFFFFF
		mov			edx, (3<<14)
	emptyStack:
		mov			ecx, eax
		and			ecx, edx
		jz			done
		fstp		st
		shr			edx, 2
		jmp			emptyStack
	done:
	}
}

/*
===============
Sys_FPU_GetState

  gets the FPU state without changing the state
===============
*/
const char *Sys_FPU_GetState() { //XXX Required
	double fpuStack[8] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
	double *fpuStackPtr = fpuStack;
	int i, numValues;
	char *ptr;

	__asm {
		mov			esi, statePtr
		mov			edi, fpuStackPtr
		fnstenv		[esi]
		mov			esi, [esi+8]
		xor			esi, 0xFFFFFFFF
		mov			edx, (3<<14)
		xor			eax, eax
		mov			ecx, esi
		and			ecx, edx
		jz			done
		fst			qword ptr [edi+0]
		inc			eax
		shr			edx, 2
		mov			ecx, esi
		and			ecx, edx
		jz			done
		fxch		st(1)
		fst			qword ptr [edi+8]
		inc			eax
		fxch		st(1)
		shr			edx, 2
		mov			ecx, esi
		and			ecx, edx
		jz			done
		fxch		st(2)
		fst			qword ptr [edi+16]
		inc			eax
		fxch		st(2)
		shr			edx, 2
		mov			ecx, esi
		and			ecx, edx
		jz			done
		fxch		st(3)
		fst			qword ptr [edi+24]
		inc			eax
		fxch		st(3)
		shr			edx, 2
		mov			ecx, esi
		and			ecx, edx
		jz			done
		fxch		st(4)
		fst			qword ptr [edi+32]
		inc			eax
		fxch		st(4)
		shr			edx, 2
		mov			ecx, esi
		and			ecx, edx
		jz			done
		fxch		st(5)
		fst			qword ptr [edi+40]
		inc			eax
		fxch		st(5)
		shr			edx, 2
		mov			ecx, esi
		and			ecx, edx
		jz			done
		fxch		st(6)
		fst			qword ptr [edi+48]
		inc			eax
		fxch		st(6)
		shr			edx, 2
		mov			ecx, esi
		and			ecx, edx
		jz			done
		fxch		st(7)
		fst			qword ptr [edi+56]
		inc			eax
		fxch		st(7)
	done:
		mov			numValues, eax
	}

	int ctrl = *(int *)&fpuState[0];
	int stat = *(int *)&fpuState[4];
	int tags = *(int *)&fpuState[8];
	int inof = *(int *)&fpuState[12];
	int inse = *(int *)&fpuState[16];
	int opof = *(int *)&fpuState[20];
	int opse = *(int *)&fpuState[24];

	ptr = fpuString;
	ptr += sprintf( ptr,"FPU State:\n"
						"num values on stack = %d\n", numValues );
	for ( i = 0; i < 8; i++ ) {
		ptr += sprintf( ptr, "ST%d = %1.10e\n", i, fpuStack[i] );
	}

	Sys_FPU_PrintStateFlags( ptr, ctrl, stat, tags, inof, inse, opof, opse );

	return fpuString;
}

/*
===============
Sys_FPU_EnableExceptions
===============
*/
void Sys_FPU_EnableExceptions( int exceptions ) {
	//Could also be done with fesetexceptflag
	int flags = 0;

	if ( exceptions & FPU_EXCEPTION_INVALID_OPERATION ) {
		flags |= _FP_EXC_INVALID;
	}
	if ( exceptions & FPU_EXCEPTION_DENORMALIZED_OPERAND ) {
		flags |= _FP_EXC_DENORMAL;
	}
	if ( exceptions & FPU_EXCEPTION_DIVIDE_BY_ZERO ) {
		flags |= _FP_EXC_DIVZERO;
	}
	if ( exceptions & FPU_EXCEPTION_NUMERIC_OVERFLOW ) {
		flags |= _FP_EXC_OVERFLOW;
	}
	if ( exceptions & FPU_EXCEPTION_NUMERIC_UNDERFLOW ) {
		flags |= _FP_EXC_UNDERFLOW;
	}
	if ( exceptions & FPU_EXCEPTION_INEXACT_RESULT ) {
		flags |= _FP_EXC_INEXACT;
	}

	//No way to simply set, we have to toggle flags on and off
	fp_exception_mask( _FP_EXC_ALL, 0 ); //Disable all exception mask flags
	fp_exception_mask( flags, 1 ); //Set exception flags
}

/*
===============
Sys_FPU_SetPrecision
===============
*/
void Sys_FPU_SetPrecision( int precision ) {
	int val = -1;

	switch( rounding ) {
	case FPU_PRECISION_SINGLE:
		val = _FP_PREC_FLOAT;
		break;
	case FPU_PRECISION_DOUBLE:
		val = _FP_PREC_DOUBLE;
		break;
	case FPU_PRECISION_DOUBLE_EXTENDED:
		val = _FP_PREC_DOUBLE_EXTENDED;
		break;
	}
	fp_precision( val );
}

/*
================
Sys_FPU_SetRounding
================
*/
void Sys_FPU_SetRounding( int rounding ) {
	//Could also be done with fesetround
	int val = -1;

	switch( rounding ) {
	case FPU_ROUNDING_TO_NEAREST:
		val = _FP_ROUND_NEAREST;
		break;
	case FPU_ROUNDING_DOWN:
		val = _FP_ROUND_NEGATIVE;
		break;
	case FPU_ROUNDING_UP:
		val = _FP_ROUND_POSITIVE;
		break;
	case FPU_ROUNDING_TO_ZERO:
		val = _FP_ROUND_ZERO;
		break;
	}
	fp_rounding( val );
}

/*
================
Sys_FPU_SetDAZ
================
*/
void Sys_FPU_SetDAZ( bool enable ) {
#ifdef ID_QNX_ARM
	//ARM does not have something equivalent to x86's DAZ, though supposedly FTZ handles both input (x86's DAZ) and output (x86's FTZ)
#elif defined(ID_QNX_X86)
	//XXX Required

	DWORD dwData;

	_asm {
		movzx	ecx, byte ptr enable
		and		ecx, 1
		shl		ecx, 6
		STMXCSR	dword ptr dwData
		mov		eax, dwData
		and		eax, ~(1<<6)	// clear DAX bit
		or		eax, ecx		// set the DAZ bit
		mov		dwData, eax
		LDMXCSR	dword ptr dwData
	}
#else
#error Unknown CPU architecture
#endif
}

/*
================
Sys_FPU_SetFTZ
================
*/
void Sys_FPU_SetFTZ( bool enable ) {
#ifdef ID_QNX_ARM
	__asm__("MOV	r1, %[enable]\n"
			"AND	r1, #0x1\n"
			"LSL	r1, #24\n"
			"FMRX	r0, FPSCR\n"
			"BIC	r0, #(1<<24)\n"	// clear FTZ bit ((1<<24) == ARM_VFP_FPSCR_FZ)
			"ORR	r0, r1\n"		// set the FTZ bit
			"FMXR	FPSCR, r0"
			:: [enable] "r" (enable) : "r0", "r1");
#elif defined(ID_QNX_X86)
	//XXX Required

	DWORD dwData;

	_asm {
		movzx	ecx, byte ptr enable
		and		ecx, 1
		shl		ecx, 15
		STMXCSR	dword ptr dwData
		mov		eax, dwData
		and		eax, ~(1<<15)	// clear FTZ bit
		or		eax, ecx		// set the FTZ bit
		mov		dwData, eax
		LDMXCSR	dword ptr dwData
	}
#else
#error Unknown CPU architecture
#endif
}
