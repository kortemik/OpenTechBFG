/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.
Copyright (C) 2012 Robert Beckebans

This file is part of the Doom 3 BFG Edition GPL Source Code ("Doom 3 BFG Edition Source Code").

Doom 3 BFG Edition Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Doom 3 BFG Edition Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Doom 3 BFG Edition Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Doom 3 BFG Edition Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Doom 3 BFG Edition Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
 */

#include <GL/glew.h>

#include <GL/gl.h>
#include <GL/glu.h>

#include <CEGUI/CEGUI.h>
#include <CEGUI/InputEvent.h>
#include <CEGUI/RendererModules/OpenGL/GLRenderer.h>

#include <SDL.h>

#include <map>


#include "CEGUI_SDLHooks.h"

#include "console/Console.h"

using namespace CEGUI;

struct cegui::ceguiVars {
	std::map<SDLKey, CEGUI::Key::Scan> m_keymap;
};

void cegui::renderAllGUIContexts(void)
{
	System::getSingleton().renderAllGUIContexts();
}

bool cegui::injectTimePulse(Uint32 ticks)
{
	/*
	 * time pulse for cegui
	 */
	bool status;
	static double ceguiTLast = 0.001*static_cast<double>(ticks);
	double ceguiTNow = 0.001*ticks;
	status = System::getSingleton().injectTimePulse( float(ceguiTNow-ceguiTLast) );
	ceguiTLast = ceguiTNow;
	return status;
}

void cegui::notifyDisplaySizeChanged(int width, int height)
{
	System::getSingleton().notifyDisplaySizeChanged(CEGUI::Sizef(width, height));
}

bool cegui::injectKeyDown (SDLKey sym)
{
	return System::getSingleton().getDefaultGUIContext().injectKeyDown(ourVars->m_keymap[sym]);
}

bool cegui::injectKeyUp (SDLKey sym)
{
	return System::getSingleton().getDefaultGUIContext().injectKeyUp(ourVars->m_keymap[sym]);
}

bool cegui::injectChar (Uint16 unicode_char)
{
/* as for the character it's a litte more complicated.
 * we'll use for translated unicode value.
 * this is described in more detail below.
 */
	return System::getSingleton().getDefaultGUIContext().injectChar(unicode_char & 0x7F);
}

bool cegui::injectMousePosition(Uint16 x, Uint16 y) {
	return System::getSingleton().getDefaultGUIContext().injectMousePosition(static_cast<float>(x), static_cast<float>(y));
}

bool cegui::injectMouseButtonLeftDown(void)
{
	return System::getSingleton().getDefaultGUIContext().injectMouseButtonDown(CEGUI::LeftButton);
}
bool cegui::injectMouseButtonLeftUp(void)
{
	return System::getSingleton().getDefaultGUIContext().injectMouseButtonUp(CEGUI::LeftButton);
}
bool cegui::injectMouseButtonMiddleDown(void)
{
	return System::getSingleton().getDefaultGUIContext().injectMouseButtonDown(CEGUI::MiddleButton);
}
bool cegui::injectMouseButtonMiddleUp(void)
{
	return System::getSingleton().getDefaultGUIContext().injectMouseButtonUp(CEGUI::MiddleButton);
}
bool cegui::injectMouseButtonRightDown(void)
{
	return System::getSingleton().getDefaultGUIContext().injectMouseButtonDown(CEGUI::RightButton);
}
bool cegui::injectMouseButtonRightUp(void)
{
	return System::getSingleton().getDefaultGUIContext().injectMouseButtonUp(CEGUI::RightButton);
}

bool cegui::injectMouseWheelChange(float delta)
{
	return System::getSingleton().getDefaultGUIContext().injectMouseWheelChange(delta);
}


void cegui::initRenderer (void)
{
	// create renderer

	OpenGLRenderer& myRenderer = OpenGLRenderer::create();
	myRenderer.enableExtraStateSettings(true);
	System::create( myRenderer );
}

void cegui::initResourceProvider (void)
{
	DefaultResourceProvider* rp = static_cast<DefaultResourceProvider*>
	(CEGUI::System::getSingleton().getResourceProvider());
	rp->setResourceGroupDirectory("schemes", "base/cegui/schemes/");
	rp->setResourceGroupDirectory("imagesets", "base/cegui/imagesets/");
	rp->setResourceGroupDirectory("fonts", "base/cegui/fonts/");
	rp->setResourceGroupDirectory("layouts", "base/cegui/layouts/");
	rp->setResourceGroupDirectory("looknfeels", "base/cegui/looknfeel/");
	rp->setResourceGroupDirectory("lua_scripts", "base/cegui/lua_scripts/");
	// This is only really needed if you are using Xerces and need to
	// specify the schemas location
	rp->setResourceGroupDirectory("schemas", "base/cegui/xml_schemas/");
}

void cegui::initResourceGroups (void)
{
	// set the default resource groups to be used
	ImageManager::setImagesetDefaultResourceGroup("imagesets");
	Font::setDefaultResourceGroup("fonts");
	Scheme::setDefaultResourceGroup("schemes");
	WidgetLookManager::setDefaultResourceGroup("looknfeels");
	WindowManager::setDefaultResourceGroup("layouts");
	ScriptModule::setDefaultResourceGroup("lua_scripts");
	// setup default group for validation schemas
	XMLParser* parser = System::getSingleton().getXMLParser();
	if (parser->isPropertyPresent("SchemaDefaultResourceGroup"))
		parser->setProperty("SchemaDefaultResourceGroup", "schemas");
}

void cegui::initResources (void)
{
	SchemeManager::getSingleton().createFromFile("TaharezLook.scheme");
	FontManager::getSingleton().createFromFile("DejaVuSans-10.font");

	System::getSingleton().getDefaultGUIContext().setDefaultFont(
			"DejaVuSans-10");
	System::getSingleton().getDefaultGUIContext().getMouseCursor().setDefaultImage(
			"TaharezLook/MouseArrow");
	System::getSingleton().getDefaultGUIContext().setDefaultTooltipType(
			"TaharezLook/Tooltip");
}

void cegui::createWindow (void)
{
	WindowManager& wmgr = WindowManager::getSingleton();
	Window* myRoot = wmgr.createWindow("DefaultWindow", "root");
	System::getSingleton().getDefaultGUIContext().setRootWindow(myRoot);
	/*
	FrameWindow* fWnd = static_cast<FrameWindow*>(wmgr.createWindow(
			"TaharezLook/FrameWindow", "testWindow"));
	myRoot->addChild(fWnd);
	// position a quarter of the way in from the top-left of parent.
	fWnd->setPosition(UVector2(UDim(0.25f, 0.0f), UDim(0.25f, 0.0f)));
	// set size to be half the size of the parent
	fWnd->setSize(USize(UDim(0.5f, 0.0f), UDim(0.5f, 0.0f)));
	fWnd->setText("Hello World!");
	*/
}

void cegui::initializeKeyMap(void) {
	ourVars->m_keymap[SDLK_1] = Key::One;
	ourVars->m_keymap[SDLK_2] = Key::Two;
	ourVars->m_keymap[SDLK_3] = Key::Three;
	ourVars->m_keymap[SDLK_4] = Key::Four;
	ourVars->m_keymap[SDLK_5] = Key::Five;
	ourVars->m_keymap[SDLK_6] = Key::Six;
	ourVars->m_keymap[SDLK_7] = Key::Seven;
	ourVars->m_keymap[SDLK_8] = Key::Eight;
	ourVars->m_keymap[SDLK_9] = Key::Nine;
	ourVars->m_keymap[SDLK_0] = Key::Zero;

	ourVars->m_keymap[SDLK_q] = Key::Q;
	ourVars->m_keymap[SDLK_w] = Key::W;
	ourVars->m_keymap[SDLK_e] = Key::E;
	ourVars->m_keymap[SDLK_r] = Key::R;
	ourVars->m_keymap[SDLK_t] = Key::T;
	ourVars->m_keymap[SDLK_y] = Key::Y;
	ourVars->m_keymap[SDLK_u] = Key::U;
	ourVars->m_keymap[SDLK_i] = Key::I;
	ourVars->m_keymap[SDLK_o] = Key::O;
	ourVars->m_keymap[SDLK_p] = Key::P;
	ourVars->m_keymap[SDLK_a] = Key::A;
	ourVars->m_keymap[SDLK_s] = Key::S;
	ourVars->m_keymap[SDLK_d] = Key::D;
	ourVars->m_keymap[SDLK_f] = Key::F;
	ourVars->m_keymap[SDLK_g] = Key::G;
	ourVars->m_keymap[SDLK_h] = Key::H;
	ourVars->m_keymap[SDLK_j] = Key::J;
	ourVars->m_keymap[SDLK_k] = Key::K;
	ourVars->m_keymap[SDLK_l] = Key::L;
	ourVars->m_keymap[SDLK_z] = Key::Z;
	ourVars->m_keymap[SDLK_x] = Key::X;
	ourVars->m_keymap[SDLK_c] = Key::C;
	ourVars->m_keymap[SDLK_v] = Key::V;
	ourVars->m_keymap[SDLK_b] = Key::B;
	ourVars->m_keymap[SDLK_n] = Key::N;
	ourVars->m_keymap[SDLK_m] = Key::M;

	ourVars->m_keymap[SDLK_COMMA] = Key::Comma;
	ourVars->m_keymap[SDLK_PERIOD] = Key::Period;
	ourVars->m_keymap[SDLK_SLASH] = Key::Slash;
	ourVars->m_keymap[SDLK_BACKSLASH] = Key::Backslash;
	ourVars->m_keymap[SDLK_MINUS] = Key::Minus;
	ourVars->m_keymap[SDLK_EQUALS] = Key::Equals;
	ourVars->m_keymap[SDLK_SEMICOLON] = Key::Semicolon;
	ourVars->m_keymap[SDLK_LEFTBRACKET] = Key::LeftBracket;
	ourVars->m_keymap[SDLK_RIGHTBRACKET] = Key::RightBracket;
	ourVars->m_keymap[SDLK_QUOTE] = Key::Apostrophe;
	ourVars->m_keymap[SDLK_BACKQUOTE] = Key::Grave;

	ourVars->m_keymap[SDLK_RETURN] = Key::Return;
	ourVars->m_keymap[SDLK_SPACE] = Key::Space;
	ourVars->m_keymap[SDLK_BACKSPACE] = Key::Backspace;
	ourVars->m_keymap[SDLK_TAB] = Key::Tab;

	ourVars->m_keymap[SDLK_ESCAPE] = Key::Escape;
	ourVars->m_keymap[SDLK_PAUSE] = Key::Pause;
	ourVars->m_keymap[SDLK_SYSREQ] = Key::SysRq;
	ourVars->m_keymap[SDLK_POWER] = Key::Power;

	ourVars->m_keymap[SDLK_NUMLOCK] = Key::NumLock;
	ourVars->m_keymap[SDLK_SCROLLOCK] = Key::ScrollLock;

	ourVars->m_keymap[SDLK_F1] = Key::F1;
	ourVars->m_keymap[SDLK_F2] = Key::F2;
	ourVars->m_keymap[SDLK_F3] = Key::F3;
	ourVars->m_keymap[SDLK_F4] = Key::F4;
	ourVars->m_keymap[SDLK_F5] = Key::F5;
	ourVars->m_keymap[SDLK_F6] = Key::F6;
	ourVars->m_keymap[SDLK_F7] = Key::F7;
	ourVars->m_keymap[SDLK_F8] = Key::F8;
	ourVars->m_keymap[SDLK_F9] = Key::F9;
	ourVars->m_keymap[SDLK_F10] = Key::F10;
	ourVars->m_keymap[SDLK_F11] = Key::F11;
	ourVars->m_keymap[SDLK_F12] = Key::F12;
	ourVars->m_keymap[SDLK_F13] = Key::F13;
	ourVars->m_keymap[SDLK_F14] = Key::F14;
	ourVars->m_keymap[SDLK_F15] = Key::F15;

	ourVars->m_keymap[SDLK_LCTRL] = Key::LeftControl;
	ourVars->m_keymap[SDLK_LALT] = Key::LeftAlt;
	ourVars->m_keymap[SDLK_LSHIFT] = Key::LeftShift;
	ourVars->m_keymap[SDLK_LSUPER] = Key::LeftWindows;
	ourVars->m_keymap[SDLK_RCTRL] = Key::RightControl;
	ourVars->m_keymap[SDLK_RALT] = Key::RightAlt;
	ourVars->m_keymap[SDLK_RSHIFT] = Key::RightShift;
	ourVars->m_keymap[SDLK_RSUPER] = Key::RightWindows;
	ourVars->m_keymap[SDLK_MENU] = Key::AppMenu;

	ourVars->m_keymap[SDLK_KP0] = Key::Numpad0;
	ourVars->m_keymap[SDLK_KP1] = Key::Numpad1;
	ourVars->m_keymap[SDLK_KP2] = Key::Numpad2;
	ourVars->m_keymap[SDLK_KP3] = Key::Numpad3;
	ourVars->m_keymap[SDLK_KP4] = Key::Numpad4;
	ourVars->m_keymap[SDLK_KP5] = Key::Numpad5;
	ourVars->m_keymap[SDLK_KP6] = Key::Numpad6;
	ourVars->m_keymap[SDLK_KP7] = Key::Numpad7;
	ourVars->m_keymap[SDLK_KP8] = Key::Numpad8;
	ourVars->m_keymap[SDLK_KP9] = Key::Numpad9;
	ourVars->m_keymap[SDLK_KP_PERIOD] = Key::Decimal;
	ourVars->m_keymap[SDLK_KP_PLUS] = Key::Add;
	ourVars->m_keymap[SDLK_KP_MINUS] = Key::Subtract;
	ourVars->m_keymap[SDLK_KP_MULTIPLY] = Key::Multiply;
	ourVars->m_keymap[SDLK_KP_DIVIDE] = Key::Divide;
	ourVars->m_keymap[SDLK_KP_ENTER] = Key::NumpadEnter;

	ourVars->m_keymap[SDLK_UP] = Key::ArrowUp;
	ourVars->m_keymap[SDLK_LEFT] = Key::ArrowLeft;
	ourVars->m_keymap[SDLK_RIGHT] = Key::ArrowRight;
	ourVars->m_keymap[SDLK_DOWN] = Key::ArrowDown;

	ourVars->m_keymap[SDLK_HOME] = Key::Home;
	ourVars->m_keymap[SDLK_END] = Key::End;
	ourVars->m_keymap[SDLK_PAGEUP] = Key::PageUp;
	ourVars->m_keymap[SDLK_PAGEDOWN] = Key::PageDown;
	ourVars->m_keymap[SDLK_INSERT] = Key::Insert;
	ourVars->m_keymap[SDLK_DELETE] = Key::Delete;
}

cegui::cegui() : ourVars(new ceguiVars)
{
initializeKeyMap();
initRenderer();
initResourceProvider();
initResourceGroups();
initResources();
createWindow();
}
