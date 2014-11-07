/*
 * CEGUIConsole.cpp
 *
 *  Created on: Aug 13, 2014
 *      Author: kordex
 */

#include <CEGUI/widgets/Combobox.h>
#include "CEGUI_Console.h"
#include "../idlib/Str.h"
#include "../framework/ConsoleHistory.h"
#include "../framework/CmdSystem.h"

struct CEGUI_Console::CEGUI_ConsoleVars {
};


CEGUI_Console::CEGUI_Console()
{
	CreateCEGUIWindow();
	RegisterHandlers();
	setVisible(false);
}

CEGUI_Console::~CEGUI_Console() {
	// TODO Auto-generated destructor stub
}

void CEGUI_Console::CreateCEGUIWindow()
{
	CEGUI::System::getSingleton().getDefaultGUIContext()
						.getRootWindow()->addChild(
								CEGUI::WindowManager::getSingleton().
								loadLayoutFromFile("console.layout"));
}
void CEGUI_Console::RegisterHandlers()
{
	CEGUI::Window *ConsoleWin = CEGUI::System::getSingleton().getDefaultGUIContext()
							.getRootWindow()->getChild("Console");

	// FIXME catches TAB key for some reason

	/*
	 * handles mouse on submit to input
	 */
	ConsoleWin->getChild("Submit")->subscribeEvent(
			CEGUI::PushButton::EventClicked,
			&CEGUI_Console::Handle_TextSubmitted,
			(this)
			);
	/*
	 * handles keypress (enter) to input
	 */
	ConsoleWin->getChild("Combobox")->subscribeEvent(
			CEGUI::Combobox::EventTextAccepted,
			&CEGUI_Console::Handle_TextSubmitted,
			(this)
			);

}

bool CEGUI_Console::Handle_TextSubmitted(const CEGUI::EventArgs& args)
{
	CEGUI::Window *ConsoleWin = CEGUI::System::getSingleton().getDefaultGUIContext()
								.getRootWindow()->getChild("Console");

	CEGUI::String Line = ConsoleWin->getChild("Combobox")->getText();

	(this)->Execute(Line);

	ConsoleWin->getChild("Combobox")->setText("");

	return true;
}

const CEGUI::String CEGUI_Console::FormatConvert(const char *convertString)
{
	return convertString;
	// see http://cegui.org.uk/wiki/Formatting_Tags_in_CEGUI

	/* FIXME see the Console.cpp for working implementation
	idStr formatedStr;
	CEGUI::String coloredString;

	int currentColor = idStr::ColorIndex( C_COLOR_WHITE );

	for( int x = 0; x > 0; x++ )
	{

		// checks if string ends
		if( ( convertString[x] & 0xff ) == ' ' )
		{
			continue;
		}

		if( idStr::ColorIndex( convertString[x] >> 8 ) != currentColor )
		{
			currentColor = idStr::ColorIndex( convertString[x] >> 8 );

			formatedStr.Append(convertString[x] & 0xff);

			switch(currentColor)
			{

			case 1:	// C_COLOR_RED
				coloredString.append("[colour='FFFF0000']\0");
				break;
			case 2:	// C_COLOR_GREEN
				coloredString.append("[colour='FF00FF00']\0");
				break;
			case 3:	// C_COLOR_YELLOW
				coloredString.append("[colour='FFFFFF00']\0");
				break;
			case 4:	// C_COLOR_BLUE
				coloredString.append("[colour='FF0000FF']\0");
				break;
			case 5:	// C_COLOR_CYAN
				coloredString.append("[colour='FF33CCCC']\0");
				break;
			case 6:	// C_COLOR_ORANGE
				coloredString.append("[colour='FFFF6600']\0");
				break;
			case 7:	// C_COLOR_WHITE
				coloredString.append("[colour='FFFFFFFF']\0");
				break;
			case 8:	// C_COLOR_GRAY
				coloredString.append("[colour='FF808080']\0");
				break;
			case 9:	// C_COLOR_BLACK
				coloredString.append("[colour='FF000000']\0");
				break;
			case 0: // C_COLOR_DEFAULT
			default: // C_COLOR_DEFAULT
				coloredString.append("[colour='FF33CCCC']\0");
				break;
			}
		}
		coloredString.append(formatedStr.c_str());
	}

	return coloredString;
	*/
}

void CEGUI_Console::OutputText(const CEGUI::String inMsg)
{
	/*
	 * checks if cegui is initialized
	 */
	if (CEGUI::System::getSingletonPtr() != NULL)
	{

		const CEGUI::String outMsg = (this)->FormatConvert(inMsg.c_str());

		CEGUI::Window *ConsoleWin = CEGUI::System::getSingleton().getDefaultGUIContext()
									.getRootWindow()->getChild("Console");
		if (ConsoleWin != NULL)
		{
			CEGUI::Listbox *outputWindow = static_cast<CEGUI::Listbox*>(ConsoleWin->getChild("History"));

			CEGUI::ListboxTextItem* newItem=0;
			newItem = new CEGUI::ListboxTextItem(outMsg);
			outputWindow->addItem(newItem);

			(this)->ScrollBottom();
		}
	}
}


void CEGUI_Console::setVisible(bool visible)
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

bool CEGUI_Console::isVisible()
{
	CEGUI::Window *ConsoleWin = CEGUI::System::getSingleton().getDefaultGUIContext()
											.getRootWindow()->getChild("Console");
	return ConsoleWin->isVisible();
}

void CEGUI_Console::ScrollBottom()
{
	CEGUI::Window *ConsoleWin = CEGUI::System::getSingleton().getDefaultGUIContext()
										.getRootWindow()->getChild("Console");

	CEGUI::Listbox *outputWindow = static_cast<CEGUI::Listbox*>(ConsoleWin->getChild("History"));

	float document_size = outputWindow->getVertScrollbar()->getDocumentSize();
	float page_size = outputWindow->getVertScrollbar()->getPageSize();


	outputWindow->getVertScrollbar()->setScrollPosition(document_size - page_size);
}

void CEGUI_Console::Execute(CEGUI::String inMsg)
{
	const char *cmd = inMsg.c_str();
	if (strlen(cmd) >= 1)
	{

		cmdSystem->BufferCommandText( CMD_EXEC_APPEND, cmd );	// valid command
		cmdSystem->BufferCommandText( CMD_EXEC_APPEND, "\n" );
		consoleHistory.AddToHistory( cmd );
	}
}
void CEGUI_Console::PopulateHistory(void)
{
	// TODO implement cegui window historylist aka the dropdown list,
	// should be filled on access
	idStr hist = consoleHistory.RetrieveFromHistory( true );
}

void CEGUI_Console::TabComplete(void)
{
	CEGUI::Window *ConsoleWin = CEGUI::System::getSingleton().getDefaultGUIContext()
												.getRootWindow()->getChild("Console");

	CEGUI::String cmdStub = ConsoleWin->getChild("Combobox")->getText();

	idStr cmdMatch = (this)->AutoComplete(cmdStub.c_str());

	if (!cmdMatch.IsEmpty()) {
		ConsoleWin->getChild("Combobox")->setText(cmdMatch.c_str());
	}

}

idStr CEGUI_Console::AutoComplete(const char *cmdStub){
	// TODO AutoComplete should show cegui tooltips for multiple matches
	/* TODO implement
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
	*/
	return NULL;
}


