/*
 * VideoMain.h
 *
 *  Created on: Aug 16, 2015
 *      Author: kordex
 */

#ifndef _MAINMENU_VIDEOGENERAL_H_
#define _MAINMENU_VIDEOGENERAL_H_

#include <cegui/menu/GameMenu.h>

namespace BFG {
namespace CEGUIMenu {

class VideoGeneral: public GameMenu {
public:
	VideoGeneral();
	virtual ~VideoGeneral();

	virtual void init();
	virtual void destroy();


protected:
	virtual void RegisterHandlers();
	virtual void LoadNestedWindows();

};

} /* namespace CEGUIMenu */
} /* namespace BFG */

#endif /* _MAINMENU_VIDEOGENERAL_H_ */
