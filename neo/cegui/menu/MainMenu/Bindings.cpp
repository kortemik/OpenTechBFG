/*
 * Bindings.cpp
 *
 *  Created on: Aug 13, 2015
 *      Author: kordex
 */

#include <cegui/menu/MainMenu/Bindings.h>

namespace BFG {
namespace CEGUIMenu {

Bindings::Bindings() :
GameMenu("MainMenu/Settings/Controls/Bindings.layout")
{

}

Bindings::~Bindings() {

}

void Bindings::init()
{
	CreateCEGUIWindow();
	setVisible( true );
}

void Bindings::destroy()
{

}

void Bindings::LoadNestedWindows()
{

}
void Bindings::RegisterHandlers()
{
	// TODO
}


} /* namespace CEGUIMenu */
} /* namespace BFG */
