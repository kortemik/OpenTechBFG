/*
 * Console.cpp
 *
 *  Created on: Sep 5, 2014
 *      Author: kordex
 */


#include "Console.h"

#include "ConsoleImpl.h"

namespace CEGUIConsole {

struct Console::ConsoleVars {
	ConsoleImpl *consoleInstance;
	idStr *missedLog[999];
	int missedLogIndex;
};

Console::Console()
: ourVars(new ConsoleVars)
{
	ourVars->consoleInstance = NULL;
	ourVars->missedLogIndex = 0;
}

Console::~Console() {
	// TODO Auto-generated destructor stub
}

bool Console::isInitialized() {
	// checks if cegui is up and running
	if (CEGUI::System::getSingletonPtr() != NULL) {
		if (ourVars->consoleInstance == NULL)
		{
			// initializes the console singleton
			ourVars->consoleInstance = &ConsoleImpl::getInstance();

			while( ourVars->missedLogIndex > 0 )
			{
				// displays buffered text from missedLog
				// TODO we should unload the missedLog in a way
				// that it would be displayed first
				// perhaps locking
				ourVars->missedLogIndex--;
				ourVars->consoleInstance->OutputText((ourVars->missedLog[ourVars->missedLogIndex])->c_str());
				delete(ourVars->missedLog[ourVars->missedLogIndex]);
			}
		}
		return true;
	}
	else
		return false;
}

bool Console::Active()
{
	if (isInitialized())
		return ourVars->consoleInstance->isVisible();
	else
		return false;
}

void Console::Open()
{
	if (isInitialized()) {
		ourVars->consoleInstance->setVisible(true);
	}
}

void Console::Close()
{
	if (isInitialized()) {
		ourVars->consoleInstance->setVisible(false);
	}
}

void Console::TabComplete(void)
{
	if (isInitialized()) {
		ourVars->consoleInstance->TabComplete();
	}
}

void Console::Print(const char * text)
{

	if (isInitialized())
	{
		ourVars->consoleInstance->OutputText(text);
	}
	else
	{
		// buffer the text into missedLog;
		// and increment the index
		if (ourVars->missedLogIndex <= 999)
		{
			ourVars->missedLog[ourVars->missedLogIndex] = new idStr(text);
			ourVars->missedLogIndex++;
		}
	}
}

bool Console::ProcessEvent( const sysEvent_t* event, bool forceAccept )
{
	if (isInitialized()) {
		const bool consoleKey = event->evType == SE_KEY && event->evValue == K_GRAVE && com_allowConsole.GetBool();

		const bool tabKey = event->evType == SE_KEY && event->evValue == K_TAB;

		// we always catch the console key event
		if( !forceAccept && consoleKey )
		{
			// ignore up events
			if( event->evValue2 == 0 )
			{
				return true;
			}

			// if window is Active we close it with consoleKey
			if( Active() )
			{
				Close();
				Sys_GrabMouseCursor( true );
			}
			else
			{
				Open();
				Sys_GrabMouseCursor( false );
			}
			return true;
		}

		// return event as processed when console open
		// aka don't pass the input further
		if( Active() )
		{
			if(tabKey) {
				TabComplete();
			}
			return true;
		}

		// if we aren't Active, dump all the other events
		if( !forceAccept && !Active() )
		{
			return false;
		}

		if (forceAccept)
		{
			// a dummy here
			return true;
		}

		// we don't handle things like mouse, joystick, and network packets
		return false;
	}
	return false;
}
} /* namespace CEGUIConsole */

static CEGUIConsole::Console localConsole;
idConsole* console = &localConsole;
