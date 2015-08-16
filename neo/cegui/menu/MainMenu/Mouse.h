/*
 * Mouse.h
 *
 *  Created on: Aug 16, 2015
 *      Author: kordex
 */

#ifndef _MAINMENU_MOUSE_H_
#define _MAINMENU_MOUSE_H_

#include <cegui/menu/GameMenu.h>

namespace BFG {
namespace CEGUIMenu {

class Mouse: public GameMenu {
public:
	Mouse();
	virtual ~Mouse();

	virtual void init();
	virtual void destroy();


protected:
	virtual void RegisterHandlers();
	virtual void LoadNestedWindows();

};

} /* namespace CEGUIMenu */
} /* namespace BFG */

#endif /* _MAINMENU_MOUSE_H_ */
