/*
 * Audio.h
 *
 *  Created on: Aug 16, 2015
 *      Author: kordex
 */

#ifndef _MAINMENU_AUDIO_H_
#define _MAINMENU_AUDIO_H_

#include <cegui/menu/GameMenu.h>

namespace BFG {
namespace CEGUIMenu {

class Audio: public GameMenu {
public:
	Audio();
	virtual ~Audio();

	virtual void init();
	virtual void destroy();


protected:
	virtual void RegisterHandlers();
	virtual void LoadNestedWindows();

};

} /* namespace CEGUIMenu */
} /* namespace BFG */

#endif /* _MAINMENU_AUDIO_H_ */
