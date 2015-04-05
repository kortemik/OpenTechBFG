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
#ifndef __MENUHANDLERSWF_H__
#define __MENUHANDLERSWF_H__

#include "../d3xp/menus/MenuWidget.h"   // for idMenuWidget
#include "../swf/SWF_ScriptVar.h"       // for idSWFScriptVar
#include "../swf/SWF_ParmList.h"        // for idSWFParmList
#include "../swf/SWF_ScriptObject.h"    // for idSWFScriptObject
#include "../swf/SWF_SpriteInstance.h"  // for idSWFSpriteInstance
#include "../framework/DeclPDA.h"       // for idDeclAudio

#include "../d3xp/menus/MenuHandler_Interface.h" // for Interfaces
#include "../d3xp/menus/MenuState.h"

#include "../d3xp/menus/MenuScreen_SWF.h"   // for idMenuScreen, etc

#include "../d3xp/menus/mpScoreboardInfo.h"

enum pdaAreas_t
{
	PDA_AREA_INVALID = -1,
	PDA_AREA_USER_DATA,
	PDA_AREA_USER_EMAIL,
	PDA_AREA_VIDEO_DISKS,
	PDA_AREA_INVENTORY,
	PDA_NUM_AREAS
};

enum hudArea_t
{
	HUD_AREA_INVALID = -1,
	HUD_AREA_PLAYING,
	HUD_NUM_AREAS
};

enum pdaHandlerWidgets_t
{
	PDA_WIDGET_NAV_BAR,
	PDA_WIDGET_PDA_LIST,
	PDA_WIDGET_PDA_LIST_SCROLLBAR,
	PDA_WIDGET_CMD_BAR
};

enum scoreboardHandlerWidgets_t
{
	SCOREBOARD_WIDGET_CMD_BAR,
};

enum menuSounds_t
{
	GUI_SOUND_MUSIC,
	GUI_SOUND_SCROLL,
	GUI_SOUND_ADVANCE,
	GUI_SOUND_BACK,
	GUI_SOUND_BUILD_ON,
	GUI_SOUND_BUILD_OFF,
	GUI_SOUND_FOCUS,
	GUI_SOUND_ROLL_OVER,
	GUI_SOUND_ROLL_OUT,
	NUM_GUI_SOUNDS,
};

static const int MAX_SCREEN_AREAS = 32;
static const int DEFAULT_REPEAT_TIME = 150;
static const int WAIT_START_TIME_LONG = 30000;
static const int WAIT_START_TIME_SHORT = 5000;

struct actionRepeater_t
{
	actionRepeater_t() :
		widget( NULL ),
		numRepetitions( 0 ),
		nextRepeatTime( 0 ),
		repeatDelay( DEFAULT_REPEAT_TIME ),
		screenIndex( -1 ),
		isActive( false )
	{
	}
	
	idMenuWidget* 		widget;
	idWidgetEvent		event;
	idWidgetAction		action;
	int					numRepetitions;
	int					nextRepeatTime;
	int					repeatDelay;
	int					screenIndex;
	bool				isActive;
};


/*
================================================
idMenuHandler
================================================
*/
class idMenuHandler : public idMenuHandler_Interface
{
public:
	idMenuHandler();
	virtual					~idMenuHandler();
	
	virtual void			Initialize( const char* swfFile, idSoundWorld* sw );
	virtual void			Cleanup();
	virtual void			Update();
	virtual void			UpdateChildren();
	virtual void			UpdateMenuDisplay( int menu );
	virtual bool			HandleGuiEvent( const sysEvent_t* sev );
	virtual bool			IsActive();
	virtual void			ActivateMenu( bool show );
	virtual void			TriggerMenu();
	virtual bool			HandleAction( idWidgetAction& action, const idWidgetEvent& event, idMenuWidget* widget, bool forceHandled = false );
	virtual int				ActiveScreen()
	{
		return activeScreen;
	}
	virtual int				NextScreen()
	{
		return nextScreen;
	}
	virtual int				MenuTransition()
	{
		return transition;
	}
	virtual idMenuScreen* 	GetMenuScreen( int index )
	{
		return NULL;
	}
	virtual void			SetNextScreen( int screen, int trans )
	{
		nextScreen = screen;
		transition = trans;
	}
	
	virtual void			StartWidgetActionRepeater( idMenuWidget* widget, const idWidgetAction& action, const idWidgetEvent& event );
	virtual void			PumpWidgetActionRepeater();
	virtual void			ClearWidgetActionRepeater();
	virtual idSWF* 			GetGUI()
	{
		return gui;
	}
	virtual void			AddChild( idMenuWidget* widget );
	virtual idMenuWidget* 	GetChildFromIndex( int index );
	virtual int				GetPlatform( bool realPlatform = false );
	virtual void			PlaySound( menuSounds_t type, int channel = -1 );
	virtual void			StopSound( int channel = SCHANNEL_ANY );
	
	idMenuWidget_CommandBar* 	GetCmdBar()
	{
		return cmdBar;
	}
	
protected:

	bool						scrollingMenu;
	int							scrollCounter;
	int							activeScreen;
	int							nextScreen;
	int							transition;
	int							platform;
	idSWF* 						gui;
	actionRepeater_t			actionRepeater;
	idMenuScreen* 				menuScreens[MAX_SCREEN_AREAS];
	idList< idMenuWidget*, TAG_IDLIB_LIST_MENU>	children;
	
	idStaticList< idStr, NUM_GUI_SOUNDS >		sounds;
	
	idMenuWidget_CommandBar* 	cmdBar;
};

/*
================================================
lobbyPlayerInfo_t
================================================
*/
struct lobbyPlayerInfo_t
{
	lobbyPlayerInfo_t() :
		partyToken( 0 ),
		voiceState( VOICECHAT_DISPLAY_NONE )
	{
	}
	
	idStr					name;
	int						partyToken;
	voiceStateDisplay_t		voiceState;
};

/*
================================================
idMenuHandler_Shell
================================================
*/
class idMenuHandler_Shell : public idMenuHandler_Shell_Interface, public idMenuHandler
{
public:
	idMenuHandler_Shell() :
		state( SHELL_STATE_INVALID ),
		nextState( SHELL_STATE_INVALID ),
		smallFrameShowing( false ),
		largeFrameShowing( false ),
		bgShowing( true ),
		waitForBinding( false ),
		waitBind( NULL ),
		menuBar( NULL ),
		pacifier( NULL ),
		timeRemaining( 0 ),
		nextPeerUpdateMs( 0 ),
		newGameType( 0 ),
		inGame( false ),
		showingIntro( false ),
		continueWaitForEnumerate( false ),
		gameComplete( false ),
		introGui( NULL ),
		typeSoundShader( NULL ),
		doom3Intro( NULL ),
		roeIntro( NULL ),
		lmIntro( NULL ),
		marsRotation( NULL )
	{
	}
	
	virtual void 			ClearWidgetActionRepeater()
	{
		idMenuHandler::ClearWidgetActionRepeater();
	}
	virtual void			SetNextScreen( int screen, int trans )
	{
		idMenuHandler::SetNextScreen( screen, trans );
	}
	
	virtual void			Update();
	virtual void			ActivateMenu( bool show );
	virtual void			Initialize( const char* swfFile, idSoundWorld* sw );
	virtual void			Cleanup();
	virtual bool			HandleAction( idWidgetAction& action, const idWidgetEvent& event, idMenuWidget* widget, bool forceHandled = false );
	virtual idMenuScreen* 	GetMenuScreen( int index );
	virtual bool			HandleGuiEvent( const sysEvent_t* sev );
	virtual bool			IsActive()
	{
		return idMenuHandler::IsActive();
	}
	
	void					UpdateSavedGames();
	void					ShowSmallFrame( bool show );
	void					ShowMPFrame( bool show );
	void					ShowLogo( bool show );
	void					SetShellState( shellState_t s )
	{
		nextState = s;
	}
	bool					IsSmallFrameShowing()
	{
		return smallFrameShowing;
	}
	void					UpdateBGState();
	void					GetMapName( int index, idStr& name );
	void					GetModeName( int index, idStr& name );
	
	idMenuWidget* 			GetPacifier()
	{
		return pacifier;
	}
	idMenuWidget_MenuBar* 	GetMenuBar()
	{
		return menuBar;
	}
	bool					IsPacifierVisible() const
	{
		return ( pacifier != NULL && pacifier->GetSprite() != NULL ) ? pacifier->GetSprite()->IsVisible() : false;
	}
	void					ShowPacifier( const idStr& msg );
	void					HidePacifier();
	
	void					SetTimeRemaining( int time )
	{
		timeRemaining = time;
	}
	int						GetTimeRemaining()
	{
		return timeRemaining;
	}
	void					SetNewGameType( int type )
	{
		newGameType = type;
	}
	int						GetNewGameType()
	{
		return newGameType;
	}
	void					SetInGame( bool val )
	{
		inGame = val;
	}
	bool					GetInGame()
	{
		return inGame;
	}
	void					HandleExitGameBtn();
	void					SetupPCOptions();
	void					SetWaitForBinding( const char* bind )
	{
		waitForBinding = true;
		waitBind = bind;
	}
	void					ClearWaitForBinding()
	{
		waitForBinding = false;
	}
	void					UpdateLeaderboard( const idLeaderboardCallback* callback );
	void					UpdateLobby( idMenuWidget_LobbyList* lobbyList );
	void					ShowDoomIntro();
	void					ShowROEIntro();
	void					ShowLEIntro();
	void					StartGame( int index );
	void					SetContinueWaitForEnumerate( bool wait )
	{
		continueWaitForEnumerate = wait;
	}
	void					SetCanContinue( bool valid );
	void					SetGameComplete()
	{
		gameComplete = true;
	}
	bool					GetGameComplete()
	{
		return gameComplete;
	}
	
private:

	shellState_t			state;
	shellState_t			nextState;
	bool					smallFrameShowing;
	bool					largeFrameShowing;
	bool					bgShowing;
	bool					waitForBinding;
	const char* 			waitBind;
	//idSysSignal				deviceRequestedSignal;
	
	idList<const char*, TAG_IDLIB_LIST_MENU>	mpGameModes;
	idList<mpMap_t, TAG_IDLIB_LIST_MENU>			mpGameMaps;
	idMenuWidget_MenuBar* 	menuBar;
	idMenuWidget* 			pacifier;
	int						timeRemaining;
	int						nextPeerUpdateMs;
	int						newGameType;
	bool					inGame;
	bool					showingIntro;
	bool					continueWaitForEnumerate;
	bool					gameComplete;
	idSWF* 					introGui;
	const idSoundShader* 	typeSoundShader;
	const idMaterial* 		doom3Intro;
	const idMaterial* 		roeIntro;
	const idMaterial* 		lmIntro;
	const idMaterial* 		marsRotation;
	idList< idStr, TAG_IDLIB_LIST_MENU>			navOptions;
	
};

/*
================================================
idMenuHandler_PDA
================================================
*/
class idMenuHandler_PDA : public idMenuHandler_PDA_Interface, public idMenuHandler
{
public:
	idMenuHandler_PDA() :
		audioLogPlaying( false ),
		videoPlaying( false ),
		audioFile( NULL )
	{
	}
	virtual ~idMenuHandler_PDA();
	
	virtual void 			ClearWidgetActionRepeater()
	{
		idMenuHandler::ClearWidgetActionRepeater();
	}
	virtual bool			HandleGuiEvent( const sysEvent_t* sev )
	{
		return idMenuHandler::HandleGuiEvent( sev );
	}
	
	virtual void			Initialize( const char* swfFile, idSoundWorld* sw );
	virtual void			Update();
	virtual void			ActivateMenu( bool show );
	virtual void			TriggerMenu();
	
	virtual bool			HandleAction( idWidgetAction& action, const idWidgetEvent& event, idMenuWidget* widget, bool forceHandled = false );
	virtual idMenuScreen* 	GetMenuScreen( int index );
	virtual bool			IsActive()
	{
		return idMenuHandler::IsActive();
	}
	
	void					UpdateAudioLogPlaying( bool playing );
	void					UdpateVideoPlaying( bool playing );
	void					ClearVideoPlaying()
	{
		videoPlaying = false;
	}
	
	bool					PlayPDAAudioLog( int pdaIndex, int audioIndex );
	virtual void			Cleanup();
	
protected:
	bool							audioLogPlaying;
	bool							videoPlaying;
	idList< idList< idStr, TAG_IDLIB_LIST_MENU >, TAG_IDLIB_LIST_MENU >		pdaNames;
	idList< idStr, TAG_IDLIB_LIST_MENU >					navOptions;
	const idDeclAudio* 				audioFile;
	idMenuWidget_ScrollBar pdaScrollBar;
	idMenuWidget_DynamicList pdaList;
	idMenuWidget_NavBar navBar;
	idMenuWidget_CommandBar commandBarWidget;
};

/*
================================================
idMenuHandler_HUD
================================================
*/
class idMenuHandler_HUD : public idMenuHandler_HUD_Interface, public idMenuHandler
{
public:

	idMenuHandler_HUD() :
		autoHideTip( true ),
		tipStartTime( 0 ),
		hiding( false ),
		radioMessage( false )
	{
	}
	
	virtual void 			ClearWidgetActionRepeater()
	{
		idMenuHandler::ClearWidgetActionRepeater();
	}
	virtual bool			HandleGuiEvent( const sysEvent_t* sev )
	{
		return idMenuHandler::HandleGuiEvent( sev );
	}
	
	
	virtual void			Update();
	virtual void			ActivateMenu( bool show );
	virtual void			Initialize( const char* swfFile, idSoundWorld* sw );
	virtual idMenuScreen* 	GetMenuScreen( int index );
	virtual bool			IsActive()
	{
		return idMenuHandler::IsActive();
	}
	
	idMenuScreen_HUD* 		GetHud();
	void					ShowTip( const char* title, const char* tip, bool autoHide );
	void					HideTip();
	void					SetRadioMessage( bool show )
	{
		radioMessage = show;
	}
	
protected:

	bool	autoHideTip;
	int		tipStartTime;
	bool	hiding;
	bool	radioMessage;
	
};

/*
================================================
idMenuHandler_Scoreboard
================================================
*/
class idMenuHandler_Scoreboard : public ::idMenuHandler_Scoreboard_Interface, public idMenuHandler
{
public:

	idMenuHandler_Scoreboard() :
		redScore( 0 ),
		blueScore( 0 ),
		activationScreen( SCOREBOARD_AREA_INVALID )
	{
	}
	
	virtual void 			ClearWidgetActionRepeater()
	{
		idMenuHandler::ClearWidgetActionRepeater();
	}
	virtual bool			HandleGuiEvent( const sysEvent_t* sev )
	{
		return idMenuHandler::HandleGuiEvent( sev );
	}
	
	virtual void			Update();
	virtual void			TriggerMenu();
	virtual void			ActivateMenu( bool show );
	virtual void			Initialize( const char* swfFile, idSoundWorld* sw );
	virtual idMenuScreen* 	GetMenuScreen( int index );
	virtual bool			HandleAction( idWidgetAction& action, const idWidgetEvent& event, idMenuWidget* widget, bool forceHandled = false );
	virtual bool			IsActive()
	{
		return idMenuHandler::IsActive();
	}
	
	void					AddPlayerInfo( int index, voiceStateDisplay_t voiceState, int team, idStr name, int score, int wins, int ping, idStr spectateData );
	void					UpdateScoreboard( idList< mpScoreboardInfo >& data, idStr gameInfo );
	void					UpdateVoiceStates();
	void					UpdateSpectating( idStr spectate, idStr follow );
	void					SetTeamScores( int r, int b );
	int						GetNumPlayers( int team );
	void					SetActivationScreen( int screen, int trans );
	void					ViewPlayerProfile( int slot );
	void					MutePlayer( int slot );
	void					GetUserID( int slot, lobbyUserID_t& luid );
	void					UpdateScoreboardSelection();
	
protected:

	int		redScore;
	int		blueScore;
	int		activationScreen;
	
	idList< mpScoreboardInfo > scoreboardInfo;
	idList< scoreboardInfo_t, TAG_IDLIB_LIST_MENU >		redInfo;
	idList< scoreboardInfo_t, TAG_IDLIB_LIST_MENU>		blueInfo;
	
};


#endif //__MENUHANDLERSWF_H__
