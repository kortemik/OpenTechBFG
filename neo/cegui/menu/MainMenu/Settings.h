/*
 * Settings.h
 *
 *  Created on: Apr 26, 2015
 *      Author: kordex
 */

#ifndef SETTINGS_H_
#define SETTINGS_H_

#include <cegui/menu/GameMenu.h>

namespace BFG
{

namespace CEGUIMenu
{

class Settings: public GameMenu
{
public:
	Settings();
	virtual ~Settings();
	
	virtual void init();
	virtual void destroy();
	
	
protected:
	virtual void LoadNestedWindows();
	virtual void RegisterHandlers();
	
private:
	void hide();

	GameMenu *advanced;
	GameMenu *audio;
	GameMenu *controls;
	GameMenu *options;
	GameMenu *video;

};

} /* namespace CEGUIMenu */

} // namespace BFG

#endif /* SETTINGS_H_ */
