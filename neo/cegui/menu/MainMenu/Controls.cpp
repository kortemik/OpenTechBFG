/*
 * Controls.cpp
 *
 *  Created on: Aug 13, 2015
 *      Author: kordex
 */

#include <cegui/menu/MainMenu/Controls.h>
#include "Bindings.h"
#include "Controller.h"
#include "Mouse.h"

namespace BFG {
namespace CEGUIMenu {

Controls::Controls() :
	GameMenu("MainMenu/Settings/Controls.layout"),
	bindings(),
	controller(),
	mouse()
{

}

Controls::~Controls() {

}

void Controls::init()
{
	CreateCEGUIWindow();
	setVisible( true );
	window->setText("Controls");
	LoadNestedWindows();
}

void Controls::destroy()
{
	delete bindings;
	delete controller;
	delete mouse;
}

void Controls::RegisterHandlers()
{

}

void Controls::LoadNestedWindows()
{
	// tab control
	CEGUI::TabControl *controlTabs = static_cast<CEGUI::TabControl*>(window);

	/*
	 * bindings
	 */
	bindings = new Bindings();
	bindings->init();
	controlTabs->addTab(bindings->getWindowPtr());

	/*
	 * controller
	 */
	controller = new Controller();
	controller->init();
	controlTabs->addTab(controller->getWindowPtr());


	/*
	 * mouse
	 */
	mouse = new Mouse();
	mouse->init();
	controlTabs->addTab(mouse->getWindowPtr());
}



} /* namespace CEGUIMenu */
} /* namespace BFG */
