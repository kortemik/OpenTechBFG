/*
 * CEGUIConsole.h
 *
 *  Created on: Aug 13, 2014
 *      Author: kordex
 */

#ifndef ConsoleImpl_H_
#define ConsoleImpl_H_

#include <CEGUI/CEGUI.h>
#include "../idlib/Str.h"

#include "ConsoleMsg.h"

namespace CEGUIConsole {

class ConsoleImpl {
public:

	static ConsoleImpl& getInstance() {
		static ConsoleImpl instance;
		return instance;
	}

	void setVisible(bool visible);
	bool isVisible();
	void OutputText(ConsoleMsg outMsg);
	void TabComplete(void);
	static void AutoCompleteCallback(const char *s);
	void TabCompleteListAdd(CEGUI::String option);

private:
	void CreateCEGUIWindow();
	void RegisterHandlers();
	bool Handle_TextSubmitted(const CEGUI::EventArgs&);

	void Execute(CEGUI::String inMsg);
	void PopulateHistory(void);

	struct ConsoleImplVars;
	ConsoleImplVars *ourVars;

	ConsoleImpl();
	virtual ~ConsoleImpl();
	ConsoleImpl(ConsoleImpl const&);
	void operator=(ConsoleImpl const&);

	void ScrollBottom();
};

} /* namespace CEGUIConsole */

#endif /* CEGUICONSOLE_H_ */
