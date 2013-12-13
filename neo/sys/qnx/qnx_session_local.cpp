/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2010 id Software LLC, a ZeniMax Media company. All Rights Reserved.
================================================================================================
*/
//XXX Is ^^ correct? Everything should be GPL, so should that be replaced with the standard GPL license?

/*
================================================================================================

Contains the QNX implementation of the network session

================================================================================================
*/

#pragma hdrstop
#include "../../idlib/precompiled.h"
#include "../../framework/Common_local.h"
#include "../sys_session_local.h"
#include "../sys_stats.h"
#include "../sys_savegame.h"
#include "../sys_lobby_backend_direct.h"
#include "../sys_voicechat.h"
#include "qnx_achievements.h"
#include "qnx_local.h"

/*
========================
Global variables
========================
*/

extern idCVar net_port;


/*
========================
idSessionLocalQnx::idSessionLocalQnx
========================
*/
class idLobbyToSessionCBLocal;
class idSessionLocalQnx : public idSessionLocal {
friend class idLobbyToSessionCBLocal;

public:
	idSessionLocalQnx();
	virtual ~idSessionLocalQnx();

	// idSessionLocal interface
	virtual void		Initialize();
	virtual void		Shutdown();

	virtual void		InitializeSoundRelatedSystems();
	virtual void		ShutdownSoundRelatedSystems();

	virtual void		PlatformPump();

	virtual void		InviteFriends();
	virtual void		InviteParty();
	virtual void		ShowPartySessions();

	virtual void		ShowSystemMarketplaceUI() const;

	virtual void					ListServers( const idCallback & callback );
	virtual void					CancelListServers();
	virtual int						NumServers() const;
	virtual const serverInfo_t *	ServerInfo( int i ) const;
	virtual void					ConnectToServer( int i );
	virtual void					ShowServerGamerCardUI( int i );

	virtual void			ShowLobbyUserGamerCardUI( lobbyUserID_t lobbyUserID );

	virtual void			ShowOnlineSignin() {}
	virtual void			UpdateRichPresence() {}
	virtual void			CheckVoicePrivileges() {}

	virtual bool			ProcessInputEvent( const sysEvent_t * ev );

	// System UI
	virtual bool			IsSystemUIShowing() const;
	virtual void			SetSystemUIShowing( bool show );

	// Invites
	virtual void			HandleBootableInvite( int64 lobbyId = 0 );
	virtual void			ClearBootableInvite();
	virtual void			ClearPendingInvite();

	virtual bool			HasPendingBootableInvite();
	virtual void			SetDiscSwapMPInvite( void * parm );
	virtual void *			GetDiscSwapMPInviteParms();

	virtual void			EnumerateDownloadableContent();

	virtual void 			HandleServerQueryRequest( lobbyAddress_t & remoteAddr, idBitMsg & msg, int msgType );
	virtual void 			HandleServerQueryAck( lobbyAddress_t & remoteAddr, idBitMsg & msg );

	// Leaderboards
	virtual void			LeaderboardUpload( lobbyUserID_t lobbyUserID, const leaderboardDefinition_t * leaderboard, const column_t * stats, const idFile_Memory * attachment = NULL );
	virtual void			LeaderboardDownload( int sessionUserIndex, const leaderboardDefinition_t * leaderboard, int startingRank, int numRows, const idLeaderboardCallback & callback );
	virtual void			LeaderboardDownloadAttachment( int sessionUserIndex, const leaderboardDefinition_t * leaderboard, int64 attachmentID );

	// Scoring (currently just for TrueSkill)
	virtual void			SetLobbyUserRelativeScore( lobbyUserID_t lobbyUserID, int relativeScore, int team ) {}

	virtual void			LeaderboardFlush();

	virtual idNetSessionPort &	GetPort( bool dedicated = false );
	virtual idLobbyBackend *	CreateLobbyBackend( const idMatchParameters & p, float skillLevel, idLobbyBackend::lobbyBackendType_t lobbyType );
	virtual idLobbyBackend *	FindLobbyBackend( const idMatchParameters & p, int numPartyUsers, float skillLevel, idLobbyBackend::lobbyBackendType_t lobbyType );
	virtual idLobbyBackend *	JoinFromConnectInfo( const lobbyConnectInfo_t & connectInfo , idLobbyBackend::lobbyBackendType_t lobbyType );
	virtual void				DestroyLobbyBackend( idLobbyBackend * lobbyBackend );
	virtual void				PumpLobbies();
	virtual void				JoinAfterSwap( void * joinID );

	virtual bool				GetLobbyAddressFromNetAddress( const netadr_t & netAddr, lobbyAddress_t & outAddr ) const;
	virtual bool				GetNetAddressFromLobbyAddress( const lobbyAddress_t & lobbyAddress, netadr_t & outNetAddr ) const;

public:
	void	Connect_f( const idCmdArgs &args );

private:
	void					EnsurePort();

	idLobbyBackend *		CreateLobbyInternal( idLobbyBackend::lobbyBackendType_t lobbyType );

	idArray< idLobbyBackend *, 3 > lobbyBackends;

	idNetSessionPort		port;
	bool					canJoinLocalHost;

	idLobbyToSessionCBLocal	* lobbyToSessionCB;
};

idSessionLocalQnx sessionLocalQnx;
idSession * session = &sessionLocalQnx;

/*
========================
idLobbyToSessionCBLocal
========================
*/
class idLobbyToSessionCBLocal : public idLobbyToSessionCB {
public:
	idLobbyToSessionCBLocal( idSessionLocalQnx * sessionLocalQnx_ ) : sessionLocalQnx( sessionLocalQnx_ ) { }

	virtual bool CanJoinLocalHost() const { sessionLocalQnx->EnsurePort(); return sessionLocalQnx->canJoinLocalHost; }
	virtual class idLobbyBackend * GetLobbyBackend( idLobbyBackend::lobbyBackendType_t type ) const { return sessionLocalQnx->lobbyBackends[ type ]; }

private:
	idSessionLocalQnx *			sessionLocalQnx;
};

idLobbyToSessionCBLocal lobbyToSessionCBLocal( &sessionLocalQnx );
idLobbyToSessionCB * lobbyToSessionCB = &lobbyToSessionCBLocal;

class idVoiceChatMgrQnx : public idVoiceChatMgr {
public:
	virtual bool	GetLocalChatDataInternal( int talkerIndex, byte * data, int & dataSize ) { return false; }
	virtual void	SubmitIncomingChatDataInternal( int talkerIndex, const byte * data, int dataSize ) { }
	virtual bool	TalkerHasData( int talkerIndex ) { return false; }
	virtual bool	RegisterTalkerInternal( int index ) { return true; }
	virtual void	UnregisterTalkerInternal( int index ) { }
};

/*
========================
Sys_LangCodes

Made to match the order of Sys_Lang

This may not be the best place for this, but it's only needed for Scoreloop which is managed here
========================
*/
const char * sysLanguageCodes[] = {
	ID_LANG_CODE_ENGLISH, ID_LANG_CODE_FRENCH, ID_LANG_CODE_ITALIAN, ID_LANG_CODE_GERMAN, ID_LANG_CODE_SPANISH, ID_LANG_CODE_JAPANESE, NULL
};
const char * Sys_LangCodes( int idx ) {
	if ( idx >= 0 && idx < Sys_NumLangs() ) {
		return sysLanguageCodes[ idx ];
	}
	return "";
}

/*
========================
idSessionLocalQnx::idSessionLocalQnx
========================
*/
idSessionLocalQnx::idSessionLocalQnx() {
	signInManager		= new (TAG_SYSTEM) idSignInManagerQnx;
	saveGameManager		= new (TAG_SAVEGAMES) idSaveGameManager();
	voiceChat			= new (TAG_SYSTEM) idVoiceChatMgrQnx();
	lobbyToSessionCB	= new (TAG_SYSTEM) idLobbyToSessionCBLocal( this );

	canJoinLocalHost	= false;

	lobbyBackends.Zero();
}

/*
========================
idSessionLocalQnx::idSessionLocalQnx
========================
*/
idSessionLocalQnx::~idSessionLocalQnx() {
	delete voiceChat;
	delete lobbyToSessionCB;
}

/*
========================
idSessionLocalQnx::Initialize
========================
*/
void idSessionLocalQnx::Initialize() {
	idSessionLocal::Initialize();

	// The shipping path doesn't load title storage
	// Instead, we inject values through code which is protected through steam DRM
	titleStorageVars.Set( "MAX_PLAYERS_ALLOWED", "8" );
	titleStorageLoaded = true;

	// First-time check for downloadable content once game is launched
	EnumerateDownloadableContent();

	GetPartyLobby().Initialize( idLobby::TYPE_PARTY, sessionCallbacks );
	GetGameLobby().Initialize( idLobby::TYPE_GAME, sessionCallbacks );
	GetGameStateLobby().Initialize( idLobby::TYPE_GAME_STATE, sessionCallbacks );

	achievementSystem = new (TAG_SYSTEM) idAchievementSystemQnx();
	achievementSystem->Init();
}

/*
========================
idSessionLocalQnx::Shutdown
========================
*/
void idSessionLocalQnx::Shutdown() {
	NET_VERBOSE_PRINT( "NET: Shutdown\n" );
	idSessionLocal::Shutdown();

	MoveToMainMenu();

	// Wait until we fully shutdown
	while ( localState != STATE_IDLE && localState != STATE_PRESS_START ) {
		Pump();
	}

	if ( achievementSystem != NULL ) {
		achievementSystem->Shutdown();
		delete achievementSystem;
		achievementSystem = NULL;
	}
}

/*
========================
idSessionLocalQnx::InitializeSoundRelatedSystems
========================
*/
void idSessionLocalQnx::InitializeSoundRelatedSystems() {
	if ( voiceChat != NULL ) {
		voiceChat->Init( NULL );
	}
}

/*
========================
idSessionLocalQnx::ShutdownSoundRelatedSystems
========================
*/
void idSessionLocalQnx::ShutdownSoundRelatedSystems() {
	if ( voiceChat != NULL ) {
		voiceChat->Shutdown();
	}
}

/*
========================
idSessionLocalQnx::PlatformPump
========================
*/
void idSessionLocalQnx::PlatformPump() {
}

/*
========================
idSessionLocalQnx::InviteFriends
========================
*/
void idSessionLocalQnx::InviteFriends() {
}

/*
========================
idSessionLocalQnx::InviteParty
========================
*/
void idSessionLocalQnx::InviteParty() {
}

/*
========================
idSessionLocalQnx::ShowPartySessions
========================
*/
void idSessionLocalQnx::ShowPartySessions() {
}

/*
========================
idSessionLocalQnx::ShowSystemMarketplaceUI
========================
*/
void idSessionLocalQnx::ShowSystemMarketplaceUI() const {
}

/*
========================
idSessionLocalQnx::ListServers
========================
*/
void idSessionLocalQnx::ListServers( const idCallback & callback ) {
	ListServersCommon();
}

/*
========================
idSessionLocalQnx::CancelListServers
========================
*/
void idSessionLocalQnx::CancelListServers() {
}

/*
========================
idSessionLocalQnx::NumServers
========================
*/
int idSessionLocalQnx::NumServers() const {
	return 0;
}

/*
========================
idSessionLocalQnx::ServerInfo
========================
*/
const serverInfo_t * idSessionLocalQnx::ServerInfo( int i ) const {
	return NULL;
}

/*
========================
idSessionLocalQnx::ConnectToServer
========================
*/
void idSessionLocalQnx::ConnectToServer( int i ) {
}

/*
========================
idSessionLocalQnx::Connect_f
========================
*/
void idSessionLocalQnx::Connect_f( const idCmdArgs &args ) {
	if ( args.Argc() < 2 ) {
		idLib::Printf( "Usage: Connect to IP.  Use with net_port. \n");
		return;
	}

	Cancel();

	if ( signInManager->GetMasterLocalUser() == NULL ) {
		signInManager->RegisterLocalUser( 0 );
	}

	lobbyConnectInfo_t connectInfo;

	Sys_StringToNetAdr( args.Argv(1), &connectInfo.netAddr, true );
	connectInfo.netAddr.port = net_port.GetInteger();

	ConnectAndMoveToLobby( GetPartyLobby(), connectInfo, false );
}

/*
========================
void Connect_f
========================
*/
CONSOLE_COMMAND( connect, "Connect to the specified IP", NULL ) {
	sessionLocalQnx.Connect_f( args );
}

/*
========================
idSessionLocalQnx::ShowServerGamerCardUI
========================
*/
void idSessionLocalQnx::ShowServerGamerCardUI( int i ) {
}

/*
========================
idSessionLocalQnx::ShowLobbyUserGamerCardUI(
========================
*/
void idSessionLocalQnx::ShowLobbyUserGamerCardUI( lobbyUserID_t lobbyUserID ) {
}

/*
========================
idSessionLocalQnx::ProcessInputEvent
========================
*/
bool idSessionLocalQnx::ProcessInputEvent( const sysEvent_t * ev ) {
	if ( GetSignInManager().ProcessInputEvent( ev ) ) {
		return true;
	}
	return false;
}

/*
========================
idSessionLocalQnx::IsSystemUIShowing
========================
*/
bool idSessionLocalQnx::IsSystemUIShowing() const {
	return ( qnx.windowState != NAVIGATOR_WINDOW_FULLSCREEN ) || isSysUIShowing; // If the user switches apps, treat it the same as bringing up the steam overlay
}

/*
========================
idSessionLocalQnx::SetSystemUIShowing
========================
*/
void idSessionLocalQnx::SetSystemUIShowing( bool show ) {
	isSysUIShowing = show;
}

/*
========================
idSessionLocalQnx::HandleServerQueryRequest
========================
*/
void idSessionLocalQnx::HandleServerQueryRequest( lobbyAddress_t & remoteAddr, idBitMsg & msg, int msgType ) {
	NET_VERBOSE_PRINT( "HandleServerQueryRequest from %s\n", remoteAddr.ToString() );
}

/*
========================
idSessionLocalQnx::HandleServerQueryAck
========================
*/
void idSessionLocalQnx::HandleServerQueryAck( lobbyAddress_t & remoteAddr, idBitMsg & msg ) {
	NET_VERBOSE_PRINT( "HandleServerQueryAck from %s\n", remoteAddr.ToString() );

}

/*
========================
idSessionLocalQnx::ClearBootableInvite
========================
*/
void idSessionLocalQnx::ClearBootableInvite() {
}

/*
========================
idSessionLocalQnx::ClearPendingInvite
========================
*/
void idSessionLocalQnx::ClearPendingInvite() {
}

/*
========================
idSessionLocalQnx::HandleBootableInvite
========================
*/
void idSessionLocalQnx::HandleBootableInvite( int64 lobbyId ) {
}

/*
========================
idSessionLocalQnx::HasPendingBootableInvite
========================
*/
bool idSessionLocalQnx::HasPendingBootableInvite() {
	return false;
}

/*
========================
idSessionLocal::SetDiscSwapMPInvite
========================
*/
void idSessionLocalQnx::SetDiscSwapMPInvite( void * parm ) {
}

/*
========================
idSessionLocal::GetDiscSwapMPInviteParms
========================
*/
void * idSessionLocalQnx::GetDiscSwapMPInviteParms() {
	return NULL;
}

/*
========================
idSessionLocalQnx::EnumerateDownloadableContent
========================
*/
void idSessionLocalQnx::EnumerateDownloadableContent() {
}

/*
========================
idSessionLocalQnx::LeaderboardUpload
========================
*/
void idSessionLocalQnx::LeaderboardUpload( lobbyUserID_t lobbyUserID, const leaderboardDefinition_t * leaderboard, const column_t * stats, const idFile_Memory * attachment ) {
}

/*
========================
idSessionLocalQnx::LeaderboardFlush
========================
*/
void idSessionLocalQnx::LeaderboardFlush() {
}

/*
========================
idSessionLocalQnx::LeaderboardDownload
========================
*/
void idSessionLocalQnx::LeaderboardDownload( int sessionUserIndex, const leaderboardDefinition_t * leaderboard, int startingRank, int numRows, const idLeaderboardCallback & callback ) {
}

/*
========================
idSessionLocalQnx::LeaderboardDownloadAttachment
========================
*/
void idSessionLocalQnx::LeaderboardDownloadAttachment( int sessionUserIndex, const leaderboardDefinition_t * leaderboard, int64 attachmentID ) {
}

/*
========================
idSessionLocalQnx::EnsurePort
========================
*/
void idSessionLocalQnx::EnsurePort() {
	// Init the port using reqular windows sockets
	if ( port.IsOpen() ) {
		return;		// Already initialized
	}

	if ( port.InitPort( net_port.GetInteger(), false ) ) {
		canJoinLocalHost = false;
	} else {
		// Assume this is another instantiation on the same machine, and just init using any available port
		port.InitPort( PORT_ANY, false );
		canJoinLocalHost = true;
	}
}

/*
========================
idSessionLocalQnx::GetPort
========================
*/
idNetSessionPort & idSessionLocalQnx::GetPort( bool dedicated ) {
	EnsurePort();
	return port;
}

/*
========================
idSessionLocalQnx::CreateLobbyBackend
========================
*/
idLobbyBackend * idSessionLocalQnx::CreateLobbyBackend( const idMatchParameters & p, float skillLevel, idLobbyBackend::lobbyBackendType_t lobbyType ) {
	idLobbyBackend * lobbyBackend = CreateLobbyInternal( lobbyType );
	lobbyBackend->StartHosting( p, skillLevel, lobbyType );
	return lobbyBackend;
}

/*
========================
idSessionLocalQnx::FindLobbyBackend
========================
*/
idLobbyBackend * idSessionLocalQnx::FindLobbyBackend( const idMatchParameters & p, int numPartyUsers, float skillLevel, idLobbyBackend::lobbyBackendType_t lobbyType ) {
	idLobbyBackend * lobbyBackend = CreateLobbyInternal( lobbyType );
	lobbyBackend->StartFinding( p, numPartyUsers, skillLevel );
	return lobbyBackend;
}

/*
========================
idSessionLocalQnx::JoinFromConnectInfo
========================
*/
idLobbyBackend * idSessionLocalQnx::JoinFromConnectInfo( const lobbyConnectInfo_t & connectInfo, idLobbyBackend::lobbyBackendType_t lobbyType ) {
	idLobbyBackend * lobbyBackend = CreateLobbyInternal( lobbyType );
	lobbyBackend->JoinFromConnectInfo( connectInfo );
	return lobbyBackend;
}

/*
========================
idSessionLocalQnx::DestroyLobbyBackend
========================
*/
void idSessionLocalQnx::DestroyLobbyBackend( idLobbyBackend * lobbyBackend ) {
    assert( lobbyBackend != NULL );
    assert( lobbyBackends[lobbyBackend->GetLobbyType()] == lobbyBackend );

	lobbyBackends[lobbyBackend->GetLobbyType()] = NULL;

	lobbyBackend->Shutdown();
	delete lobbyBackend;
}

/*
========================
idSessionLocalQnx::PumpLobbies
========================
*/
void idSessionLocalQnx::PumpLobbies() {
	assert( lobbyBackends[idLobbyBackend::TYPE_PARTY] == NULL || lobbyBackends[idLobbyBackend::TYPE_PARTY]->GetLobbyType() == idLobbyBackend::TYPE_PARTY );
	assert( lobbyBackends[idLobbyBackend::TYPE_GAME] == NULL || lobbyBackends[idLobbyBackend::TYPE_GAME]->GetLobbyType() == idLobbyBackend::TYPE_GAME );
	assert( lobbyBackends[idLobbyBackend::TYPE_GAME_STATE] == NULL || lobbyBackends[idLobbyBackend::TYPE_GAME_STATE]->GetLobbyType() == idLobbyBackend::TYPE_GAME_STATE );

	// Pump lobbyBackends
	for ( int i = 0; i < lobbyBackends.Num(); i++ ) {
		if ( lobbyBackends[i] != NULL ) {
			lobbyBackends[i]->Pump();
		}
	}
}

/*
========================
idSessionLocalQnx::CreateLobbyInternal
========================
*/
idLobbyBackend * idSessionLocalQnx::CreateLobbyInternal( idLobbyBackend::lobbyBackendType_t lobbyType ) {
	EnsurePort();
	idLobbyBackend * lobbyBackend = new (TAG_NETWORKING) idLobbyBackendDirect();

	lobbyBackend->SetLobbyType( lobbyType );

	assert( lobbyBackends[lobbyType] == NULL );
	lobbyBackends[lobbyType] = lobbyBackend;

	return lobbyBackend;
}

/*
========================
idSessionLocalQnx::JoinAfterSwap
========================
*/
void idSessionLocalQnx::JoinAfterSwap( void * joinID ) {
}

/*
========================
idSessionLocalQnx::GetLobbyAddressFromNetAddress
========================
*/
bool idSessionLocalQnx::GetLobbyAddressFromNetAddress( const netadr_t & netAddr, lobbyAddress_t & outAddr ) const {
	return false;
}

/*
========================
idSessionLocalQnx::GetNetAddressFromLobbyAddress
========================
*/
bool idSessionLocalQnx::GetNetAddressFromLobbyAddress( const lobbyAddress_t & lobbyAddress, netadr_t & outNetAddr ) const {
	return false;
}
