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
#include "../../precompiled.h"
#include <pthread.h>
#include <sched.h>
#include <atomic.h>
#include <time.h>
#include <errno.h>

// There is no "exchange" or "compare and exchange" functions on QNX for some reason, so we need to go low level
#include _NTO_CPU_HDR_(smpxchg.h)

/*
================================================================================================
================================================================================================
*/

/*
========================
Sys_SetCurrentThreadName
========================
*/
void Sys_SetCurrentThreadName( const char * name ) {
	pthread_setname_np( pthread_self(), name );
}

/*
========================
Sys_Createthread
========================
*/
/* 80 % duty cycle of 8 secs on 2 secs off */
struct timespec thread_init_budget = { 2, 0 };
struct timespec thread_repl_period = { 10, 0 };

// Whenever the thread blocks or reaches it's execution limit, it has to be "replenished".
// If this happens too much, the scheduling algorithm becomes a burden. So there is a upper-limit to the
// number of replenishments that will happen before the thread is set as a lower priority.
// The value 15 is arbitrary as there is no real upper or lower limit and it's unknown how often a thread might block.
#define QNX_THREAD_MAX_REPL 15

typedef void *(*pthread_function_t)(void *);

uintptr_t Sys_CreateThread( xthread_t function, void *parms, xthreadPriority priority, const char *name, core_t core, int stackSize, bool suspended ) {

	pthread_attr_t attr;
	pthread_attr_init( &attr );
	pthread_attr_setstacksize( &attr, stackSize );
	//XXX suspended
	// Ignore core affinity. Let QNX handle it. Though if we did want to handle it, we would use ThreadCtl(_NTO_TCTL_RUNMASK, ...) within it

	//Setup thread scheduling (based off http://developer.blackberry.com/native/reference/core/com.qnx.doc.neutrino.lib_ref/topic/s/sched_param.html)
	struct sched_param params;
	pthread_attr_setinheritsched( &attr, PTHREAD_EXPLICIT_SCHED );
	pthread_attr_setschedpolicy( &attr, SCHED_SPORADIC );

	int minPriority = sched_get_priority_min( SCHED_SPORADIC );
	int maxPriority = sched_get_priority_max( SCHED_SPORADIC );
	int priorityOffset = ( maxPriority - minPriority ) / 4;

#define QNX_THREAD_PRIORITY(level) ( ( priorityOffset * (level) ) + minPriority + 1 )

	// sched_priority must be between minPriority and maxPriority and higher then sched_ss_low_priority.
	// So QNX_THREAD_PRIORITY is 1 higher then minPriority, while sched_ss_low_priority will be minPriority.
	// For high priority threads, the sched_ss_low_priority will be higher then minPriority to a degree.
	params.sched_ss_low_priority = minPriority;
	if ( priority == THREAD_HIGHEST ) {
		params.sched_priority = QNX_THREAD_PRIORITY( 4 );
		params.sched_ss_low_priority = QNX_THREAD_PRIORITY( 2 );
	} else if ( priority == THREAD_ABOVE_NORMAL ) {
		params.sched_priority = QNX_THREAD_PRIORITY( 3 );
		params.sched_ss_low_priority = QNX_THREAD_PRIORITY( 1 );
	} else if ( priority == THREAD_NORMAL ) {
		params.sched_priority = QNX_THREAD_PRIORITY( 2 );
		params.sched_ss_low_priority = QNX_THREAD_PRIORITY( 0 );
	} else if ( priority == THREAD_BELOW_NORMAL ) {
		params.sched_priority = QNX_THREAD_PRIORITY( 1 );
	} else if ( priority == THREAD_LOWEST ) {
		params.sched_priority = QNX_THREAD_PRIORITY( 0 );
	}
	if ( params.sched_priority > maxPriority ) {
		params.sched_priority = minPriority;
	}

	params.sched_ss_max_repl = QNX_THREAD_MAX_REPL;
	memcpy( &params.sched_ss_init_budget, &thread_init_budget, sizeof( thread_init_budget ) );
	memcpy( &params.sched_ss_repl_period, &thread_repl_period, sizeof( thread_repl_period ) );

	pthread_attr_setschedparam( &attr, &params );

	//Create thread and set name
	pthread_t thread;
	int ret = pthread_create(&thread, &attr, ( pthread_function_t )function, parms);
	pthread_attr_destroy( &attr );
	if ( ret != EOK ) {
		idLib::common->FatalError( "pthread_create error: %i", ret );
		return (uintptr_t)0;
	}
	pthread_setname_np( thread, name );

	return (uintptr_t)thread;
}

/*
========================
Sys_GetCurrentThreadID
========================
*/
uintptr_t Sys_GetCurrentThreadID() {
	return (uintptr_t)pthread_self();
}

/*
========================
Sys_WaitForThread
========================
*/
void Sys_WaitForThread( uintptr_t threadHandle ) {
	pthread_join( ( pthread_t )threadHandle, NULL );
}

/*
========================
Sys_DestroyThread
========================
*/
void Sys_DestroyThread( uintptr_t threadHandle ) {
	if ( threadHandle == 0 ) {
		return;
	}
	pthread_cancel( ( pthread_t )threadHandle );
	pthread_join( ( pthread_t )threadHandle, NULL );
}

/*
========================
Sys_Yield
========================
*/
void Sys_Yield() {
	sched_yield();
}

/*
================================================================================================

	Signal

================================================================================================
*/

/*
========================
Sys_SignalCreate
========================
*/
void Sys_SignalCreate( signalHandle_t & handle, bool manualReset ) {
	handle = new (TAG_THREAD) idSysThreadSignal( manualReset, false );
}

/*
========================
Sys_SignalDestroy
========================
*/
void Sys_SignalDestroy( signalHandle_t &handle ) {
	if ( handle ) {
		delete handle;
	}
}

/*
========================
Sys_SignalRaise
========================
*/
void Sys_SignalRaise( signalHandle_t & handle ) {
	pthread_mutex_lock( &handle->mutex );
	handle->triggered = true;
	pthread_cond_signal( &handle->cond );
	pthread_mutex_unlock( &handle->mutex );
}

/*
========================
Sys_SignalClear
========================
*/
void Sys_SignalClear( signalHandle_t & handle ) {
	pthread_mutex_lock( &handle->mutex );
	handle->triggered = false;
	pthread_mutex_unlock( &handle->mutex );
}

/*
========================
Sys_SignalWait_ErrorReturn
========================
*/
int Sys_SignalWait_ErrorReturn( signalHandle_t & handle, int timeout ) {
	int ret = EOK;
	pthread_mutex_lock( &handle->mutex );
	if ( timeout == idSysSignal::WAIT_INFINITE ) {
		while ( !handle->triggered ) {
			ret = pthread_cond_wait( &handle->cond, &handle->mutex );
		}
		if ( ret == EOK && !handle->manualReset ) {
			handle->triggered = false;
		}
	} else {
		struct timespec tm;
		// Get the time, add the timeout "offset", wait. If it works, then we're good. If it doesn't, then we get the error
		clock_gettime( CLOCK_MONOTONIC, &tm );
		if ( timeout != 0 ) {
			nsec2timespec( &tm, timespec2nsec( &tm ) + ( timeout * 1000000UL ) ); //Millisecond to nanosecond
		}
		while ( !handle->triggered && ret == EOK ) {
			ret = pthread_cond_timedwait( &handle->cond, &handle->mutex, &tm );
		}
		if ( ret == EOK && !handle->manualReset ) {
			handle->triggered = false;
		}
	}
	pthread_mutex_unlock( &handle->mutex );
	return ret;
}

/*
========================
Sys_SignalWait
========================
*/
bool Sys_SignalWait( signalHandle_t & handle, int timeout ) {
	int result = Sys_SignalWait_ErrorReturn( handle, timeout );
	assert( result == 0 || ( timeout != idSysSignal::WAIT_INFINITE && result == ETIMEDOUT ) );
	return ( result == 0 );
}

/*
================================================================================================

	Mutex

================================================================================================
*/

/*
========================
Sys_MutexCreate
========================
*/
void Sys_MutexCreate( mutexHandle_t & handle ) {
	pthread_mutexattr_t attr;
	pthread_mutexattr_init( &attr );
	pthread_mutexattr_setrecursive( &attr, PTHREAD_RECURSIVE_ENABLE );
	pthread_mutex_init( &handle, &attr );
	pthread_mutexattr_destroy( &attr );
}

/*
========================
Sys_MutexDestroy
========================
*/
void Sys_MutexDestroy( mutexHandle_t & handle ) {
	pthread_mutex_destroy( &handle );
}

/*
========================
Sys_MutexLock
========================
*/
bool Sys_MutexLock( mutexHandle_t & handle, bool blocking ) {
	if ( pthread_mutex_trylock( &handle ) != EOK ) {
		if ( !blocking ) {
			return false;
		}
		pthread_mutex_lock( &handle );
	}
	return true;
}

/*
========================
Sys_MutexUnlock
========================
*/
void Sys_MutexUnlock( mutexHandle_t & handle ) {
	pthread_mutex_unlock( &handle );
}

/*
================================================================================================

	Interlocked Integer

================================================================================================
*/

/*
========================
Sys_InterlockedIncrement
========================
*/
interlockedInt_t Sys_InterlockedIncrement( interlockedInt_t & value ) {
	return ( interlockedInt_t )(atomic_add_value( ( unsigned* )( &value ), 1 ) + 1);
}

/*
========================
Sys_InterlockedDecrement
========================
*/
interlockedInt_t Sys_InterlockedDecrement( interlockedInt_t & value ) {
	return ( interlockedInt_t )(atomic_sub_value( ( unsigned* )( &value ), 1 ) - 1);
}

/*
========================
Sys_InterlockedAdd
========================
*/
interlockedInt_t Sys_InterlockedAdd( interlockedInt_t & value, interlockedInt_t i ) {
	return ( interlockedInt_t )(atomic_add_value( ( unsigned* )( &value ), ( unsigned )i ) + i);
}

/*
========================
Sys_InterlockedSub
========================
*/
interlockedInt_t Sys_InterlockedSub( interlockedInt_t & value, interlockedInt_t i ) {
	return ( interlockedInt_t )(atomic_sub_value( ( unsigned* )( &value ), ( unsigned )i ) - i);
}

/*
========================
Sys_InterlockedExchange
========================
*/
interlockedInt_t Sys_InterlockedExchange( interlockedInt_t & value, interlockedInt_t exchange ) {
	return ( interlockedInt_t )_smp_xchg( ( unsigned* )( &value ), ( unsigned )exchange );
}

/*
========================
Sys_InterlockedCompareExchange
========================
*/
interlockedInt_t Sys_InterlockedCompareExchange( interlockedInt_t & value, interlockedInt_t comparand, interlockedInt_t exchange ) {
	return ( interlockedInt_t )_smp_cmpxchg( ( unsigned* )( &value ), ( unsigned )comparand, ( unsigned )exchange );
}

/*
================================================================================================

	Interlocked Pointer

================================================================================================
*/

/*
========================
Sys_InterlockedExchangePointer
========================
*/
void *Sys_InterlockedExchangePointer( void *& ptr, void * exchange ) {
#if __SIZEOF_POINTER__ != __SIZEOF_INT__
#error void* is not the same size as int. Exchange will fail
#endif
	return ( void* )_smp_xchg( ( unsigned* )( &ptr ), ( unsigned )exchange );
}

/*
========================
Sys_InterlockedCompareExchangePointer
========================
*/
void * Sys_InterlockedCompareExchangePointer( void * & ptr, void * comparand, void * exchange ) {
#if __SIZEOF_POINTER__ != __SIZEOF_INT__
#error void* is not the same size as int. Compare-Exchange will fail
#endif
	return ( void* )_smp_cmpxchg( ( unsigned* )( &ptr ), ( unsigned )comparand, ( unsigned )exchange );
}
