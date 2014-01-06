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
#include "../../idlib/precompiled.h"

#include "qnx_local.h"
#include <screen/screen.h>

idCVar qnx_skipPointerPolling( "qnx_skipMousePolling", "0", CVAR_SYSTEM | CVAR_ARCHIVE | CVAR_INTEGER, "1 = poll, but don't make mouse system events, 2 = don't poll mouse" );

/*
===========
Sys_ShutdownInput
===========
*/
void Sys_ShutdownInput() {
	if ( screen_stop_events( qnx.screenCtx ) != BPS_SUCCESS ) {
		idLib::Warning( "Sys_ShutdownInput: Could not stop screen events" );
	}
}

/*
===========
Sys_InitInput
===========
*/
void Sys_InitInput() {
	common->Printf ("\n------- Input Initialization -------\n");

	if ( !qnx.joystick.Init() ) {
		idLib::FatalError( "Sys_InitInput: Could not initialize joysticks/gamepads" );
	}

	if ( screen_request_events( qnx.screenCtx ) != BPS_SUCCESS ) {
		idLib::FatalError( "Sys_InitInput: Could not start requesting screen events" );
	}

	common->Printf ("------------------------------------\n");
}

//=====================================================================================
//	Keyboard Input Handling
//=====================================================================================

static struct pollKeys_s {
	int		ch;
	bool	state;
} pollKeys[KEYBOARD_KEYS];

/*
===========
Sys_PollKeyboardInputEvents
===========
*/
int Sys_PollKeyboardInputEvents() {
	int numEvents = 0;

	for ( int i = 0; i < KEYBOARD_KEYS; i++ ) {
		if ( qnx.polledKeys[i] != qnx.polledKeysOld[i] ) {
			pollKeys[numEvents].ch = i;
			pollKeys[numEvents].state = qnx.polledKeys[i];
			numEvents++;
		}
	}
	memcpy( qnx.polledKeysOld, qnx.polledKeys, sizeof( qnx.polledKeys ) );

	return numEvents;
}

/*
===========
Sys_ReturnKeyboardInputEvent
===========
*/
int Sys_ReturnKeyboardInputEvent( const int n, int &ch, bool &state ) {
	ch = pollKeys[n].ch;
	state = pollKeys[n].state;
	return ch;
}

/*
===========
Sys_EndKeyboardInputEvents
===========
*/
void Sys_EndKeyboardInputEvents() {
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
	int numEvents = 0;

	int pollingType = qnx_skipPointerPolling.GetInteger();
	if ( pollingType == 2 ) {
		return numEvents;
	}

	if ( qnx.polledMouseButtonsPressed != qnx.polledMouseButtonsPressedOld ) {
		// Left mouse
		mouseEvents[numEvents][0] = M_INVALID;
		mouseEvents[numEvents][1] = 0;

		if ( qnx.polledMouseButtonsPressed & SCREEN_LEFT_MOUSE_BUTTON ) {
			if ( !( qnx.polledMouseButtonsPressed & SCREEN_LEFT_MOUSE_BUTTON ) ) {
				mouseEvents[numEvents][0] = M_ACTION1;
				mouseEvents[numEvents][1] = true;
				numEvents++;
				if ( pollingType != 1 ) {
					Sys_QueEvent( SE_KEY, K_MOUSE1, true, 0, NULL, 0 );
				}
			}
		} else if ( qnx.polledMouseButtonsPressed & SCREEN_LEFT_MOUSE_BUTTON ) {
			mouseEvents[numEvents][0] = M_ACTION1;
			mouseEvents[numEvents][1] = false;
			numEvents++;
			if ( pollingType != 1 ) {
				Sys_QueEvent( SE_KEY, K_MOUSE1, false, 0, NULL, 0 );
			}
		}

		// Right mouse
		mouseEvents[numEvents][0] = M_INVALID;
		mouseEvents[numEvents][1] = 0;

		if ( qnx.polledMouseButtonsPressed & SCREEN_RIGHT_MOUSE_BUTTON ) {
			if ( !( qnx.polledMouseButtonsPressed & SCREEN_RIGHT_MOUSE_BUTTON ) ) {
				mouseEvents[numEvents][0] = M_ACTION2;
				mouseEvents[numEvents][1] = true;
				numEvents++;
				if ( pollingType != 1 ) {
					Sys_QueEvent( SE_KEY, K_MOUSE2, true, 0, NULL, 0 );
				}
			}
		} else if ( qnx.polledMouseButtonsPressed & SCREEN_RIGHT_MOUSE_BUTTON ) {
			mouseEvents[numEvents][0] = M_ACTION2;
			mouseEvents[numEvents][1] = false;
			numEvents++;
			if ( pollingType != 1 ) {
				Sys_QueEvent( SE_KEY, K_MOUSE2, false, 0, NULL, 0 );
			}
		}

		// Right mouse
		mouseEvents[numEvents][0] = M_INVALID;
		mouseEvents[numEvents][1] = 0;

		if ( qnx.polledMouseButtonsPressed & SCREEN_MIDDLE_MOUSE_BUTTON ) {
			if ( !( qnx.polledMouseButtonsPressed & SCREEN_MIDDLE_MOUSE_BUTTON ) ) {
				mouseEvents[numEvents][0] = M_ACTION3;
				mouseEvents[numEvents][1] = true;
				numEvents++;
				if ( pollingType != 1 ) {
					Sys_QueEvent( SE_KEY, K_MOUSE3, true, 0, NULL, 0 );
				}
			}
		} else if ( qnx.polledMouseButtonsPressed & SCREEN_MIDDLE_MOUSE_BUTTON ) {
			mouseEvents[numEvents][0] = M_ACTION3;
			mouseEvents[numEvents][1] = false;
			numEvents++;
			if ( pollingType != 1 ) {
				Sys_QueEvent( SE_KEY, K_MOUSE3, false, 0, NULL, 0 );
			}
		}

		qnx.polledMouseButtonsPressedOld = qnx.polledMouseButtonsPressed;
	}

	if ( qnx.polledMouseX != qnx.polledMouseXOld ) {
		const int rel = qnx.polledMouseXOld - qnx.polledMouseX;
		mouseEvents[numEvents][0] = M_DELTAX;
		mouseEvents[numEvents][1] = rel;
		numEvents++;
		if ( pollingType != 1 ) {
			Sys_QueEvent( SE_MOUSE, rel, 0, 0, NULL, 0 );
		}
		qnx.polledMouseXOld = qnx.polledMouseX;
	}

	if ( qnx.polledMouseY != qnx.polledMouseYOld ) {
		const int rel = qnx.polledMouseYOld - qnx.polledMouseY;
		mouseEvents[numEvents][0] = M_DELTAY;
		mouseEvents[numEvents][1] = rel;
		numEvents++;
		if ( pollingType != 1 ) {
			Sys_QueEvent( SE_MOUSE, 0, rel, 0, NULL, 0 );
		}
		qnx.polledMouseYOld = qnx.polledMouseY;
	}

	if ( qnx.polledMouseWheelPosition != qnx.polledMouseWheelPositionOld ) {
		const int value = ( qnx.polledMouseWheelPositionOld - qnx.polledMouseWheelPosition ) / WHEEL_DELTA;
		mouseEvents[numEvents][0] = M_DELTAZ;
		mouseEvents[numEvents][1] = value;
		numEvents++;
		if ( pollingType != 1 ) {
			const int key = value < 0 ? K_MWHEELDOWN : K_MWHEELUP;
			const int iterations = abs( value );
			for ( int i = 0; i < iterations; i++ ) {
				Sys_QueEvent( SE_KEY, key, true, 0, NULL, 0 );
				Sys_QueEvent( SE_KEY, key, false, 0, NULL, 0 );
			}
		}
		qnx.polledMouseWheelPositionOld = qnx.polledMouseWheelPosition;
	}

	return numEvents;
}

/*
===========
Sys_GrabMouseCursor
===========
*/
void Sys_GrabMouseCursor( bool grabIt ) {
}

//=====================================================================================
//	Joystick Input Handling
//=====================================================================================

static struct {
	int vid;
	int did;
	const char *vendor;
	const char *name;
	bool hasAnalogTriggers;
} knownJoysticks[] = {
	// VID		 DID	 Vendor			 Name			 Analog Triggers?
	{ 0x057E,	0x0306,	"Nintendo",		"Wii Remote",			false },
	{ 0x20D6,	0x0DAD,	"MOGA",			"Pro",					true },
	//{ 0x20D6,	?,		"MOGA",			"Hero Power",			true },
	//{ 0x20D6,	?,		"MOGA",			"Pro Power",			true },
	{ 0x1038,	0x1412,"SteelSeries",	"FREE",					false },
	//{ ?,		?,		"Gametel",		"Controller",			false },
	//{ ?,		?,		"Logitech",		"F310",					true },
	//{ ?,		?,		"Logitech",		"F710",					true },
	//{ ?,		?,		"Microsoft",	"Xbox 360 Controller",	true },
	//{ ?,		?,		"Razer",		"Sabertooth",			true },
	{ 0,		0,		NULL,			NULL,					false }
};

/*
===========
Sys_SetRumble
===========
*/
void Sys_SetRumble( int device, int low, int hi ) {
	qnx.joystick.SetRumble( device, low, hi );
}

/*
===========
Sys_PollJoystickInputEvents
===========
*/
int Sys_PollJoystickInputEvents( int deviceNum ) {
	return qnx.joystick.PollInputEvents( deviceNum );
}

/*
===========
Sys_ReturnJoystickInputEvent
===========
*/
int Sys_ReturnJoystickInputEvent( const int n, int &action, int &value ) {
	return qnx.joystick.ReturnInputEvent( n, action, value );
}

/*
===========
Sys_EndJoystickInputEvents
===========
*/
void Sys_EndJoystickInputEvents() {
}

/*
========================
idJoystickQnx::idJoystickQnx
========================
*/
idJoystickQnx::idJoystickQnx() {
	numEvents = 0;
	memset( &events, 0, sizeof( events ) );
	memset( &controllers, 0, sizeof( controllers ) );
	memset( buttonStates, 0, sizeof( buttonStates ) );
	memset( joyAxis, 0, sizeof( joyAxis ) );
}

/*
========================
idJoystickQnx::Init
========================
*/
bool idJoystickQnx::Init() {
	idJoystick::Init();

	// Reset handles
	for ( int i = 0 ; i < MAX_JOYSTICKS ; i++ ) {
		controllers[i].handle = NULL;
	}

	if ( qnx.screenCtx == NULL ) {
		return false;
	}

	int deviceCount;
	screen_get_context_property_iv( qnx.screenCtx, SCREEN_PROPERTY_DEVICE_COUNT, &deviceCount );

	screen_device_t* devices = new (TAG_TEMP) screen_device_t[deviceCount];

	screen_get_context_property_pv( qnx.screenCtx, SCREEN_PROPERTY_DEVICES, (void**)devices );

	for ( int i = 0 ; i < deviceCount && controllers[MAX_JOYSTICKS - 1].handle == NULL ; i++ ) {
	    int type = 0;
	    screen_get_device_property_iv( devices[i], SCREEN_PROPERTY_TYPE, &type );

	    if ( type == SCREEN_EVENT_GAMEPAD || type == SCREEN_EVENT_JOYSTICK ) {
	    	UpdateDevice( true, devices[i] );
	    }
	}

	delete[] devices;

	return true;
}

/*
========================
idJoystickQnx::SetRumble
========================
*/
void idJoystickQnx::UpdateDevice( bool attached, screen_device_t device ) {
	// expect that device is either SCREEN_EVENT_GAMEPAD or SCREEN_EVENT_JOYSTICK
	for ( int i = 0 ; i < MAX_JOYSTICKS ; i++ ) {
		if ( attached && controllers[i].handle == NULL ) {
			// attach controller
			controllers[i].handle = device;
			controllers[i].infoIndex = -1;

			screen_get_device_property_cv( device, SCREEN_PROPERTY_ID_STRING, sizeof( controllers[i].id ), controllers[i].id );
			screen_get_device_property_iv( device, SCREEN_PROPERTY_BUTTON_COUNT, &controllers[i].buttonCount );
			controllers[i].analogCount = 0;

			screen_get_device_property_iv( device, SCREEN_PROPERTY_BUTTONS, &controllers[i].buttonBits );
			if ( !screen_get_device_property_iv( device, SCREEN_PROPERTY_ANALOG0, controllers[i].analog0 ) ) {
				controllers[i].analogCount++;
			}
			if ( !screen_get_device_property_iv( device, SCREEN_PROPERTY_ANALOG1, controllers[i].analog1 ) ) {
				controllers[i].analogCount++;
			}

			memcpy( &controllers[i].pButtonBits, &controllers[i].buttonBits, sizeof( int ) * 7 );

			// Try to determine controller
			int vendorId = -1;
			int productId = -1;
			char* token = strtok( controllers[i].id, "-" );
			token = strtok( NULL, "-" );
			if ( token ) {
				vendorId = strtol( token, NULL, 16 );
			} else {
				// Won't be able to match anything, so just exit now
				common->Printf( "Couldn't parse gamepad/joystick ID: %s\n", controllers[i].id );
				break;
			}

			token = strtok( NULL, "-" );
			if ( token ) {
				productId = strtol( token, NULL, 16 );
			}

			for ( int k = 0; ; k++ ) {
				if ( !knownJoysticks[k].vendor ) {
					break;
				}

				if ( knownJoysticks[k].vid == vendorId && knownJoysticks[k].did == productId ) {
					controllers[i].infoIndex = k;
					break;
				}
			}

			if ( controllers[i].infoIndex == -1 ) {
				common->Printf( "Unknown gamepad/joystick ID: %s\n", controllers[i].id );
			}
			break;
		} else if ( controllers[i].handle == device ) {
			if ( !attached ) {
				// disconnect controller
				memset( &controllers[i], 0, sizeof( controllerState_t ) );
			}
			break;
		}
	}
}

/*
========================
idJoystickQnx::IsKnownDevice
========================
*/
int idJoystickQnx::IsKnownDevice( int inputDeviceNum ) {
	if ( inputDeviceNum >= 0 && inputDeviceNum < MAX_JOYSTICKS && controllers[inputDeviceNum].handle != NULL ) {
		return controllers[inputDeviceNum].infoIndex;
	}
	return -1;
}

/*
========================
idJoystickQnx::IsKnownDevice
========================
*/
int idJoystickQnx::HasTriggers( int inputDeviceNum ) {
	if ( inputDeviceNum >= 0 && inputDeviceNum < MAX_JOYSTICKS && controllers[inputDeviceNum].handle != NULL ) {
		if ( controllers[inputDeviceNum].infoIndex >= 0 ) { // Only want known devices to be returned with triggers
			return knownJoysticks[ controllers[inputDeviceNum].infoIndex ].hasAnalogTriggers;
		}
	}
	return -1;
}

/*
========================
idJoystickQnx::SetRumble
========================
*/
void idJoystickQnx::SetRumble( int inputDeviceNum, int rumbleLow, int rumbleHigh ) {
	if ( inputDeviceNum < 0 || inputDeviceNum >= MAX_JOYSTICKS ) {
		return;
	}
	if ( controllers[inputDeviceNum].handle == NULL ) {
		return;
	}
	//TODO: there is no way to set rumble right now on QNX
}

/*
========================
idJoystickQnx::PostInputEvent

From idJoystickWin32
========================
*/
void idJoystickQnx::PostInputEvent( int inputDeviceNum, int event, int value, int range ) {
	// These events are used for GUI button presses
	if ( ( event >= J_ACTION1 ) && ( event <= J_ACTION_MAX ) ) {
		PushButton( inputDeviceNum, K_JOY1 + ( event - J_ACTION1 ), value != 0 );
	} else if ( event == J_AXIS_LEFT_X ) {
		PushButton( inputDeviceNum, K_JOY_STICK1_LEFT, ( value < -range ) );
		PushButton( inputDeviceNum, K_JOY_STICK1_RIGHT, ( value > range ) );
	} else if ( event == J_AXIS_LEFT_Y ) {
		PushButton( inputDeviceNum, K_JOY_STICK1_UP, ( value < -range ) );
		PushButton( inputDeviceNum, K_JOY_STICK1_DOWN, ( value > range ) );
	} else if ( event == J_AXIS_RIGHT_X ) {
		PushButton( inputDeviceNum, K_JOY_STICK2_LEFT, ( value < -range ) );
		PushButton( inputDeviceNum, K_JOY_STICK2_RIGHT, ( value > range ) );
	} else if ( event == J_AXIS_RIGHT_Y ) {
		PushButton( inputDeviceNum, K_JOY_STICK2_UP, ( value < -range ) );
		PushButton( inputDeviceNum, K_JOY_STICK2_DOWN, ( value > range ) );
	} else if ( ( event >= J_DPAD_UP ) && ( event <= J_DPAD_RIGHT ) ) {
		PushButton( inputDeviceNum, K_JOY_DPAD_UP + ( event - J_DPAD_UP ), value != 0 );
	} else if ( event == J_AXIS_LEFT_TRIG ) {
		PushButton( inputDeviceNum, K_JOY_TRIGGER1, ( value > range ) );
	} else if ( event == J_AXIS_RIGHT_TRIG ) {
		PushButton( inputDeviceNum, K_JOY_TRIGGER2, ( value > range ) );
	}
	if ( event >= J_AXIS_MIN && event <= J_AXIS_MAX ) {
		int axis = event - J_AXIS_MIN;
		// not sure why "* 16" is used. If value is 256 (max) and range is 128 (default), doing this will result in max 32 %. Same goes for Win32 version
		int percent = ( value * 16 ) / range;
		if ( joyAxis[inputDeviceNum][axis] != percent ) {
			joyAxis[inputDeviceNum][axis] = percent;
			Sys_QueEvent( SE_JOYSTICK, axis, percent, 0, NULL, inputDeviceNum );
		}
		// UsercmdGen uses this value but it needs to be scaled to -32768 - 32767, of which it is currently -256 - 255. Multiple by 128 to scale
		value *= 128;
	}

	// These events are used for actual game input
	events[numEvents].event = event;
	events[numEvents].value = value;
	numEvents++;
}

/*
========================
idJoystickQnx::PollInputEvents
========================
*/
int idJoystickQnx::PollInputEvents( int inputDeviceNum ) {
	numEvents = 0;

	if ( qnx.windowState != NAVIGATOR_WINDOW_FULLSCREEN ) {
		return numEvents;
	}

	assert( inputDeviceNum < MAX_JOYSTICKS );

	controllerState_t *cs = &controllers[ inputDeviceNum ];

	if ( cs->handle == NULL ) {
		return numEvents;
	}

	// get current and prior state
	int currentState[7]; //buttonBits, analog0(x, y, trig), analog1(x, y, trig)
	int priorState[7];
	screen_get_device_property_iv( cs->handle, SCREEN_PROPERTY_BUTTONS, &cs->buttonBits );
	if ( cs->analogCount > 0 ) {
		screen_get_device_property_iv( cs->handle, SCREEN_PROPERTY_ANALOG0, cs->analog0 );
	}
	if ( cs->analogCount > 1 ) {
		screen_get_device_property_iv( cs->handle, SCREEN_PROPERTY_ANALOG1, cs->analog1 );
	}
	memcpy( currentState, &cs->buttonBits, sizeof( currentState ) );
	memcpy( priorState, &cs->pButtonBits, sizeof( priorState ) );
	memcpy( &cs->pButtonBits, &cs->buttonBits, sizeof( currentState ) );

	if ( session->IsSystemUIShowing() ) {
		// memset currentState so the current input does not get latched if the UI is showing (XXX may just be a Windows thing, which is where this code is from)
		memset( &currentState, 0, sizeof( currentState ) );
	}

	int joyRemap[20] = {
		J_ACTION1,		J_ACTION2,	J_ACTION11,	// A, B, C (unused)
		J_ACTION3,		J_ACTION4,	J_ACTION12,	// X, Y, Z (unused)
		J_ACTION10,		J_ACTION9,				// Menu 1 (Back), Menu 2 (Start)
		J_ACTION13,		J_ACTION14,				// Menu 3 (unused), Menu 4 (unused)
		J_ACTION5,		J_ACTION15,	J_ACTION7,	// L1 (Left Shoulder), L2 (unused), L3 (Left Stick Down)
		J_ACTION6,		J_ACTION16,	J_ACTION8,	// R1 (Right Shoulder), R2 (unused), R3 (Right Stick Down)
		J_DPAD_UP,		J_DPAD_DOWN,			// Up, Down
		J_DPAD_LEFT,	J_DPAD_RIGHT			// Left, Right
	};

	// Check the digital buttons
	for ( int i = 0; i < 20; i++ ) {
		int mask = ( 1 << i );
		if ( ( currentState[0] & mask ) != ( priorState[0] & mask ) ) {
			PostInputEvent( inputDeviceNum, joyRemap[i], ( currentState[0] & mask ) > 0 );
		}
	}

	// Check the triggers
	if ( cs->analogCount > 0 && currentState[3] != priorState[3] ) {
		PostInputEvent( inputDeviceNum, J_AXIS_LEFT_TRIG, currentState[3] );
	}
	if ( cs->analogCount > 1 && currentState[6] != priorState[6] ) {
		PostInputEvent( inputDeviceNum, J_AXIS_RIGHT_TRIG, currentState[6] );
	}

	if ( cs->analogCount > 0 ) {
		// scale values by 2 so that range is applied correctly
		currentState[1] *= 2;
		currentState[2] *= 2;
		priorState[1] *= 2;
		priorState[2] *= 2;
		if ( currentState[1] != priorState[1] ) {
			PostInputEvent( inputDeviceNum, J_AXIS_LEFT_X, currentState[1] );
		}
		if ( currentState[2] != priorState[2] ) {
			PostInputEvent( inputDeviceNum, J_AXIS_LEFT_Y, currentState[2] );
		}
	}
	if ( cs->analogCount > 1 ) {
		currentState[4] *= 2;
		currentState[5] *= 2;
		priorState[4] *= 2;
		priorState[5] *= 2;
		if ( currentState[4] != priorState[4] ) {
			PostInputEvent( inputDeviceNum, J_AXIS_RIGHT_X, currentState[4] );
		}
		if ( currentState[5] != priorState[5] ) {
			PostInputEvent( inputDeviceNum, J_AXIS_RIGHT_Y, currentState[5] );
		}
	}

	return numEvents;
}

/*
========================
idJoystickQnx::ReturnInputEvent

From idJoystickWin32
========================
*/
int idJoystickQnx::ReturnInputEvent( const int n, int & action, int &value ) {
	if ( ( n < 0 ) || ( n >= MAX_JOY_EVENT ) ) {
		return 0;
	}

	action = events[ n ].event;
	value = events[ n ].value;

	return 1;
}

/*
========================
idJoystickQnx::PushButton

From idJoystickWin32
========================
*/
void idJoystickQnx::PushButton( int inputDeviceNum, int key, bool value ) {
	// So we don't keep sending the same SE_KEY message over and over again
	if ( buttonStates[inputDeviceNum][key] != value ) {
		buttonStates[inputDeviceNum][key] = value;
		Sys_QueEvent( SE_KEY, key, value, 0, NULL, inputDeviceNum );
	}
}
