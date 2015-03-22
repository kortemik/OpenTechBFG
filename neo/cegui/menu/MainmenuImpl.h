#pragma once

#ifndef MainmenuImpl_H_
#define MainmenuImpl_H_

#include "cegui/wrap_cegui.h"

namespace CEGUIMenu
{

class MainmenuImpl
{
public:
	MainmenuImpl();
	virtual ~MainmenuImpl();
	
	void setVisible( bool visible );
	bool isVisible();
	
private:
	void CreateCEGUIWindow();
	void RegisterHandlers();
	bool Handle_DemoMapClick( const CEGUI::EventArgs& );
	bool Handle_QuitClick( const CEGUI::EventArgs& );
	
};


} /* namespace CEGUIMenu */

#endif /* MainmenuImpl_H_ */