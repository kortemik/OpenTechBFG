/*
 * Video.cpp
 *
 *  Created on: Aug 16, 2015
 *      Author: kordex
 */

#include <cegui/menu/MainMenu/Video.h>
#include "VideoGeneral.h"
#include "Video3DOptions.h"

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
	videogeneral->destroy();
	delete videogeneral;

	video3doptions->destroy();
	delete video3doptions;
}

void Video::LoadNestedWindows()
{
	// tab control
	CEGUI::TabControl *controlTabs = static_cast<CEGUI::TabControl*>(window);

	/*
	 * VideoMain
	 */
	videogeneral = new VideoGeneral();
	videogeneral->init();
	controlTabs->addTab(videogeneral->getWindowPtr());

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
