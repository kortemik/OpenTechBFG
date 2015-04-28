/*
 * Settings.cpp
 *
 *  Created on: Apr 26, 2015
 *      Author: kordex
 */

#include <cegui/menu/MainMenu/Settings.h>

namespace CEGUIMenu {

Settings::Settings() :
	GameMenu("MainMenu/Settings.layout")
{


}

Settings::~Settings() {

}

void Settings::init()
{
	CreateCEGUIWindow();
	setVisible( true );

	LoadNestedWindows();
	RegisterHandlers();
}

void Settings::destroy()
{

}

void Settings::LoadNestedWindows()
{

}

void Settings::RegisterHandlers()
{

}

} /* namespace CEGUIMenu */
