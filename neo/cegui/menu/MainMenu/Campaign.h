/*
 * Campaign.h
 *
 *  Created on: Aug 19, 2015
 *      Author: kordex
 */

#ifndef _MAINMENU_AMPAIGN_H_
#define _MAINMENU_CAMPAIGN_H_

#include <cegui/menu/GameMenu.h>

namespace BFG {
namespace CEGUIMenu {

class Campaign: public GameMenu {
public:
	Campaign();
	virtual ~Campaign();

	virtual void init();
	virtual void destroy();


protected:
	virtual void LoadNestedWindows();
	virtual void RegisterHandlers();
};

} /* namespace CEGUIMenu */
} /* namespace BFG */

#endif /* _MAINMENU_CAMPAIGN_H_ */
