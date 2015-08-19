/*
 * Campaign.cpp
 *
 *  Created on: Aug 19, 2015
 *      Author: kordex
 */

#include <cegui/menu/MainMenu/Campaign.h>

namespace BFG {
namespace CEGUIMenu {

Campaign::Campaign() :
	GameMenu( "MainMenu/Campaign.layout" )
{

}

Campaign::~Campaign() {
}

void Campaign::init()
{
	CreateCEGUIWindow();
	setVisible( true );
	window->setText("Campaign");
}

void Campaign::destroy()
{

}

void Campaign::LoadNestedWindows()
{

}
void Campaign::RegisterHandlers()
{

}

} /* namespace CEGUIMenu */
} /* namespace BFG */
