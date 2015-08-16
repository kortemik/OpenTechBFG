/*
 * Advanced.cpp
 *
 *  Created on: Aug 16, 2015
 *      Author: kordex
 */

#include <cegui/menu/MainMenu/Advanced.h>

namespace BFG {
namespace CEGUIMenu {

Advanced::Advanced() :
		GameMenu("MainMenu/Settings/Advanced.layout")
{

}

Advanced::~Advanced() {
}

void Advanced::init()
{
	CreateCEGUIWindow();
	setVisible( true );
	window->setText("Advanced");
}

void Advanced::destroy()
{

}

void Advanced::LoadNestedWindows()
{

}
void Advanced::RegisterHandlers()
{

}

} /* namespace CEGUIMenu */
} /* namespace BFG */
