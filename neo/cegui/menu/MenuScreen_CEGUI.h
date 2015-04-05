/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.
Copyright (C) 2014 Robert Beckebans

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
#ifndef __MENUSCREENCEGUI_H__
#define __MENUSCREENCEGUI_H__

#include "../d3xp/menus/MenuScreen_Interface.h"   // for idMenuScreen_HUD_Interface
#include "../d3xp/menus/MenuState.h"

//*
//================================================
//idMenuScreen_HUD
//================================================
//*/
class idMenuScreen_HUD : public idMenuScreen_HUD_Interface
{
public:
	idMenuScreen_HUD();
	~idMenuScreen_HUD();
	
	virtual void UpdateHealthArmor( idPlayer* player );
	virtual void UpdateStamina( idPlayer* player );
	virtual void UpdateLocation( idPlayer* player );
	virtual void UpdateWeaponInfo( idPlayer* player );
	virtual void UpdateWeaponStates( idPlayer* player, bool weaponChanged );
	
	virtual void DownloadVideo();
	virtual void DownloadPDA( const idDeclPDA* pda, bool newSecurity );
	virtual void UpdatedSecurity();
	
	virtual void ClearNewPDAInfo();
	
	virtual void UpdateOxygen( bool show, int val = 0 );
	virtual void SetupObjective( const idStr& title, const idStr& desc, const idMaterial* screenshot );
	virtual void SetupObjectiveComplete( const idStr& title );
	virtual void ShowObjective( bool complete );
	virtual void HideObjective( bool complete );
	virtual void GiveWeapon( idPlayer* player, int weaponIndex );
	virtual void UpdatePickupInfo( int index, const idStr& name );
	bool IsPickupListReady();
	virtual void ShowPickups();
	virtual void SetCursorState( idPlayer* player, cursorState_t state, int set );
	virtual void SetCursorText( const idStr& action, const idStr& focus );
	virtual void UpdateCursorState();
	virtual void CombatCursorFlash();
	virtual void UpdateSoulCube( bool ready );
	virtual void ShowRespawnMessage( bool show );
	virtual void SetShowSoulCubeOnLoad( bool show );
	
	// MULTIPLAYER
	
	virtual void ToggleMPInfo( bool show, bool showTeams, bool isCTF = false );
	virtual void SetFlagState( int team, int state );
	virtual void SetTeamScore( int team, int score );
	virtual void SetTeam( int team );
	virtual void TriggerHitTarget( bool show, const idStr& target, int color = 0 );
	virtual void ToggleLagged( bool show );
	virtual void UpdateGameTime( const char* time );
	virtual void UpdateMessage( bool show, const idStr& message );
	virtual void ShowNewItem( const char* name, const char* icon );
	virtual void UpdateFlashlight( idPlayer* player );
	virtual void UpdateChattingHud( idPlayer* player );
};

#endif // __MENUSCREENCEGUI_H__
