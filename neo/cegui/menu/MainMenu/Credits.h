/*
 * Credits.h
 *
 *  Created on: Aug 19, 2015
 *      Author: kordex
 */

#ifndef _MAINMENU_CREDITS_H_
#define _MAINMENU_CREDITS_H_

#include <cegui/menu/GameMenu.h>

namespace BFG {
namespace CEGUIMenu {

class Credits: public GameMenu {
public:
	Credits();
	virtual ~Credits();

	virtual void init();
	virtual void destroy();


protected:
	virtual void LoadNestedWindows();
	virtual void RegisterHandlers();
};

} /* namespace CEGUIMenu */
} /* namespace BFG */

#endif /* _MAINMENU_CREDITS_H_ */
