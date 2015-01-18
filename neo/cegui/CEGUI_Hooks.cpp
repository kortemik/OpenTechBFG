/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.
Copyright (C) 2014-2015 Daniel Gibson and Mikko Kortelainen (OpenTechBFG)

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

// hooks to pass engine events (key, mouse, ...) into CEGUI

#ifdef USE_CEGUI

#include <GL/glew.h> // for gl* in RenderGUIContexts()

#include "CEGUI_Hooks.h"

#ifdef __GNUC__ // gcc and clang - ignore warning about non-virtual destructors in cegui code

#pragma GCC diagnostic push // save old warning settings

#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"

#endif // __GNUC__

#include <CEGUI/CEGUI.h>
#include <CEGUI/InputEvent.h>
#include <CEGUI/RendererModules/OpenGL/GLRenderer.h>

#ifdef __GNUC__
#pragma GCC diagnostic pop // restore old warning settings
#endif // __GNUC__

using namespace CEGUI;

namespace // anon. namespace for helper functions and global state
{
	CEGUI::System* ceguiSys = NULL; // the CEGUI System Singleton, available after Init()
	double oldTimePulseSec = -1.0;  // the last "time pulse" in seconds. updated by Update() (or rather UpdateTimePulse())

	void UpdateTimePulse()
	{
		double newTimePulseSec = 0.001 * Sys_Milliseconds();

		ceguiSys->injectTimePulse(newTimePulseSec - oldTimePulseSec);

		oldTimePulseSec = newTimePulseSec;
	}

	void RenderGUIContexts()
	{
		GLint glProgId;
		GLint glVertexArray;
		GLint glArrayBuffer;

		// save the state
		glGetIntegerv(GL_CURRENT_PROGRAM, &glProgId);
		glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &glVertexArray);
		glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &glArrayBuffer);

		// set defaults for cegui
		glUseProgram(0);
		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glActiveTexture(GL_TEXTURE0);

		// let cegui render
		ceguiSys->renderAllGUIContexts();

		// restore state
		glActiveTexture(GL_TEXTURE0);
		glBindBuffer(GL_ARRAY_BUFFER, glArrayBuffer);
		glBindVertexArray(glVertexArray);
		glUseProgram(glProgId);
	}

	void initRenderer (void)
	{
		// create renderer

		OpenGLRenderer& myRenderer = OpenGLRenderer::create();
		myRenderer.enableExtraStateSettings(true);
		System::create( myRenderer );
	}

	void initResourceProvider (void)
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

	void initResourceGroups (void)
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
		{
			parser->setProperty("SchemaDefaultResourceGroup", "schemas");
		}
	}

	void initResources (void)
	{
		SchemeManager::getSingleton().createFromFile("TaharezLook.scheme");
		FontManager::getSingleton().createFromFile("DejaVuSans-10.font");

		System::getSingleton().getDefaultGUIContext().setDefaultFont( "DejaVuSans-10" );
		System::getSingleton().getDefaultGUIContext().getMouseCursor().setDefaultImage( "TaharezLook/MouseArrow" );
		System::getSingleton().getDefaultGUIContext().setDefaultTooltipType( "TaharezLook/Tooltip" );
	}

	void createWindow (void)
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

} //anon namespace

bool idCEGUI::Init()
{
	initRenderer();
	initResourceProvider();
	initResourceGroups();
	initResources();
	createWindow();

	ceguiSys = &(System::getSingleton());
	oldTimePulseSec = 0.001 * Sys_Milliseconds();

	return true;
}

void idCEGUI::NotifyDisplaySizeChanged(int width, int height)
{
	ceguiSys->notifyDisplaySizeChanged( Sizef(width, height) );
}

bool idCEGUI::InjectChar(uint32 utf32char)
{
	return ceguiSys->getDefaultGUIContext().injectChar(utf32char);
}

bool idCEGUI::InjectKeyDown(keyNum_t key)
{
	// Key::Scan is dinput keynums, and so is keyNum_t
	return ceguiSys->getDefaultGUIContext().injectKeyDown( static_cast<Key::Scan>(key) );
}

bool idCEGUI::InjectKeyUp(keyNum_t key)
{
	// Key::Scan is dinput keynums, and so is keyNum_t
	return ceguiSys->getDefaultGUIContext().injectKeyUp( static_cast<Key::Scan>(key) );
}

bool idCEGUI::InjectMousePosition(int x, int y)
{
	return ceguiSys->getDefaultGUIContext().injectMousePosition(x, y);
}

bool idCEGUI::InjectMouseButtonDown(int button)
{
	// luckily cegui enumerates the mouse buttons in the same way we do (it just supports less)
	if(button < MouseButtonCount)
	{
		return ceguiSys->getDefaultGUIContext().injectMouseButtonDown(static_cast<MouseButton>(button));
	}
	return false;
}

bool idCEGUI::InjectMouseButtonUp(int button)
{
	// luckily cegui enumerates the mouse buttons in the same way we do (it just supports less)
	if(button < MouseButtonCount)
	{
		return ceguiSys->getDefaultGUIContext().injectMouseButtonUp(static_cast<MouseButton>(button));
	}
	return false;
}

bool idCEGUI::InjectMouseWheel(int delta)
{
	// TODO: d3bfg uses int for the deltas, cegui uses float - do we need a factor?
	return ceguiSys->getDefaultGUIContext().injectMouseWheelChange(delta);
}

void idCEGUI::Update()
{
	UpdateTimePulse();

	RenderGUIContexts();
}

#endif // USE_CEGUI