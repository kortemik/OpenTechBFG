/*
 * Quit.cpp
 *
 *  Created on: Aug 19, 2015
 *      Author: kordex
 */

#include <cegui/menu/MainMenu/Quit.h>

namespace BFG {
namespace CEGUIMenu {

Quit::Quit() :
	GameMenu( "MainMenu/Quit.layout" )
{

}

Quit::~Quit() {
}

void Quit::init()
{
	CreateCEGUIWindow();
	setVisible( true );
	window->setText("Quit");
}

void Quit::destroy()
{

}

void Quit::LoadNestedWindows()
{

}
void Quit::RegisterHandlers()
{

}

} /* namespace CEGUIMenu */
} /* namespace BFG */
