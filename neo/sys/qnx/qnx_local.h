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

#ifndef __QNX_LOCAL_H__
#define __QNX_LOCAL_H__

#include "../../renderer/OpenGL/qgl_es.h"
#include "qnx_input.h"

#include <sys/slog2.h>
#include <bps/navigator.h>
#include <bps/screen.h>

#define BATTERY_MIN_TO_LOW_BATTERY_WARNING 20
#define SLOG_BUFFER_COUNT 1

void	Sys_QueEvent( sysEventType_t type, int value, int value2, int ptrLength, void *ptr, int inputDeviceNum );

cpuid_t	Sys_GetCPUId();

uint64	Sys_Microseconds();

bool	EmailCrashReport( const char* messageText );

typedef struct {
	cpuid_t						cpuid;

	slog2_buffer_t				logBuffers[SLOG_BUFFER_COUNT];

	// System state vars
	bool						quitStarted;
	navigator_window_state_t	windowState;

	// Input vars
	int							mouseWheelPosition;
	int							mouseButtonsPressed;
	idJoystickQnx				joystick;

	// Polling input vars
	int							polledMouseWheelPosition;
	int							polledMouseWheelPositionOld;
	int							polledMouseButtonsPressed;
	int							polledMouseButtonsPressedOld;
	int							polledMouseX; // Must be followed by polledMouseY
	int							polledMouseY;
	int							polledMouseXOld;
	int							polledMouseYOld;
	bool						polledKeys[KEYBOARD_KEYS];
	bool						polledKeysOld[KEYBOARD_KEYS];

	// GL vars
	void*						eglLib;
	void*						openGLLib;
	bool						glLoggingInit;
	int							screenDisplayID;
	screen_context_t			screenCtx;
	screen_window_t				screenWin;

	// EGL vars
	EGLContext					eglContext;
	EGLDisplay					eglDisplay;
	EGLSurface					eglSurface;
	EGLConfig					eglConfig;

	// GLES Framebuffer vars
	//TODO: framebuffers (4 symbols for them [front, back (aka, back_left), back_left, back_right])
	GLenum						frontBuffer;
	GLenum						backBuffer;

	// SMP acceleration vars
	signalHandle_t				renderCommandsEvent;
	signalHandle_t				renderCompletedEvent;
	signalHandle_t				renderActiveEvent;
	uintptr_t					renderThread;
	void*						smpData;

	// CVar
	static idCVar				sys_arch;
	static idCVar				sys_cpustring;
} QNXVars_t;

extern idCVar qnx_skipPointerPolling;

extern QNXVars_t	qnx;

typedef struct {
	void ( * glReadBufferImpl )(GLenum);
	void ( * glDrawBufferImpl )(GLenum);
} EGLFunctionReplacements_t;

#endif /* !__QNX_LOCAL_H__ */
