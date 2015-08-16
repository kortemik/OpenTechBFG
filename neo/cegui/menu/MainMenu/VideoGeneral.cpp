/*
 * VideoMain.cpp
 *
 *  Created on: Aug 16, 2015
 *      Author: kordex
 */

#include <cegui/menu/MainMenu/VideoGeneral.h>

namespace BFG {
namespace CEGUIMenu {

VideoGeneral::VideoGeneral() :
		GameMenu("MainMenu/Settings/Video/Main.layout")
{
	// TODO Auto-generated constructor stub

}

VideoGeneral::~VideoGeneral() {
	// TODO Auto-generated destructor stub
}

void VideoGeneral::init()
{
	CreateCEGUIWindow();
	setVisible( true );
	window->setText("General");
}

void VideoGeneral::destroy()
{

}

void VideoGeneral::LoadNestedWindows()
{

}
void VideoGeneral::RegisterHandlers()
{

}

} /* namespace CEGUIMenu */
} /* namespace BFG */
