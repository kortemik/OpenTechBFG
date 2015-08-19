/*
 * Multiplayer.cpp
 *
 *  Created on: Aug 19, 2015
 *      Author: kordex
 */

#include <cegui/menu/MainMenu/Multiplayer.h>

namespace BFG {
namespace CEGUIMenu {

Multiplayer::Multiplayer() :
	GameMenu( "MainMenu/Multiplayer.layout" )
{

}

Multiplayer::~Multiplayer() {
}

void Multiplayer::init()
{
	CreateCEGUIWindow();
	setVisible( true );
	window->setText("Multiplayer");
}

void Multiplayer::destroy()
{

}

void Multiplayer::LoadNestedWindows()
{

}
void Multiplayer::RegisterHandlers()
{

}

} /* namespace CEGUIMenu */
} /* namespace BFG */
