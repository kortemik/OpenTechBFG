/*
 * CEGUIConsole.h
 *
 *  Created on: Aug 13, 2014
 *      Author: kordex
 */

#ifndef CEGUI_CONSOLE_H_
#define CEGUI_CONSOLE_H_

#include <CEGUI/CEGUI.h>
#include "../idlib/Str.h"

class CEGUI_Console {
public:

	static CEGUI_Console& getInstance() {
		static CEGUI_Console instance;
		return instance;
	}

	void setVisible(bool visible);
	bool isVisible();
	void OutputText(CEGUI::String inMsg,
			CEGUI::Colour colour = CEGUI::Colour( 0xFFFFFFFF));

private:
	void CreateCEGUIWindow();
	void RegisterHandlers();
	bool Handle_TextSubmitted(const CEGUI::EventArgs&);

	void Execute(CEGUI::String inMsg);
	void PopulateHistory(void);
	idStr AutoComplete(const char *cmdStub);

	struct CEGUI_ConsoleVars;
	CEGUI_ConsoleVars *ourVars;

	CEGUI_Console();
	virtual ~CEGUI_Console();
	CEGUI_Console(CEGUI_Console const&);
	void operator=(CEGUI_Console const&);

	void ScrollBottom();
};

#endif /* CEGUICONSOLE_H_ */
