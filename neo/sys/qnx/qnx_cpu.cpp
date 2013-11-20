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

#include <sys/neutrino.h>
#include <sys/syspage.h>
#include <inttypes.h>
#include <fpstatus.h>

#ifdef ID_QNX_X86
#include <cpuid.h>
#elif defined( ID_QNX_ARM )
#include <arm/vfp.h>
#else
#error Unknown CPU architecture
#endif

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
/*
================
IsAMD
================
*/
#if __GNUC__ >= 4 && __GNUC_MINOR__ >= 8
#define IsAMD() __builtin_cpu_is( "amd" )
#else
static bool IsAMD() {
	unsigned a, b, c, d;
	char processorString[13];

	// get name of processor
	__cpuid( 0, a, b, c, d );
	*( ( unsigned * )&processorString[0] ) = b;
	*( ( unsigned * )&processorString[4] ) = d;
	*( ( unsigned * )&processorString[8] ) = c;
	processorString[12] = 0;

	return strcmp( processorString, "AuthenticAMD" ) == 0;
}
#endif

/*
================
Has3DNow
================
*/
static bool Has3DNow() {
	unsigned a, b, c, d;

	// check AMD-specific functions
	__cpuid( 0x80000000, a, b, c, d );
	if ( a < 0x80000000 ) {
		return false;
	}

	// bit 31 of EDX denotes 3DNow! support
	__cpuid( 0x80000001, a, b, c, d );
	return d & bit_3DNOW;
}

/*
================
HasSSE3
================
*/
#if __GNUC__ >= 4 && __GNUC_MINOR__ >= 8
#define HasSSE3() __builtin_cpu_supports( "sse3" )
#else
static bool HasSSE3() {
	unsigned a, b, c, d;

	// get CPU feature bits
	__cpuid( 1, a, b, c, d );

	// bit 0 of ECX denotes SSE3 existence
	return c & bit_SSE3;
}
#endif

/*
================
HasHTT
================
*/
static bool HasHTT() {
	unsigned a, b, c, d;

	// get CPU feature bits
	__cpuid( 1, a, b, c, d );

	// bit 28 of EDX denotes HTT existence
	return d & ( 1 << 28 );
}

/*
================
IsDAZEnabled
================
*/
static bool IsDAZEnabled() {
	unsigned char FXSaveArea[512] __attribute__((__aligned__(16)));
	unsigned char *FXArea = FXSaveArea;
	unsigned int mask = 0;

	memset( FXArea, 0, sizeof( FXSaveArea ) );

	__asm__ __volatile__("FXSAVE (%[area])" : [area] "+r" (FXArea) :: "memory");

	mask = *(unsigned int *)&FXArea[28];			// Read the MXCSR Mask
	return ( ( mask & ( 1 << 6 ) ) == ( 1 << 6 ) );	// Return if the DAZ bit is set
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
cpuid_t Sys_GetCPUId() {
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

#if __GNUC__ >= 4 && __GNUC_MINOR__ >= 8
	// setup for checking CPU features
	__builtin_cpu_init();
#endif

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
	if ( ( cpuFlags & X86_CPU_FXSR ) != 0 && IsDAZEnabled() ) {
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

#ifdef ID_QNX_X86
static byte fpuState[128], *statePtr = fpuState;
#endif
static char fpuString[2048];
static bitFlag_t controlWordFlags[] = {
#ifdef ID_QNX_X86
	{ "Invalid operation", 0 },
	{ "Denormalized operand", 1 },
	{ "Divide-by-zero", 2 },
	{ "Numeric overflow", 3 },
	{ "Numeric underflow", 4 },
	{ "Inexact result (precision)", 5 },
	{ "Infinity control", 12 },
#elif defined(ID_QNX_ARM)
	{ "Invalid operation", 8 },
	{ "Divide-by-zero", 9 },
	{ "Numeric overflow", 10 },
	{ "Numeric underflow", 11 },
	{ "Inexact result (precision)", 12 },
	{ "Flush-to-Zero", 24 },
#else
#error Unknown CPU architecture
#endif
	{ "", 0 }
};
static char *precisionControlField[] = {
#ifdef ID_QNX_X86
	"Single Precision (24-bits)",
	"Reserved",
	"Double Precision (53-bits)",
	"Double Extended Precision (64-bits)"
#elif defined(ID_QNX_ARM)
	"Single Precision",
	"Double Precision",
	"Extended Precision",
	"Double Extended Precision"
#else
#error Unknown CPU architecture
#endif
};
static char *roundingControlField[] = {
	"Round to nearest",
#ifdef ID_QNX_X86
	"Round down",
	"Round up",
#elif defined(ID_QNX_ARM)
	"Round up",
	"Round down",
#else
#error Unknown CPU architecture
#endif
	"Round toward zero"
};
static bitFlag_t statusWordFlags[] = {
#ifdef ID_QNX_X86
	{ "Invalid operation", 0 },
	{ "Denormalized operand", 1 },
	{ "Divide-by-zero", 2 },
	{ "Numeric overflow", 3 },
	{ "Numeric underflow", 4 },
	{ "Inexact result (precision)", 5 },
	{ "Stack fault", 6 },
	{ "Error summary status", 7 },
	{ "FPU busy", 15 },
#elif defined(ID_QNX_ARM)
	{ "Invalid operation", 0 },
	{ "Divide-by-zero", 1 },
	{ "Numeric overflow", 2 },
	{ "Numeric underflow", 3 },
	{ "Inexact result (precision)", 4 },
#else
#error Unknown CPU architecture
#endif
	{ "", 0 }
};
#ifdef ID_QNX_ARM
static char *strideControlField[] = {
	"Stride of 1",
	"Invalid Stride",
	"Stride of 2"
};
#endif

/*
===============
Sys_FPU_PrintStateFlags
===============
*/
int Sys_FPU_PrintStateFlags( char *ptr, int ctrl, int stat, int tags, int inof, int inse, int opof, int opse ) {
	int i, length = 0;

#ifdef ID_QNX_ARM
	length += sprintf( ptr+length,	"CTRL-STAT = %08x\n"
									"PREC = %08x\n"
									"\n",
									ctrl, stat );

	length += sprintf( ptr+length, "Control:\n" );
	for ( i = 0; controlWordFlags[i].name[0]; i++ ) {
		length += sprintf( ptr+length, "  %-30s = %s\n", controlWordFlags[i].name, ( ctrl & ( 1 << controlWordFlags[i].bit ) ) ? "true" : "false" );
	}
	length += sprintf( ptr+length, "  %-30s = %s\n", "Precision control", precisionControlField[stat&3] );
	length += sprintf( ptr+length, "  %-30s = %s\n", "Rounding control", roundingControlField[(ctrl>>22)&3] );
	length += sprintf( ptr+length, "  %-30s = %s\n", "Vector Stride control", strideControlField[(ctrl>>20)&3] );
	length += sprintf( ptr+length, "  %-30s = %d\n", "Vector Length control", (((ctrl>>16)&7)+1) );

	length += sprintf( ptr+length, "Status:\n" );
	for ( i = 0; statusWordFlags[i].name[0]; i++ ) {
		ptr += sprintf( ptr+length, "  %-30s = %s\n", statusWordFlags[i].name, ( ctrl & ( 1 << statusWordFlags[i].bit ) ) ? "true" : "false" );
	}
	length += sprintf( ptr+length, "  %-30s = N-%d, Z-%d, C-%d, V-%d\n", "Condition code", (ctrl>>31)&1, (ctrl>>30)&1, (ctrl>>29)&1, (ctrl>>28)&1 );
#else
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
#endif

	return length;
}

/*
===============
Sys_FPU_StackIsEmpty
===============
*/
bool Sys_FPU_StackIsEmpty() {
#ifdef ID_QNX_ARM
	return true; //ARM does not use a stack for VFP or NEON operations
#elif defined(ID_QNX_X86)
	int ret;
	__asm__ __volatile__(
			"fnstenv	(%[state])\n"
			"mov		8(%[state]), %[state]\n"
			"xor		$0xFFFFFFFF, %[state]\n"
			"and		$0x0000FFFF, %[state]\n"
			"mov		%[state], %[ret]"
			: [ret] "=r" (ret) : [state] "r" (statePtr) : "memory");
	return ret == 0;
#else
#error Unknown CPU architecture
#endif
}

/*
===============
Sys_FPU_ClearStack
===============
*/
void Sys_FPU_ClearStack() {
#ifdef ID_QNX_ARM
	//ARM does not use a stack for VFP or NEON operations
#elif defined(ID_QNX_X86)
	__asm__ __volatile__(
			"fnstenv	(%[state])\n"
			"mov		8(%[state]), %[state]\n"
			"xor		$0xFFFFFFFF, %[state]\n"
			"mov		$(3<<14), %%edx\n"
		"emptyStack:\n"
			"mov		%[state], %%ecx\n"
			"and		%%edx, %%ecx\n"
			"jz			done\n"
			"fstp		%%st\n"
			"shr		$2, %%edx\n"
			"jmp		emptyStack\n"
		"done:"
			:: [state] "r" (statePtr) : "ecx", "edx", "memory");
#else
#error Unknown CPU architecture
#endif
}

/*
===============
Sys_FPU_GetState

  gets the FPU state without changing the state
===============
*/
const char *Sys_FPU_GetState() {
	char *ptr;
	int i;
#ifdef ID_QNX_X86
	double fpuStack[8] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
	int numValues;

	__asm__ __volatile__(
			"fnstenv	(%[state])\n"
			"mov		8(%[state]), %[state]\n"
			"xor		$0xFFFFFFFF, %[state]\n"
			"mov		$(3<<14), %%edx\n"
			"mov		%[state], %%ecx\n"
			"xor		%[numValues], %[numValues]\n"
			"and		%%edx, %%ecx\n"
			"jz			done\n"
			"fstl		0(%[fpuStack])\n"
			"inc		%[numValues]\n"
			"shr		$2, %%edx\n"
			"mov		%[state], %%ecx\n"
			"and		%%edx, %%ecx\n"
			"jz			done\n"
			"fxch		%%st(1)\n"
			"fstl		8(%[fpuStack])\n"
			"inc		%[numValues]\n"
			"fxch		%%st(1)\n"
			"shr		$2, %%edx\n"
			"mov		%[state], %%ecx\n"
			"and		%%edx, %%ecx\n"
			"jz			done\n"
			"fxch		%%st(2)\n"
			"fstl		16(%[fpuStack])\n"
			"inc		%[numValues]\n"
			"fxch		%%st(2)\n"
			"shr		$2, %%edx\n"
			"mov		%[state], %%ecx\n"
			"and		%%edx, %%ecx\n"
			"jz			done\n"
			"fxch		%%st(3)\n"
			"fstl		24(%[fpuStack])\n"
			"inc		%[numValues]\n"
			"fxch		%%st(3)\n"
			"shr		$2, %%edx\n"
			"mov		%[state], %%ecx\n"
			"and		%%edx, %%ecx\n"
			"jz			done\n"
			"fxch		%%st(4)\n"
			"fstl		32(%[fpuStack])\n"
			"inc		%[numValues]\n"
			"fxch		%%st(4)\n"
			"shr		$2, %%edx\n"
			"mov		%[state], %%ecx\n"
			"and		%%edx, %%ecx\n"
			"jz			done\n"
			"fxch		%%st(5)\n"
			"fstl		40(%[fpuStack])\n"
			"inc		%[numValues]\n"
			"fxch		%%st(5)\n"
			"shr		$2, %%edx\n"
			"mov		%[state], %%ecx\n"
			"and		%%edx, %%ecx\n"
			"jz			done\n"
			"fxch		%%st(6)\n"
			"fstl		48(%[fpuStack])\n"
			"inc		%[numValues]\n"
			"fxch		%%st(6)\n"
			"shr		$2, %%edx\n"
			"mov		%[state], %%ecx\n"
			"and		%%edx, %%ecx\n"
			"jz			done\n"
			"fxch		%%st(7)\n"
			"fstl		56(%[fpuStack])\n"
			"inc		%[numValues]\n"
			"fxch		%%st(7)\n"
		"done:\n"
			: [numValues] "=r" (numValues) : [state] "r" (statePtr), [fpuStack] "r" (fpuStack) : "ecx", "edx", "memory");

	int ctrl = *(int *)&fpuState[0];
	int stat = *(int *)&fpuState[4];
	int tags = *(int *)&fpuState[8];
	int inof = *(int *)&fpuState[12];
	int inse = *(int *)&fpuState[16];
	int opof = *(int *)&fpuState[20];
	int opse = *(int *)&fpuState[24];
#elif defined(ID_QNX_ARM)
	int ctrlStat = 0;
#ifdef ID_QNX_ARM_NEON_ASM
	__asm__("VMRS %[ctst], FPSCR" : [ctst] "=r" (ctrlStat) ::); //VFP would use FMRX instead of VMRS
#else
#error Must compile with -mfpu=neon
#endif

	int prec = fp_precision( -1 );
#else
#error Unknown CPU architecture
#endif

	ptr = fpuString;
#ifdef ID_QNX_X86
	ptr += sprintf( ptr,"FPU State:\n"
						"num values on stack = %d\n", numValues );
	for ( i = 0; i < 8; i++ ) {
		ptr += sprintf( ptr, "ST%d = %1.10e\n", i, fpuStack[i] );
	}

	Sys_FPU_PrintStateFlags( ptr, ctrl, stat, tags, inof, inse, opof, opse );
#elif defined(ID_QNX_ARM)
	ptr += sprintf( ptr,"FPU State:\n" );

	Sys_FPU_PrintStateFlags( ptr, ctrlStat, prec, 0, 0, 0, 0, 0 );
#else
#error Unknown CPU architecture
#endif

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

	switch( precision ) {
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
	unsigned int data;

	__asm__("movzxb		%[enable], %%ecx\n"
			"and		$1, %%ecx\n"
			"shl		$6, %%ecx\n"
			"STMXCSR	%[data]\n"
			"mov		%[data], %%eax\n"
			"and		$(~(1<<6)), %%eax\n"	// clear DAX bit
			"or			%%ecx, %%eax\n"			// set the DAZ bit
			"mov		%%eax, %[data]\n"
			"LDMXCSR	%[data]"
			:: [enable] "r" (enable), [data] "g" (data) : "eax", "ecx");
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

#ifdef ID_QNX_ARM_NEON_ASM
	//NEON: VMRS, VMSR
	//VFP:  FMRX, FMXR
	__asm__("MOV	r1, %[enable]\n"
			"AND	r1, #0x1\n"
			"LSL	r1, #24\n"
			"VMRS	r0, FPSCR\n"
			"BIC	r0, %[FZ_BIT]\n"	//Clear FTZ bit
			"ORR	r0, r1\n"			//Set FTZ bit
			"VMSR	FPSCR, r0"
			:: [enable] "r" (enable), [FZ_BIT] "I" (ARM_VFP_FPSCR_FZ) : "r0", "r1");
#else
#error Must compile with -mfpu=neon
#endif

#elif defined(ID_QNX_X86)
	unsigned int data;

	__asm__("movzxb		%[enable], %%ecx\n"
			"and		$1, %%ecx\n"
			"shl		$15, %%ecx\n"
			"STMXCSR	%[data]\n"
			"mov		%[data], %%eax\n"
			"and		$(~(1<<15)), %%eax\n"	// clear FTZ bit
			"or			%%ecx, %%eax\n"			// set the FTZ bit
			"mov		%%eax, %[data]\n"
			"LDMXCSR	%[data]"
			:: [enable] "r" (enable), [data] "g" (data) : "eax", "ecx");
#else
#error Unknown CPU architecture
#endif
}

/*
================
Sys_FPU_VFP_SetLEN_STRIDE

Used by ARM VFP to process vectors
================
*/
void Sys_FPU_VFP_SetLEN_STRIDE( int length, int stride ) {
#ifdef ID_QNX_ARM

#ifdef ID_QNX_ARM_NEON_ASM
	__asm__("VMRS r0, FPSCR\n"
			"BIC r0, %[mask]\n"
			"MOV r1, %[stride]\n"
			"AND r1, #3\n"
			"LSL r1, #4\n"
			"ORR r1, %[len]\n"
			"AND r1, #((3<<4)|7)\n"
			"LSL r1, #16\n"
			"ORR r0, r1\n"
			"VMSR FPSCR, r0"
			:: [stride] "r" (stride), [len] "r" (length), [mask] "I" (ARM_VFP_FPSCR_LEN|ARM_VFP_FPSCR_STRIDE) : "r0", "r1");
#else
#error Must compile with -mfpu=neon
#endif

#endif
}
