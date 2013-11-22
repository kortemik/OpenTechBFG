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

#include <sys/keycodes.h>
#include <input/screen_helpers.h>
#include <bps/bps.h>
#include <bps/event.h>
#include <bps/screen.h>
#include <bps/virtualkeyboard.h>

#include <signal.h>

#include "qnx_local.h"

/*
========================================================================

EVENT LOOP

========================================================================
*/

#define	MAX_QUED_EVENTS		256
#define	MASK_QUED_EVENTS	( MAX_QUED_EVENTS - 1 )

sysEvent_t	eventQue[MAX_QUED_EVENTS];
int			eventHead = 0;
int			eventTail = 0;

/*
================
Sys_QueEvent

Ptr should either be null, or point to a block of data that can
be freed by the game later.
================
*/
void Sys_QueEvent( sysEventType_t type, int value, int value2, int ptrLength, void *ptr, int inputDeviceNum ) {
	sysEvent_t * ev = &eventQue[ eventHead & MASK_QUED_EVENTS ];

	if ( eventHead - eventTail >= MAX_QUED_EVENTS ) {
		common->Printf("Sys_QueEvent: overflow\n");
		// we are discarding an event, but don't leak memory
		if ( ev->evPtr ) {
			Mem_Free( ev->evPtr );
		}
		eventTail++;
	}

	eventHead++;

	ev->evType = type;
	ev->evValue = value;
	ev->evValue2 = value2;
	ev->evPtrLength = ptrLength;
	ev->evPtr = ptr;
	ev->inputDevice = inputDeviceNum;
}

/*
=============
Sys_PumpEvents
=============
*/
void Sys_QuitDelay( int value ) {
	navigator_extend_terminate();
	alarm( 1 );
}

/*
=============
Sys_TranslateKey

Not the most ideal way to do it, but keyNum_t isn't "in" an order
=============
*/
int Sys_TranslateKey( int key ) {
	switch( key ) {
	//TODO
	}
	return K_NONE;
}

/*
=============
Sys_TranslateKeyToUnicode
=============
*/
int Sys_TranslateKeyToUnicode( int key ) {
	//TODO
	return '\0';
}

/*
=============
Sys_PumpEvents
=============
*/
#define WHEEL_DELTA 120

void Sys_PumpEvents() {
	bps_event_t* event = NULL;
	int ret;
	int domain;
	int code;

	while( ( ret = bps_get_event(&event, 0) ) == BPS_SUCCESS ) {
		if ( event ) {
			domain = bps_event_get_domain(event);

			if ( domain == screen_get_domain() ) {
				screen_event_t screenEvent = screen_event_get_event( event );
				screen_get_event_property_iv( screenEvent, SCREEN_PROPERTY_TYPE, &code );
				switch ( code ) {
				case SCREEN_EVENT_POINTER: {
					int buttons;
					int wheel;
					int position[2];

					bool move = true;
					bool left_move = false;

					screen_get_event_property_iv(screenEvent, SCREEN_PROPERTY_BUTTONS, &buttons);
					screen_get_event_property_iv(screenEvent, SCREEN_PROPERTY_SOURCE_POSITION, position);
					screen_get_event_property_iv(screenEvent, SCREEN_PROPERTY_MOUSE_WHEEL, &wheel);

					// Left mouse button
					if ( buttons & SCREEN_LEFT_MOUSE_BUTTON ) {
						if ( qnx.mouseButtonsPressed & SCREEN_LEFT_MOUSE_BUTTON ) {
							left_move = true;
						} else {
							move = false;
							qnx.mouseButtonsPressed |= SCREEN_LEFT_MOUSE_BUTTON;
							Sys_QueEvent( SE_KEY, K_MOUSE1, 1, 0, NULL, 0 );
						}
					} else if ( qnx.mouseButtonsPressed & SCREEN_LEFT_MOUSE_BUTTON ) {
						move = false;
						qnx.mouseButtonsPressed &= ~SCREEN_LEFT_MOUSE_BUTTON;
						Sys_QueEvent( SE_KEY, K_MOUSE1, 0, 0, NULL, 0 );
					}

					// Right mouse button
					if ( buttons & SCREEN_RIGHT_MOUSE_BUTTON ) {
						if ( !( qnx.mouseButtonsPressed & SCREEN_RIGHT_MOUSE_BUTTON ) ) {
							move = false;
							qnx.mouseButtonsPressed |= SCREEN_RIGHT_MOUSE_BUTTON;
							Sys_QueEvent( SE_KEY, K_MOUSE2, 1, 0, NULL, 0 );
						}
					} else if ( qnx.mouseButtonsPressed & SCREEN_RIGHT_MOUSE_BUTTON ) {
						move = false;
						qnx.mouseButtonsPressed &= ~SCREEN_RIGHT_MOUSE_BUTTON;
						Sys_QueEvent( SE_KEY, K_MOUSE2, 0, 0, NULL, 0 );
					}

					// Middle mouse button
					if ( buttons & SCREEN_MIDDLE_MOUSE_BUTTON ) {
						if ( !( qnx.mouseButtonsPressed & SCREEN_MIDDLE_MOUSE_BUTTON ) ) {
							move = false;
							qnx.mouseButtonsPressed |= SCREEN_MIDDLE_MOUSE_BUTTON;
							Sys_QueEvent( SE_KEY, K_MOUSE3, 1, 0, NULL, 0 );
						}
					} else if ( qnx.mouseButtonsPressed & SCREEN_MIDDLE_MOUSE_BUTTON ) {
						move = false;
						qnx.mouseButtonsPressed &= ~SCREEN_MIDDLE_MOUSE_BUTTON;
						Sys_QueEvent( SE_KEY, K_MOUSE3, 0, 0, NULL, 0 );
					}

					// Mouse position
					if ( left_move || move ) {
						Sys_QueEvent( SE_MOUSE_ABSOLUTE, position[0], position[1], 0, NULL, 0 );
					}

					// Mouse wheel
					int delta = ( qnx.mouseWheelPosition - wheel ) / WHEEL_DELTA;
					qnx.mouseWheelPosition -= wheel;
					int key = delta < 0 ? K_MWHEELDOWN : K_MWHEELUP;
					delta = abs( delta );
					while( delta-- > 0 ) {
						Sys_QueEvent( SE_KEY, key, true, 0, NULL, 0 );
						Sys_QueEvent( SE_KEY, key, false, 0, NULL, 0 );
					}
					break;
				}

				case SCREEN_EVENT_KEYBOARD: {
					int flags;
					int value;

					screen_get_event_property_iv(screenEvent, SCREEN_PROPERTY_KEY_FLAGS, &flags);
					screen_get_event_property_iv(screenEvent, SCREEN_PROPERTY_KEY_SYM, &value);

					if ( !( flags & KEY_REPEAT ) ) {
						int key = Sys_TranslateKey( value );
						if ( flags & KEY_DOWN ) {
							//XXX Is this needed or is it just a windows thing? Without separate keyboard, there is no keyboard or pause keys
							if ( key == K_NUMLOCK ) {
								key = K_PAUSE;
							} else if ( key == K_PAUSE ) {
								key = K_NUMLOCK;
							}
						}
						Sys_QueEvent( SE_KEY, key, ( flags & KEY_DOWN ), 0, NULL, 0 );
						if ( flags & ( KEY_SYM_VALID | KEY_DOWN ) ) {
							key = Sys_TranslateKeyToUnicode( value );
							if ( key ) {
								Sys_QueEvent( SE_CHAR, key, 0, 0, NULL, 0 );
							}
						}
					}
					break;
				}

				//TODO: gamepad/joystick (connect and disconnect)
				}
			} else if ( domain == navigator_get_domain() ) {
				code = bps_event_get_code( event );
				switch( code ) {
				case NAVIGATOR_EXIT:
					// Everything must stop within 3 seconds, otherwise the navigator will force-close the program.
					// To prevent this, we setup a timer to delay shutdown until execution loop finishes.
					// Shutdown timer will take 3 seconds before killing process, but extension only extends it by 2
					// seconds. To prevent never closing, there is a limit of 15 delays or 30 seconds before the app
					// will be force-closed.

					struct sigaction act;
					sigset_t set;

					if ( !qnx.quitStarted ) {
						qnx.quitStarted = true;

						sigemptyset( &set );
						sigaddset( &set, SIGALRM );

						act.sa_flags = 0;
						act.sa_mask = set;
						act.sa_handler = &Sys_QuitDelay;

						soundSystem->SetMute( true );
						cmdSystem->BufferCommandText( CMD_EXEC_APPEND, "quit\n" );
						if ( sigaction( SIGALRM, &act, NULL ) != 0 ) {
							cmdSystem->ExecuteCommandBuffer();
						} else {
							alarm( 2 );
						}
					}
					break;

				case NAVIGATOR_SWIPE_DOWN:
					// Acts like pressing the escape key, causing the menu to show up
					Sys_QueEvent( SE_KEY, K_ESCAPE, true, 0, NULL, 0 );
					Sys_QueEvent( SE_KEY, K_ESCAPE, false, 0, NULL, 0 );
					break;

				case NAVIGATOR_WINDOW_STATE: {
					qnx.windowState = navigator_event_get_window_state( event );

					// Pause the game (fake-press the escape key) if minimized
					if ( qnx.windowState != NAVIGATOR_WINDOW_FULLSCREEN && game && game->IsInGame() && !game->Shell_IsActive() ) {
						Sys_QueEvent( SE_KEY, K_ESCAPE, true, 0, NULL, 0 );
						Sys_QueEvent( SE_KEY, K_ESCAPE, false, 0, NULL, 0 );
					}

					soundSystem->SetMute( qnx.windowState != NAVIGATOR_WINDOW_FULLSCREEN );
					break;
				}

				//TODO
				//NAVIGATOR_ORIENTATION_CHECK
				//NAVIGATOR_ORIENTATION

				case NAVIGATOR_BACK:
					// Acts like pressing ~ to open in-game console //XXX This should be disabled if key-bindings is in use
					Sys_QueEvent( SE_KEY, K_GRAVE, true, 0, NULL, 0 );
					Sys_QueEvent( SE_KEY, K_GRAVE, false, 0, NULL, 0 );
					break;

				//TODO
				//NAVIGATOR_ORIENTATION_DONE (similar to WM_SIZING)
				//NAVIGATOR_INVOKE_QUERY_RESULT
				//NAVIGATOR_INVOKE_TARGET_RESULT
				//NAVIGATOR_INVOKE_VIEWER_RESULT
				//NAVIGATOR_INVOKE_VIEWER_RELAY
				//NAVIGATOR_INVOKE_VIEWER_STOPPED
				//NAVIGATOR_KEYBOARD_STATE
				//NAVIGATOR_KEYBOARD_POSITION
				//NAVIGATOR_INVOKE_VIEWER_RELAY_RESULT
				//NAVIGATOR_WINDOW_COVER_ENTER
				//NAVIGATOR_WINDOW_COVER_EXIT
				//NAVIGATOR_CARD_PEEK_STARTED
				//NAVIGATOR_CARD_PEEK_STOPPED
				//NAVIGATOR_ORIENTATION_SIZE
				}
			}
		} else {
			break;
		}
	}
	if ( ret != BPS_SUCCESS ) {
		common->Warning( "Sys_PumpEvents: couldn't get event, bps_get_event event failed\n" );
	}
}

/*
================
Sys_GenerateEvents
================
*/
void Sys_GenerateEvents() {
	static int entered = false;

	if ( entered ) {
		return;
	}
	entered = true;

	// pump the message loop
	Sys_PumpEvents();

	entered = false;
}

/*
================
Sys_ClearEvents
================
*/
void Sys_ClearEvents() {
	eventHead = eventTail = 0;
}

/*
================
Sys_GetEvent
================
*/
sysEvent_t Sys_GetEvent() {
	sysEvent_t	ev;

	// return if we have data
	if ( eventHead > eventTail ) {
		eventTail++;
		return eventQue[ ( eventTail - 1 ) & MASK_QUED_EVENTS ];
	}

	// return the empty event
	memset( &ev, 0, sizeof( ev ) );

	return ev;
}

//================================================================
