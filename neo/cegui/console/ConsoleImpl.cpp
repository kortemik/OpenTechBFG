/*
 * CEGUIConsole.cpp
 *
 *  Created on: Aug 13, 2014
 *      Author: kordex
 */

#include <memory>
#include <CEGUI/widgets/Combobox.h>
#include "ConsoleImpl.h"
#include "ConsoleMsg.h"

#include "../idlib/Str.h"
#include "../framework/ConsoleHistory.h"
#include "../framework/CmdSystem.h"

namespace CEGUIConsole {

struct ConsoleImpl::ConsoleImplVars {
};


ConsoleImpl::ConsoleImpl()
{
	CreateCEGUIWindow();
	RegisterHandlers();
	setVisible(false);
}

ConsoleImpl::~ConsoleImpl() {
	// TODO Auto-generated destructor stub
}

void ConsoleImpl::CreateCEGUIWindow()
{
	CEGUI::System::getSingleton().getDefaultGUIContext()
						.getRootWindow()->addChild(
								CEGUI::WindowManager::getSingleton().
								loadLayoutFromFile("console.layout"));
}
void ConsoleImpl::RegisterHandlers()
{
	CEGUI::Window *ConsoleWin = CEGUI::System::getSingleton().getDefaultGUIContext()
							.getRootWindow()->getChild("Console");

	// FIXME catches TAB key for some reason

	/*
	 * handles mouse on submit to input
	 */
	ConsoleWin->getChild("Submit")->subscribeEvent(
			CEGUI::PushButton::EventClicked,
			&ConsoleImpl::Handle_TextSubmitted,
			(this)
			);
	/*
	 * handles keypress (enter) to input
	 */
	ConsoleWin->getChild("Combobox")->subscribeEvent(
			CEGUI::Combobox::EventTextAccepted,
			&ConsoleImpl::Handle_TextSubmitted,
			(this)
			);

}

bool ConsoleImpl::Handle_TextSubmitted(const CEGUI::EventArgs& args)
{

	const CEGUI::KeyEventArgs& keyEventArg =
		    static_cast<const CEGUI::KeyEventArgs&>(args);

	if (keyEventArg.scancode != CEGUI::Key::Tab)
	{

		CEGUI::Window *ConsoleWin = CEGUI::System::getSingleton().getDefaultGUIContext()
										.getRootWindow()->getChild("Console");

		CEGUI::String Line = ConsoleWin->getChild("Combobox")->getText();

		(this)->Execute(Line);

		ConsoleWin->getChild("Combobox")->setText("");
	}
	return true;

}

void ConsoleImpl::OutputText(ConsoleMsg outMsg)
{
	/*
	 * checks if cegui is initialized
	 */
	if (CEGUI::System::getSingletonPtr() != NULL)
	{
		//ConsoleMsg outMsg = ConsoleMsg(inMsg);

		CEGUI::Window *ConsoleWin = CEGUI::System::getSingleton().getDefaultGUIContext()
									.getRootWindow()->getChild("Console");
		if (ConsoleWin != NULL)
		{
			CEGUI::Listbox *outputWindow = static_cast<CEGUI::Listbox*>(ConsoleWin->getChild("History"));

			CEGUI::ListboxTextItem* newItem=0;
			newItem = new CEGUI::ListboxTextItem(outMsg.msg);
			outputWindow->addItem(newItem);

			(this)->ScrollBottom();
		}

	}
}


void ConsoleImpl::setVisible(bool visible)
{
	CEGUI::Window *ConsoleWin = CEGUI::System::getSingleton().getDefaultGUIContext()
										.getRootWindow()->getChild("Console");
	ConsoleWin->setVisible(visible);

	if(visible)
	{
		ConsoleWin->getChild("Combobox")->activate();
	    CEGUI::System::getSingleton().getDefaultGUIContext().getMouseCursor().show();
	}
	else {
		ConsoleWin->getChild("Combobox")->deactivate();
		CEGUI::System::getSingleton().getDefaultGUIContext().getMouseCursor().hide();
	}
}

bool ConsoleImpl::isVisible()
{
	CEGUI::Window *ConsoleWin = CEGUI::System::getSingleton().getDefaultGUIContext()
											.getRootWindow()->getChild("Console");
	return ConsoleWin->isVisible();
}

void ConsoleImpl::ScrollBottom()
{
	CEGUI::Window *ConsoleWin = CEGUI::System::getSingleton().getDefaultGUIContext()
										.getRootWindow()->getChild("Console");

	CEGUI::Listbox *outputWindow = static_cast<CEGUI::Listbox*>(ConsoleWin->getChild("History"));

	float document_size = outputWindow->getVertScrollbar()->getDocumentSize();
	float page_size = outputWindow->getVertScrollbar()->getPageSize();


	outputWindow->getVertScrollbar()->setScrollPosition(document_size - page_size);
}

void ConsoleImpl::Execute(CEGUI::String inMsg)
{
	const char *cmd = inMsg.c_str();
	if (strlen(cmd) >= 1)
	{

		cmdSystem->BufferCommandText( CMD_EXEC_APPEND, cmd );	// valid command
		cmdSystem->BufferCommandText( CMD_EXEC_APPEND, "\n" );
		consoleHistory.AddToHistory( cmd );
	}
}
void ConsoleImpl::PopulateHistory(void)
{
	// TODO implement cegui window historylist aka the dropdown list,
	// should be filled on access
	idStr hist = consoleHistory.RetrieveFromHistory( true );
}

void ConsoleImpl::TabComplete(void)
{
	CEGUI::Window *ConsoleWin = CEGUI::System::getSingleton().getDefaultGUIContext()
												.getRootWindow()->getChild("Console");

	CEGUI::String cmdStub = ConsoleWin->getChild("Combobox")->getText();

	idStr cmdMatch = (this)->AutoComplete(cmdStub.c_str());

	if (!cmdMatch.IsEmpty()) {
		ConsoleWin->getChild("Combobox")->setText(cmdMatch.c_str());
	}

}

void ConsoleImpl::TabToolTip(const char *s)
{
	// this little fellow just gets all the matches and is responsible for acting accordingly
	// for now let's test and print..

	getInstance().OutputText(ConsoleMsg(s));
#if 0
	int		i;
	const char *completionString = "ma"; //p
	char *currentMatch;
	int matchCount = 0;

	if( idStr::Icmpn( buf, completionString, strlen( completionString ) ) != 0 )
	{
		return;
	}
	matchCount++;
	if( matchCount == 1 )
	{
		idStr::Copynz( currentMatch, buf, sizeof( currentMatch ) );
		return;
	}

	// cut currentMatch to the amount common with s
	for( i = 0; buf[i]; i++ )
	{
		if( tolower( currentMatch[i] ) != tolower( buf[i] ) )
		{
			currentMatch[i] = 0;
			break;
		}
	}
	currentMatch[i] = 0;
#endif
}

idStr ConsoleImpl::AutoComplete(const char *cmdStub){
	// TODO AutoComplete should show cegui tooltips for multiple matches
	// TODO implement
	const char *cmdString = "map";
	cmdSystem->CommandCompletion( TabToolTip );
	cmdSystem->ArgCompletion( cmdString, TabToolTip );
#if 0
	char completionArgString[MAX_EDIT_LINE];
	idCmdArgs args;

	if( !autoComplete.valid )
	{
		args.TokenizeString( buffer, false );
		idStr::Copynz( autoComplete.completionString, args.Argv( 0 ), sizeof( autoComplete.completionString ) );
		idStr::Copynz( completionArgString, args.Args(), sizeof( completionArgString ) );
		autoComplete.matchCount = 0;
		autoComplete.matchIndex = 0;
		autoComplete.currentMatch[0] = 0;

		if( strlen( autoComplete.completionString ) == 0 )
		{
			return;
		}

		globalAutoComplete = autoComplete;

		cmdSystem->CommandCompletion( FindMatches );
		cvarSystem->CommandCompletion( FindMatches );

		autoComplete = globalAutoComplete;

		if( autoComplete.matchCount == 0 )
		{
			return;	// no matches
		}

		// when there's only one match or there's an argument
		if( autoComplete.matchCount == 1 || completionArgString[0] != '\0' )
		{

			/// try completing arguments
			idStr::Append( autoComplete.completionString, sizeof( autoComplete.completionString ), " " );
			idStr::Append( autoComplete.completionString, sizeof( autoComplete.completionString ), completionArgString );
			autoComplete.matchCount = 0;

			globalAutoComplete = autoComplete;

			cmdSystem->ArgCompletion( autoComplete.completionString, FindMatches );
			cvarSystem->ArgCompletion( autoComplete.completionString, FindMatches );

			autoComplete = globalAutoComplete;

			idStr::snPrintf( buffer, sizeof( buffer ), "%s", autoComplete.currentMatch );

			if( autoComplete.matchCount == 0 )
			{
				// no argument matches
				idStr::Append( buffer, sizeof( buffer ), " " );
				idStr::Append( buffer, sizeof( buffer ), completionArgString );
				SetCursor( strlen( buffer ) );
				return;
			}
		}
		else
		{

			// multiple matches, complete to shortest
			idStr::snPrintf( buffer, sizeof( buffer ), "%s", autoComplete.currentMatch );
			if( strlen( completionArgString ) )
			{
				idStr::Append( buffer, sizeof( buffer ), " " );
				idStr::Append( buffer, sizeof( buffer ), completionArgString );
			}
		}

		autoComplete.length = strlen( buffer );
		autoComplete.valid = ( autoComplete.matchCount != 1 );
		SetCursor( autoComplete.length );

		common->Printf( "]%s\n", buffer );

		// run through again, printing matches
		globalAutoComplete = autoComplete;

		cmdSystem->CommandCompletion( PrintMatches );
		cmdSystem->ArgCompletion( autoComplete.completionString, PrintMatches );
		cvarSystem->CommandCompletion( PrintCvarMatches );
		cvarSystem->ArgCompletion( autoComplete.completionString, PrintMatches );

	}
	else if( autoComplete.matchCount != 1 )
	{

		// get the next match and show instead
		autoComplete.matchIndex++;
		if( autoComplete.matchIndex == autoComplete.matchCount )
		{
			autoComplete.matchIndex = 0;
		}
		autoComplete.findMatchIndex = 0;

		globalAutoComplete = autoComplete;

		cmdSystem->CommandCompletion( FindIndexMatch );
		cmdSystem->ArgCompletion( autoComplete.completionString, FindIndexMatch );
		cvarSystem->CommandCompletion( FindIndexMatch );
		cvarSystem->ArgCompletion( autoComplete.completionString, FindIndexMatch );

		autoComplete = globalAutoComplete;

		// and print it
		idStr::snPrintf( buffer, sizeof( buffer ), autoComplete.currentMatch );
		if( autoComplete.length > ( int )strlen( buffer ) )
		{
			autoComplete.length = strlen( buffer );
		}
		SetCursor( autoComplete.length );
	}
#endif
	return NULL;
}

} /* namespace CEGUIConsole */
