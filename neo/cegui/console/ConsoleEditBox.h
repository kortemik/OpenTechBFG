/*
 * ConsoleEditBox.h
 *
 *  Created on: Jan 13, 2015
 *      Author: kordex
 */

#ifndef CONSOLEEDITBOX_H_
#define CONSOLEEDITBOX_H_

#include "../wrap_cegui.h"

namespace BFG
{

namespace CEGUIConsole
{

class ConsoleEditBox: public CEGUI::Editbox
{
public:
	ConsoleEditBox( const CEGUI::String& type, const CEGUI::String& name )
		: Editbox( type, name ) {};
		
	static const CEGUI::String WidgetTypeName;
	
	void onKeyDown( CEGUI::KeyEventArgs& e );
};

} /* namespace CEGUIConsole */

} // namespace BFG

#endif /* CONSOLEEDITBOX_H_ */
