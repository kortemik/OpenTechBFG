/*
 * Video3DOptions.h
 *
 *  Created on: Aug 16, 2015
 *      Author: kordex
 */

#ifndef _MAINMENU_VIDEO3DOPTIONS_H_
#define _MAINMENU_VIDEO3DOPTIONS_H_

#include <cegui/menu/GameMenu.h>

namespace BFG {
namespace CEGUIMenu {

class Video3DOptions: public GameMenu {
public:
	Video3DOptions();
	virtual ~Video3DOptions();

	virtual void init();
	virtual void destroy();


protected:
	virtual void RegisterHandlers();
	virtual void LoadNestedWindows();

};

} /* namespace CEGUIMenu */
} /* namespace BFG */

#endif /* _MAINMENU_VIDEO3DOPTIONS_H_ */
