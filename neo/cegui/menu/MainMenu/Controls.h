/*
 * Controls.h
 *
 *  Created on: Aug 13, 2015
 *      Author: kordex
 */

#ifndef _MAINMENU_CONTROLS_H_
#define _MAINMENU_CONTROLS_H_

#include <cegui/menu/GameMenu.h>

namespace BFG {
namespace CEGUIMenu {

class Controls: public GameMenu {
public:
	Controls();
	virtual ~Controls();

	virtual void init();
	virtual void destroy();


protected:
	virtual void RegisterHandlers();
	virtual void LoadNestedWindows();

private:
	GameMenu *bindings;
	GameMenu *controller;
	GameMenu *mouse;

};

} /* namespace CEGUIMenu */
} /* namespace BFG */

#endif /* _MAINMENU_CONTROLS_H_ */
