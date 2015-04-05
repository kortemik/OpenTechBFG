/*
 * MenuLocator.h
 *
 *  Created on: 5.4.2015
 *      Author: Administrator
 */

#ifndef _MENULOCATOR_H_
#define _MENULOCATOR_H_

#include "GameMenu.h"
#include "MainMenu.h"

namespace CEGUIMenu {

class MenuLocator {
public:
	MenuLocator();
	virtual ~MenuLocator();

	static GameMenu& getMain() { return localMainMenu; }

private:
	static MainMenu localMainMenu;
};

MainMenu MenuLocator::localMainMenu;

} /* namespace CEGUIMenu */

#endif /* _MENULOCATOR_H_ */

