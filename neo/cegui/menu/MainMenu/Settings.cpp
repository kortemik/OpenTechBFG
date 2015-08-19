/*
 * Settings.cpp
 *
 *  Created on: Apr 26, 2015
 *      Author: kordex
 */

#include <cegui/menu/MainMenu/Settings.h>

#include "Advanced.h"
#include "Audio.h"
#include "Controls.h"
#include "Options.h"
#include "Video.h"

namespace BFG
{

namespace CEGUIMenu
{

Settings::Settings() :
	GameMenu( "MainMenu/Settings.layout" ),
	advanced(),
	audio(),
	controls(),
	options(),
	video()
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

	window->setText("Settings");

}

void Settings::destroy()
{
	advanced->destroy();
	delete advanced;
	audio->destroy();
	delete audio;
	controls->destroy();
	delete controls;
	options->destroy();
	delete options;
	video->destroy();
	delete video;
}

void Settings::LoadNestedWindows()
{
	// tab control
	CEGUI::TabControl *settingTabs = static_cast<CEGUI::TabControl*>(window);

	/*
	 * advanced
	 */
	advanced = new Advanced();
	advanced->init();
	settingTabs->addTab(advanced->getWindowPtr());

	/*
	 * audio
	 */
	audio = new Audio();
	audio->init();
	settingTabs->addTab(audio->getWindowPtr());

	/*
	 * controls
	 */
	controls = new Controls();
	controls->init();
	settingTabs->addTab(controls->getWindowPtr());

	/*
	 * Options
	 */
	options = new Options();
	options->init();
	settingTabs->addTab(options->getWindowPtr());

	/*
	 * Video
	 */
	video = new Video();
	video->init();
	settingTabs->addTab(video->getWindowPtr());
}

void Settings::RegisterHandlers()
{

}

} /* namespace CEGUIMenu */

} // namespace BFG
