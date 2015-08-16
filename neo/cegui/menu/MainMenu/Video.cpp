/*
 * Video.cpp
 *
 *  Created on: Aug 16, 2015
 *      Author: kordex
 */

#include <cegui/menu/MainMenu/Video.h>

namespace BFG {
namespace CEGUIMenu {

Video::Video() :
		GameMenu("MainMenu/Settings/Video.layout")
{
	// TODO Auto-generated constructor stub

}

Video::~Video() {
	// TODO Auto-generated destructor stub
}

void Video::init()
{
	CreateCEGUIWindow();
	setVisible( true );
	window->setText("Video");
	LoadNestedWindows();
}

void Video::destroy()
{

}

void Video::LoadNestedWindows()
{
	// tab control
	CEGUI::TabControl *controlTabs = static_cast<CEGUI::TabControl*>(window);

	/*
	 * VideoMain
	 */
	videomain = new VideoMain();
	videomain->init();
	controlTabs->addTab(videomain->getWindowPtr());

	/*
	 * Video3DOptions
	 */
	video3doptions = new Video3DOptions();
	video3doptions->init();
	controlTabs->addTab(video3doptions->getWindowPtr());
}

void Video::RegisterHandlers()
{

}

} /* namespace CEGUIMenu */
} /* namespace BFG */
