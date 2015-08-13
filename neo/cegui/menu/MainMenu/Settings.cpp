/*
 * Settings.cpp
 *
 *  Created on: Apr 26, 2015
 *      Author: kordex
 */

#include <cegui/menu/MainMenu/Settings.h>
#include "Controls.h"

namespace BFG
{

namespace CEGUIMenu
{

Settings::Settings() :
	GameMenu( "MainMenu/Settings.layout" ),
	controls()
{

}

Settings::~Settings()
{

}

void Settings::init()
{
	CreateCEGUIWindow();
	setVisible( false );
	
	LoadNestedWindows();
	RegisterHandlers();
}

void Settings::destroy()
{
	delete controls;
}

void Settings::LoadNestedWindows()
{
	controls = new Controls();
	controls->init();
	CEGUI::TabControl *settingTabs = static_cast<CEGUI::TabControl*>(window->getChild("TabScreen"));
	settingTabs->addTab(controls->getWindowPtr());
}

void Settings::RegisterHandlers()
{
	window->subscribeEvent(
		CEGUI::FrameWindow::EventCloseClicked,
		&Settings::hide,
		( this )
	);
}

void Settings::hide()
{
	setVisible( false );
}

} /* namespace CEGUIMenu */

} // namespace BFG
