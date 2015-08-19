#include "MainMenu.h"

#include "cegui/CEGUI_Hooks.h" // for initialization check

#include "../framework/CmdSystem.h"

#include "Campaign.h"
#include "Multiplayer.h"
#include "Settings.h"
#include "Credits.h"
#include "Quit.h"

namespace BFG
{

namespace CEGUIMenu
{
MainMenu::MainMenu() :
	GameMenu( "MainMenu/MainMenu.layout" ),
	campaign(),
	multiplayer(),
	settings(),
	credits(),
	quit()
{

}

MainMenu::~MainMenu()
{

}

void MainMenu::init()
{
	CreateCEGUIWindow();
	AttachCEGUIRootWindow();
	setVisible( false );
	
	LoadNestedWindows();
	RegisterHandlers();
}

void MainMenu::destroy()
{
	campaign->destroy();
	delete campaign;

	multiplayer->destroy();
	delete multiplayer;

	settings->destroy();
	delete settings;

	credits->destroy();
	delete credits;

	quit->destroy();
	delete quit;
}

void MainMenu::AttachCEGUIRootWindow()
{
	if( idCEGUI::IsInitialized() )
	{
		CEGUI::System::getSingleton().getDefaultGUIContext()
		.getRootWindow()->addChild( window );
	}
}


void MainMenu::LoadNestedWindows()
{
	CEGUI::TabControl *tabScreen = static_cast<CEGUI::TabControl*>(window->getChild("TabScreen"));
	/*
	 * campaign
	 */
	campaign = new Campaign();
	campaign->init();
	tabScreen->addTab(campaign->getWindowPtr());

	/*
	 * multiplayer
	 */
	multiplayer = new Multiplayer();
	multiplayer->init();
	tabScreen->addTab(multiplayer->getWindowPtr());

	/*
	 * settings
	 */
	settings = new Settings();
	settings->init();
	tabScreen->addTab( settings->getWindowPtr() );

	/*
	 * credits
	 */
	credits = new Credits();
	credits->init();
	tabScreen->addTab(credits->getWindowPtr());

	/*
	 * quit
	 */
	quit = new Quit();
	quit->init();
	tabScreen->addTab(quit->getWindowPtr());
}

void MainMenu::RegisterHandlers()
{
	if( idCEGUI::IsInitialized() )
	{
	/*
		window->getChild( "MenuVertical/TopSelect/Campaign" )->subscribeEvent(
			CEGUI::PushButton::EventClicked,
			&MainMenu::Handle_CampaignClick,
			( this )
		);
		
		window->getChild( "MenuVertical/TopSelect/Multiplayer" )->subscribeEvent(
			CEGUI::PushButton::EventClicked,
			&MainMenu::Handle_MultiplayerClick,
			( this )
		);
		
		window->getChild( "MenuVertical/TopSelect/Settings" )->subscribeEvent(
			CEGUI::PushButton::EventClicked,
			&MainMenu::Handle_SettingsClick,
			( this )
		);
		
		window->getChild( "MenuVertical/TopSelect/Credits" )->subscribeEvent(
			CEGUI::PushButton::EventClicked,
			&MainMenu::Handle_CreditsClick,
			( this )
		);
		
		window->getChild( "MenuVertical/TopSelect/Quit" )->subscribeEvent(
			CEGUI::PushButton::EventClicked,
			&MainMenu::Handle_QuitClick,
			( this )
		);
		*/
	}
}

void MainMenu::setVisible( bool visible )
{
	if( idCEGUI::IsInitialized() )
	{
		window->setVisible( visible );
		
		if( visible )
		{
			CEGUI::System::getSingleton().getDefaultGUIContext().getMouseCursor().show();
		}
		else
		{
			CEGUI::System::getSingleton().getDefaultGUIContext().getMouseCursor().hide();
		}
	}
}


bool MainMenu::Handle_CampaignClick( const CEGUI::EventArgs& )
{
	if( idCEGUI::IsInitialized() )
	{
		BFG::cmdSystem->BufferCommandText( BFG::CMD_EXEC_APPEND, "map testmaps/test\n" );
		return true;
	}
	return false;
}

bool MainMenu::Handle_MultiplayerClick( const CEGUI::EventArgs& )
{
	if( idCEGUI::IsInitialized() )
	{
		return true;
	}
	return false;
}

bool MainMenu::Handle_SettingsClick( const CEGUI::EventArgs& )
{
	if( idCEGUI::IsInitialized() )
	{
		settings->setVisible( true );
		return true;
	}
	return false;
}

bool MainMenu::Handle_CreditsClick( const CEGUI::EventArgs& )
{
	if( idCEGUI::IsInitialized() )
	{
		return true;
	}
	return false;
}

bool MainMenu::Handle_QuitClick( const CEGUI::EventArgs& )
{
	if( idCEGUI::IsInitialized() )
	{
		BFG::cmdSystem->BufferCommandText( BFG::CMD_EXEC_APPEND, "quit\n" );
		return true;
	}
	return false;
}

} /* namespace CEGUIMenu */

} // namespace BFG
