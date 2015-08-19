/*
 * Multiplayer.h
 *
 *  Created on: Aug 19, 2015
 *      Author: kordex
 */

#ifndef _MAINMENU_MULTIPLAYER_H_
#define _MAINMENU_MULTIPLAYER_H_

#include <cegui/menu/GameMenu.h>

namespace BFG {
namespace CEGUIMenu {

class Multiplayer: public GameMenu {
public:
	Multiplayer();
	virtual ~Multiplayer();

	virtual void init();
	virtual void destroy();


protected:
	virtual void LoadNestedWindows();
	virtual void RegisterHandlers();
};

} /* namespace CEGUIMenu */
} /* namespace BFG */

#endif /* _MAINMENU_MULTIPLAYER_H_ */
