/*
 * Advanced.h
 *
 *  Created on: Aug 16, 2015
 *      Author: kordex
 */

#ifndef _MAINMENU_ADVANCED_H_
#define _MAINMENU_ADVANCED_H_

#include <cegui/menu/GameMenu.h>

namespace BFG {
namespace CEGUIMenu {

class Advanced: public GameMenu {
public:
	Advanced();
	virtual ~Advanced();

	virtual void init();
	virtual void destroy();


protected:
	virtual void RegisterHandlers();
	virtual void LoadNestedWindows();

};

} /* namespace CEGUIMenu */
} /* namespace BFG */

#endif /* _MAINMENU_ADVANCED_H_ */
