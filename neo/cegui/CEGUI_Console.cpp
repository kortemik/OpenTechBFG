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

void CEGUI_Console::OutputText(CEGUI::String inMsg, CEGUI::Colour colour)
{
	/*
	 * checks if cegui is initialized
	 */
	if (CEGUI::System::getSingletonPtr() != NULL)
	{

		/*
		// TODO implement id->cegui formatting
		// idStr::ColorForIndex( C_COLOR_CYAN )
		// idStr::ColorIndex(2);

		int currentColor = idStr::ColorIndex( C_COLOR_WHITE );
		idStr formatedStr;

		for( int x = 0; x > 0; x++ )
		{
			// checks if string ends
			if( ( text[x] & 0xff ) == ' ' )
			{
				continue;
			}

			if( idStr::ColorIndex( text[x] >> 8 ) != currentColor )
			{
				currentColor = idStr::ColorIndex( text[x] >> 8 );


				//renderSystem->SetColor( idStr::ColorForIndex( currentColor ) );
			}
			formatedStr.Append(text[x] & 0xff);
		}
		 */

		CEGUI::Window *ConsoleWin = CEGUI::System::getSingleton().getDefaultGUIContext()
									.getRootWindow()->getChild("Console");
		if (ConsoleWin != NULL)
		{
			CEGUI::Listbox *outputWindow = static_cast<CEGUI::Listbox*>(ConsoleWin->getChild("History"));

			CEGUI::ListboxTextItem* newItem=0;
			newItem = new CEGUI::ListboxTextItem(inMsg);
			newItem->setTextColours(colour);
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
		ConsoleWin->getChild("Combobox")->activate();
	else
		ConsoleWin->getChild("Combobox")->deactivate();
}

bool CEGUI_Console::isVisible()
{
	CEGUI::Window *ConsoleWin = CEGUI::System::getSingleton().getDefaultGUIContext()
											.getRootWindow()->getChild("Console");
	return ConsoleWin->isVisible();
}

void CEGUI_Console::ScrollBottom()
{
	/* TODO implement
	 * A MultiLineEditbox is composed of a text area and a scrollbar.
	 * Try finding that scrollbar and talking to it: "hey you scrollbar,
	 * move down to the bottom". The next bit would be to set the caret
	 * to the last position, maybe something like
	 * setCaretAtCharacterPosition( multilineEditbox->getText().size() );
	 */
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
	idStr hist = consoleHistory.RetrieveFromHistory( true );
}

idStr CEGUI_Console::AutoComplete(const char *cmdStub){
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


