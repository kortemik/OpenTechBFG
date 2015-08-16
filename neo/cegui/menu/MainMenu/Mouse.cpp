/*
 * Mouse.cpp
 *
 *  Created on: Aug 16, 2015
 *      Author: kordex
 */

#include <cegui/menu/MainMenu/Mouse.h>

namespace BFG {
namespace CEGUIMenu {

Mouse::Mouse() :
		GameMenu("MainMenu/Settings/Controls/Mouse.layout")
{
	// TODO Auto-generated constructor stub

}

Mouse::~Mouse() {
	// TODO Auto-generated destructor stub
}

void Mouse::init()
{
	CreateCEGUIWindow();
	setVisible( true );
	window->setText("Mouse");
}

void Mouse::destroy()
{

}

void Mouse::LoadNestedWindows()
{

}
void Mouse::RegisterHandlers()
{

}

} /* namespace CEGUIMenu */
} /* namespace BFG */
