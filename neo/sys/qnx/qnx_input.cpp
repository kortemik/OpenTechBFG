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

/*
===========
Sys_ShutdownInput
===========
*/
void Sys_ShutdownInput() {
	if ( screen_stop_events( qnx.screenCtx ) != BPS_SUCCESS ) {
		idLib::Warning( "Sys_ShutdownInput: Could not stop screen events" );
	}
	//TODO: cleanup gamepads
}

/*
===========
Sys_InitInput
===========
*/
void Sys_InitInput() {
	common->Printf ("\n------- Input Initialization -------\n");

	//TODO: setup gamepads (find them and setup list for more to be loaded)
	if ( screen_request_events( qnx.screenCtx ) != BPS_SUCCESS ) {
		idLib::FatalError( "Sys_InitInput: Could not start requesting screen events" );
	}

	common->Printf ("------------------------------------\n");
}

//XXX Can query screen context for devices (which can then be used for polling)

//=====================================================================================
//	Keyboard Input Handling
//=====================================================================================

/*
===========
Sys_PollKeyboardInputEvents
===========
*/
int Sys_PollKeyboardInputEvents() {
	//TODO
	return 0;
}

/*
===========
Sys_ReturnKeyboardInputEvent
===========
*/
int Sys_ReturnKeyboardInputEvent( const int n, int &ch, bool &state ) {
	//TODO
	return 0;
}

/*
===========
Sys_EndKeyboardInputEvents
===========
*/
void Sys_EndKeyboardInputEvents() {
	//TODO
}

//=====================================================================================
//	Mouse Input Handling
//=====================================================================================

/*
===========
Sys_PollMouseInputEvents
===========
*/
int Sys_PollMouseInputEvents( int mouseEvents[MAX_MOUSE_EVENTS][2] ) {
	//TODO
	return 0;
}

/*
===========
Sys_GrabMouseCursor
===========
*/
void Sys_GrabMouseCursor( bool grabIt ) {
	//TODO
}

//=====================================================================================
//	Joystick Input Handling
//=====================================================================================

/*
===========
Sys_SetRumble
===========
*/
void Sys_SetRumble( int device, int low, int hi ) {
	//TODO
}

/*
===========
Sys_PollJoystickInputEvents
===========
*/
int Sys_PollJoystickInputEvents( int deviceNum ) {
	//TODO
	return 0;
}

/*
===========
Sys_ReturnJoystickInputEvent
===========
*/
int Sys_ReturnJoystickInputEvent( const int n, int &action, int &value ) {
	//TODO
	return 0;
}

/*
===========
Sys_EndJoystickInputEvents
===========
*/
void Sys_EndJoystickInputEvents() {
	//TODO
}
