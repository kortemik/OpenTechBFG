/*
 * Options.cpp
 *
 *  Created on: Aug 16, 2015
 *      Author: kordex
 */

#include <cegui/menu/MainMenu/Options.h>

namespace BFG {
namespace CEGUIMenu {

Options::Options() :
		GameMenu("MainMenu/Settings/Options.layout")
{
	// TODO Auto-generated constructor stub

}

Options::~Options() {
	// TODO Auto-generated destructor stub
}

void Options::init()
{
	CreateCEGUIWindow();
	setVisible( true );
	window->setText("Options");
}

void Options::destroy()
{

}

void Options::LoadNestedWindows()
{

}
void Options::RegisterHandlers()
{

}

} /* namespace CEGUIMenu */
} /* namespace BFG */
