/*
 * ConsoleEditBox.h
 *
 *  Created on: Jan 13, 2015
 *      Author: kordex
 */

#ifndef CONSOLEEDITBOX_H_
#define CONSOLEEDITBOX_H_

#include <CEGUI/widgets/Editbox.h>

namespace CEGUIConsole {

class ConsoleEditBox: public CEGUI::Editbox {
public:
	ConsoleEditBox();
	virtual ~ConsoleEditBox();

    void onKeyDown(CEGUI::KeyEventArgs& e);
};

} /* namespace CEGUIConsole */

#endif /* CONSOLEEDITBOX_H_ */
