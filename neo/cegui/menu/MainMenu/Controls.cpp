/*
 * Controls.cpp
 *
 *  Created on: Aug 13, 2015
 *      Author: kordex
 */

#include <cegui/menu/MainMenu/Controls.h>
#include "Bindings.h"

namespace BFG {
namespace CEGUIMenu {

Controls::Controls() :
	GameMenu("MainMenu/Settings/Controls.layout"),
	bindings()
{

}

Controls::~Controls() {

}

void Controls::init()
{
	CreateCEGUIWindow();
	setVisible( true );

	LoadNestedWindows();
}

void Controls::destroy()
{
	delete bindings;
}

void Controls::RegisterHandlers()
{

}

void Controls::LoadNestedWindows()
{
	bindings = new Bindings();
	bindings->init();
	CEGUI::TabControl *controlTabs = static_cast<CEGUI::TabControl*>(window);
	controlTabs->addTab(bindings->getWindowPtr());
}



} /* namespace CEGUIMenu */
} /* namespace BFG */
