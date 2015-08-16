/*
 * Audio.cpp
 *
 *  Created on: Aug 16, 2015
 *      Author: kordex
 */

#include <cegui/menu/MainMenu/Audio.h>

namespace BFG {
namespace CEGUIMenu {

Audio::Audio() :
		GameMenu("MainMenu/Settings/Audio.layout"){

}

Audio::~Audio() {
}

void Audio::init()
{
	CreateCEGUIWindow();
	setVisible( true );
	window->setText("Audio");
}

void Audio::destroy()
{

}

void Audio::LoadNestedWindows()
{

}
void Audio::RegisterHandlers()
{

}

} /* namespace CEGUIMenu */
} /* namespace BFG */
