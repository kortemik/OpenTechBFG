#include "MainmenuImpl.h"

#include "cegui/CEGUI_Hooks.h" // for initialization check

#include "../framework/CmdSystem.h"

namespace CEGUIMenu
{

MainmenuImpl::MainmenuImpl()
{
	CreateCEGUIWindow();
	RegisterHandlers();
	setVisible( true );
}


MainmenuImpl::~MainmenuImpl()
{
	// TODO should remove subscriptions and windows
}

void MainmenuImpl::CreateCEGUIWindow()
{
	if( idCEGUI::IsInitialized() )
	{
		CEGUI::System::getSingleton().getDefaultGUIContext()
		.getRootWindow()->addChild(
			CEGUI::WindowManager::getSingleton().
			loadLayoutFromFile( "mainmenu.layout" ) );
	}
}

void MainmenuImpl::RegisterHandlers()
{
	if( idCEGUI::IsInitialized() )
	{
		CEGUI::Window* MainmenuWin = CEGUI::System::getSingleton().getDefaultGUIContext()
									 .getRootWindow()->getChild( "MainMenu" );
									 
									 
		/*
		* handles mouse on submit to input
		*/
		MainmenuWin->getChild( "DemoMapClickArea" )->subscribeEvent(
			CEGUI::PushButton::EventClicked,
			&MainmenuImpl::Handle_DemoMapClick,
			( this )
		);
		/*
		* handles keypress (enter) to input
		*/
		
		// FIXME catches tab too, some way to avoid that?
		MainmenuWin->getChild( "QuitClickArea" )->subscribeEvent(
			CEGUI::Combobox::EventTextAccepted,
			&MainmenuImpl::Handle_QuitClick,
			( this )
		);
	}
}

bool MainmenuImpl::Handle_DemoMapClick( const CEGUI::EventArgs& )
{
	if( idCEGUI::IsInitialized() )
	{
		cmdSystem->BufferCommandText( CMD_EXEC_APPEND, "map demo.map\n" );
		return true;
	}
	return false;
}

bool MainmenuImpl::Handle_QuitClick( const CEGUI::EventArgs& )
{
	if( idCEGUI::IsInitialized() )
	{
		cmdSystem->BufferCommandText( CMD_EXEC_APPEND, "quit\n" );
		return true;
	}
	return false;
}

void MainmenuImpl::setVisible( bool visible )
{
	if( idCEGUI::IsInitialized() )
	{
		CEGUI::Window* MainmenuWin = CEGUI::System::getSingleton().getDefaultGUIContext()
									 .getRootWindow()->getChild( "MainMenu" );
		MainmenuWin->setVisible( visible );
		
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

bool MainmenuImpl::isVisible()
{
	if( idCEGUI::IsInitialized() )
	{
		CEGUI::Window* MainmenuWin = CEGUI::System::getSingleton().getDefaultGUIContext()
									 .getRootWindow()->getChild( "MainMenu" );
		return MainmenuWin->isVisible();
	}
	return false;
}

} /* namespace CEGUIMenu */
