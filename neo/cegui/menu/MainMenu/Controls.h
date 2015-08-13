/*
 * Controls.h
 *
 *  Created on: Aug 13, 2015
 *      Author: kordex
 */

#ifndef CONTROLS_H_
#define CONTROLS_H_

#include <cegui/menu/GameMenu.h>

namespace BFG {
namespace CEGUIMenu {

class Controls: public GameMenu {
public:
	Controls();
	virtual ~Controls();

	virtual void init();
	virtual void destroy();


protected:
	virtual void RegisterHandlers();
	virtual void LoadNestedWindows();

private:
	GameMenu *bindings;

};

} /* namespace CEGUIMenu */
} /* namespace BFG */

#endif /* CONTROLS_H_ */
