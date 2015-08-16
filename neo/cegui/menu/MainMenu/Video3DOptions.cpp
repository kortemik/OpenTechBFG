/*
 * Video3DOptions.cpp
 *
 *  Created on: Aug 16, 2015
 *      Author: kordex
 */

#include <cegui/menu/MainMenu/Video3DOptions.h>

namespace BFG {
namespace CEGUIMenu {

Video3DOptions::Video3DOptions() :
		GameMenu("MainMenu/Settings/Video/3DOptions.layout")
{
	// TODO Auto-generated constructor stub

}

Video3DOptions::~Video3DOptions() {
	// TODO Auto-generated destructor stub
}

void Video3DOptions::init()
{
	CreateCEGUIWindow();
	setVisible( true );
	window->setText("3D Options");
}

void Video3DOptions::destroy()
{

}

void Video3DOptions::LoadNestedWindows()
{

}
void Video3DOptions::RegisterHandlers()
{

}

} /* namespace CEGUIMenu */
} /* namespace BFG */
