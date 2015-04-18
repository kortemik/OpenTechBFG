#include "MainMenu.h"

#include "cegui/CEGUI_Hooks.h" // for initialization check

#include "../framework/CmdSystem.h"

namespace CEGUIMenu
{
MainMenu::MainMenu() :
	GameMenu( "MainMenu.layout", "MainMenu" )
{
	RegisterHandlers();
}

MainMenu::~MainMenu()
{
	// TODO MainMenu destructor
}
void MainMenu::RegisterHandlers()
{
	if( idCEGUI::IsInitialized() )
	{
		/*
		 * handles mouse on submit to input
		 */
		rootWindow->getChild("Bottom/Left/Campaign")->subscribeEvent(
			CEGUI::PushButton::EventClicked,
			&MainMenu::Handle_DemoMapClick,
			( this )
		);
		/*
		* handles keypress (enter) to input
		*/

		rootWindow->getChild("Bottom/Left/Quit")->subscribeEvent(
			CEGUI::Combobox::EventTextAccepted,
			&MainMenu::Handle_QuitClick,
			( this )
		);
	}
}

bool MainMenu::Handle_DemoMapClick( const CEGUI::EventArgs& )
{
	if( idCEGUI::IsInitialized() )
	{
		BFG::cmdSystem->BufferCommandText( BFG::CMD_EXEC_APPEND, "map demo.map\n" );
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
