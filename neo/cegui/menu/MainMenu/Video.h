/*
 * Video.h
 *
 *  Created on: Aug 16, 2015
 *      Author: kordex
 */

#ifndef _MAINMENU_VIDEO_H_
#define _MAINMENU_VIDEO_H_

#include <cegui/menu/GameMenu.h>

namespace BFG {
namespace CEGUIMenu {

class Video: public GameMenu {
public:
	Video();
	virtual ~Video();

	virtual void init();
	virtual void destroy();


protected:
	virtual void RegisterHandlers();
	virtual void LoadNestedWindows();

private:
	GameMenu *video3doptions;
	GameMenu *videogeneral;

};

} /* namespace CEGUIMenu */
} /* namespace BFG */

#endif /* _MAINMENU_VIDEO_H_ */
