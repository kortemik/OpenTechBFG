/*
 * Quit.h
 *
 *  Created on: Aug 19, 2015
 *      Author: kordex
 */

#ifndef _MAINMENU_QUIT_H_
#define _MAINMENU_QUIT_H_

#include <cegui/menu/GameMenu.h>

namespace BFG {
namespace CEGUIMenu {

class Quit: public GameMenu {
public:
	Quit();
	virtual ~Quit();

	virtual void init();
	virtual void destroy();


protected:
	virtual void LoadNestedWindows();
	virtual void RegisterHandlers();
};

} /* namespace CEGUIMenu */
} /* namespace BFG */

#endif /* _MAINMENU_QUIT_H_ */
