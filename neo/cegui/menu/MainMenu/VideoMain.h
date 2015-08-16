/*
 * VideoMain.h
 *
 *  Created on: Aug 16, 2015
 *      Author: kordex
 */

#ifndef _MAINMENU_VIDEOMAIN_H_
#define _MAINMENU_VIDEOMAIN_H_

#include <cegui/menu/GameMenu.h>

namespace BFG {
namespace CEGUIMenu {

class VideoMain: public GameMenu {
public:
	VideoMain();
	virtual ~VideoMain();

	virtual void init();
	virtual void destroy();


protected:
	virtual void RegisterHandlers();
	virtual void LoadNestedWindows();

};

} /* namespace CEGUIMenu */
} /* namespace BFG */

#endif /* _MAINMENU_VIDEOMAIN_H_ */
