/*
 * ConsoleComboBox.h
 *
 *  Created on: Jan 14, 2015
 *      Author: kordex
 */

#ifndef CONSOLECOMBOBOX_H_
#define CONSOLECOMBOBOX_H_

#include <CEGUI/widgets/Combobox.h>

namespace CEGUIConsole {

class ConsoleComboBox: public CEGUI::Combobox {
public:

	ConsoleComboBox(const CEGUI::String& type, const CEGUI::String& name)
	: Combobox(type, name) {};

    static const CEGUI::String WidgetTypeName;             //!< Window factory name

    static const CEGUI::String EditboxName;    //!< Widget name for the editbox component.

};

}
#endif /* CONSOLECOMBOBOX_H_ */


