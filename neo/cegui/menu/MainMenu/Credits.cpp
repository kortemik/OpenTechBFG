/*
 * Credits.cpp
 *
 *  Created on: Aug 19, 2015
 *      Author: kordex
 */

#include <cegui/menu/MainMenu/Credits.h>

namespace BFG {
namespace CEGUIMenu {

Credits::Credits() :
	GameMenu( "MainMenu/Credits.layout" )
{

}

Credits::~Credits() {
}

void Credits::init()
{
	CreateCEGUIWindow();
	setVisible( true );
	window->setText("Credits");
}

void Credits::destroy()
{

}

void Credits::LoadNestedWindows()
{

}
void Credits::RegisterHandlers()
{

}

} /* namespace CEGUIMenu */
} /* namespace BFG */
