/*
 * Controller.cpp
 *
 *  Created on: Aug 16, 2015
 *      Author: kordex
 */

#include <cegui/menu/MainMenu/Controller.h>

namespace BFG {
namespace CEGUIMenu {

Controller::Controller() :
		GameMenu("MainMenu/Settings/Controls/Controller.layout")
{

}

Controller::~Controller() {
}

void Controller::init()
{
	CreateCEGUIWindow();
	setVisible( true );
	window->setText("Controller");
}

void Controller::destroy()
{

}

void Controller::LoadNestedWindows()
{

}
void Controller::RegisterHandlers()
{

}

} /* namespace CEGUIMenu */
} /* namespace BFG */
