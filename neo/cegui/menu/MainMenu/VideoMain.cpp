/*
 * VideoMain.cpp
 *
 *  Created on: Aug 16, 2015
 *      Author: kordex
 */

#include <cegui/menu/MainMenu/VideoMain.h>

namespace BFG {
namespace CEGUIMenu {

VideoMain::VideoMain() :
		GameMenu("MainMenu/Settings/Video/Main.layout")
{
	// TODO Auto-generated constructor stub

}

VideoMain::~VideoMain() {
	// TODO Auto-generated destructor stub
}

void VideoMain::init()
{
	CreateCEGUIWindow();
	setVisible( true );
	window->setText("Main");
}

void VideoMain::destroy()
{

}

void VideoMain::LoadNestedWindows()
{

}
void VideoMain::RegisterHandlers()
{

}

} /* namespace CEGUIMenu */
} /* namespace BFG */
