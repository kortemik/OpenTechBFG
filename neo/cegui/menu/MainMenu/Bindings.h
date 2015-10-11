/*
 * Bindings.h
 *
 *  Created on: Aug 13, 2015
 *      Author: kordex
 */

#ifndef _MAINMENU_BINDINGS_H_
#define _MAINMENU_BINDINGS_H_

#include <cegui/menu/GameMenu.h>

namespace BFG {
namespace CEGUIMenu {

class Bindings: public GameMenu {
public:
	Bindings();
	virtual ~Bindings();

	virtual void init();
	virtual void destroy();


protected:
	virtual void RegisterHandlers();
	virtual void LoadNestedWindows();

};

} /* namespace CEGUIMenu */
} /* namespace BFG */

#endif /* _MAINMENU_BINDINGS_H_ */