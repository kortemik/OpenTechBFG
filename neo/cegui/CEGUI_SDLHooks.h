/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.
Copyright (C) 2012 Robert Beckebans

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

#ifndef CEGUI_SDLHOOKS_H_
#define CEGUI_SDLHOOKS_H_

#include <SDL.h>

class cegui {
public:
	static cegui& getInstance() {
		static cegui instance;
		return instance;
	}

	void notifyDisplaySizeChanged(int width, int height);
	bool injectChar (Uint16 unicode_char);
	bool injectKeyDown (SDLKey sym);
	bool injectKeyUp (SDLKey sym);
	bool injectMousePosition(Uint16 x, Uint16 y);
	bool injectMouseButtonLeftDown(void);
	bool injectMouseButtonLeftUp(void);
	bool injectMouseButtonMiddleDown(void);
	bool injectMouseButtonMiddleUp(void);
	bool injectMouseButtonRightDown(void);
	bool injectMouseButtonRightUp(void);
	bool injectMouseWheelChange(float delta);

	bool injectTimePulse (Uint32 ticks);

	void renderAllGUIContexts(void);

private:
	void initializeKeyMap(void);
	void initRenderer(void);
	void initResourceProvider(void);
	void initResourceGroups(void);
	void initResources(void);
	void createWindow(void);

	struct ceguiVars;
	ceguiVars *ourVars;

	cegui();

	cegui(cegui const&);
	void operator=(cegui const&);
};


#endif /* CEGUI_H_ */

