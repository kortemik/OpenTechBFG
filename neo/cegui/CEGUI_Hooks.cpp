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
#include "wrap_cegui.h"

#include <CEGUI/RendererModules/OpenGL/GLRenderer.h>

#include "console/Console.h"

#include "menu/MenuSystem.h"

namespace // anon. namespace for helper functions and global state
{
CEGUI::System* ceguiSys = NULL; // the CEGUI System Singleton, available after Init()
double oldTimePulseSec = -1.0;  // the last "time pulse" in seconds. updated by Update() (or rather UpdateTimePulse())

void UpdateTimePulse()
{
	double newTimePulseSec = 0.001 * Sys_Milliseconds();
	
	ceguiSys->injectTimePulse( newTimePulseSec - oldTimePulseSec );
	
	oldTimePulseSec = newTimePulseSec;
}

void RenderGUIContexts()
{
	GLint glProgId;
	GLint glVertexArray;
	GLint glArrayBuffer;
	
	// save the state
	glGetIntegerv( GL_CURRENT_PROGRAM, &glProgId );
	glGetIntegerv( GL_VERTEX_ARRAY_BINDING, &glVertexArray );
	glGetIntegerv( GL_ARRAY_BUFFER_BINDING, &glArrayBuffer );
	
	// set defaults for cegui
	glUseProgram( 0 );
	glBindVertexArray( 0 );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glActiveTexture( GL_TEXTURE0 );
	
	// let cegui render
	ceguiSys->renderAllGUIContexts();
	
	// restore state
	glActiveTexture( GL_TEXTURE0 );
	glBindBuffer( GL_ARRAY_BUFFER, glArrayBuffer );
	glBindVertexArray( glVertexArray );
	glUseProgram( glProgId );
}

void initSystem( void )
{
	// create renderer
	
	CEGUI::OpenGLRenderer& myRenderer = CEGUI::OpenGLRenderer::create();
	myRenderer.enableExtraStateSettings( true );
	CEGUI::System::create( myRenderer );
}

void initResourceProvider( void )
{
	if( idCEGUI::IsInitialized() )
	{
		CEGUI::DefaultResourceProvider* rp = static_cast<CEGUI::DefaultResourceProvider*>
											 ( CEGUI::System::getSingleton().getResourceProvider() );
		rp->setResourceGroupDirectory( "schemes", "base/cegui/schemes/" );
		rp->setResourceGroupDirectory( "imagesets", "base/cegui/imagesets/" );
		rp->setResourceGroupDirectory( "fonts", "base/cegui/fonts/" );
		rp->setResourceGroupDirectory( "layouts", "base/cegui/layouts/" );
		rp->setResourceGroupDirectory( "looknfeels", "base/cegui/looknfeel/" );
		rp->setResourceGroupDirectory( "lua_scripts", "base/cegui/lua_scripts/" );
		// This is only really needed if you are using Xerces and need to
		// specify the schemas location
		rp->setResourceGroupDirectory( "schemas", "base/cegui/xml_schemas/" );
	}
}

void initResourceGroups( void )
{
	if( idCEGUI::IsInitialized() )
	{
		// set the default resource groups to be used
		CEGUI::ImageManager::setImagesetDefaultResourceGroup( "imagesets" );
		CEGUI::Font::setDefaultResourceGroup( "fonts" );
		CEGUI::Scheme::setDefaultResourceGroup( "schemes" );
		CEGUI::WidgetLookManager::setDefaultResourceGroup( "looknfeels" );
		CEGUI::WindowManager::setDefaultResourceGroup( "layouts" );
		CEGUI::ScriptModule::setDefaultResourceGroup( "lua_scripts" );
		// setup default group for validation schemas
		CEGUI::XMLParser* parser = CEGUI::System::getSingleton().getXMLParser();
		if( parser->isPropertyPresent( "SchemaDefaultResourceGroup" ) )
		{
			parser->setProperty( "SchemaDefaultResourceGroup", "schemas" );
		}
	}
}

void initResources( void )
{
	if( idCEGUI::IsInitialized() )
	{
		CEGUI::SchemeManager::getSingleton().createFromFile( "TaharezLook.scheme" );
		CEGUI::FontManager::getSingleton().createFromFile( "DejaVuSans-10.font" );
		
		CEGUI::System::getSingleton().getDefaultGUIContext().setDefaultFont( "DejaVuSans-10" );
		CEGUI::System::getSingleton().getDefaultGUIContext().getMouseCursor().setDefaultImage( "TaharezLook/MouseArrow" );
		CEGUI::System::getSingleton().getDefaultGUIContext().setDefaultTooltipType( "TaharezLook/Tooltip" );
	}
}

void createWindow( void )
{
	if( idCEGUI::IsInitialized() )
	{
		CEGUI::WindowManager& wmgr = CEGUI::WindowManager::getSingleton();
		CEGUI::Window* myRoot = wmgr.createWindow( "DefaultWindow", "root" );
		CEGUI::System::getSingleton().getDefaultGUIContext().setRootWindow( myRoot );
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
}

// keyboard keys, mouse keys, mousewheel (TODO: really?)
bool HandleKeyEvent( const sysEvent_t& keyEvent )
{
	if( idCEGUI::IsInitialized() )
	{
		assert( keyEvent.evType == SE_KEY );
		
		keyNum_t keyNum = static_cast<keyNum_t>( keyEvent.evValue );
		bool pressed = keyEvent.evValue2;
		
		if( keyNum < K_JOY1 )
		{
			// Key::Scan is dinput keynums, and so is keyNum_t (at least for everything below K_JOY1)
			CEGUI::Key::Scan key = static_cast<CEGUI::Key::Scan>( keyEvent.evValue );
			
			if( pressed ) // 1 if pressed, 0 if released
			{
				return ceguiSys->getDefaultGUIContext().injectKeyDown( key );
			}
			else
			{
				return ceguiSys->getDefaultGUIContext().injectKeyUp( key );
			}
		}
		else if( keyNum >= K_MOUSE1 && keyNum < K_MOUSE1 + CEGUI::MouseButtonCount )
		{
			// K_MOUSE* are contiguous, so are CEGUI::MouseButton::*Button - and have the same order
			// (left, right, middle, X1, X2). CEGUI::LeftButton is 0.
			CEGUI::MouseButton button = static_cast<CEGUI::MouseButton>( keyNum - K_MOUSE1 );
			if( pressed )
			{
				return ceguiSys->getDefaultGUIContext().injectMouseButtonDown( button );
			}
			else
			{
				return ceguiSys->getDefaultGUIContext().injectMouseButtonUp( button );
			}
		}
	}
	return false;
}

CEGUIMenu::MenuSystem *ourMenuSystem;

void Startup()
{
	// all functions for console/menu/etc should be here
	// this will create a cegui console for idConsole to use
	console->Init();

	// MenuSystem
	ourMenuSystem = new CEGUIMenu::MenuSystem();
	ourMenuSystem->Init();
}

void Shutdown()
{
	// all functions for console/menu/etc should be here
	console->Shutdown();

	// MenuSystem
	ourMenuSystem->Destroy();
	delete ourMenuSystem;
}

} //anon namespace

bool idCEGUI::Init()
{
	initSystem();
	initResourceProvider();
	initResourceGroups();
	initResources();
	createWindow();
	
	if( IsInitialized() )
	{
		ceguiSys = &( CEGUI::System::getSingleton() );
		oldTimePulseSec = 0.001 * Sys_Milliseconds();
	}
	
	Startup();
	
	return true;
}

void idCEGUI::NotifyDisplaySizeChanged( int width, int height )
{
	if( IsInitialized() )
	{
		ceguiSys->notifyDisplaySizeChanged( CEGUI::Sizef( width, height ) );
	}
}

// inject a sys event
bool idCEGUI::InjectSysEvent( const sysEvent_t* event )
{
	if( IsInitialized() )
	{
		if( event == NULL )
		{
			assert( 0 ); // I think this shouldn't happen
			return false;
		}
		
		const sysEvent_t& ev = *event;
		
		switch( ev.evType )
		{
			case SE_KEY:
				return HandleKeyEvent( ev );
			case SE_MOUSE_ABSOLUTE:
				return ceguiSys->getDefaultGUIContext().injectMousePosition( ev.evValue, ev.evValue2 );
			case SE_CHAR:
				return ceguiSys->getDefaultGUIContext().injectChar( ev.evValue );
			case SE_MOUSE_LEAVE:
				// not sure why this is interesting (and mouse entering again not), but whatever..
				return ceguiSys->getDefaultGUIContext().injectMouseLeaves();
				
			default:
				break;
		}
	}
	return false;
}

bool idCEGUI::InjectMouseWheel( int delta )
{
	if( IsInitialized() )
	{
		// TODO: d3bfg uses int for the deltas, cegui uses float - do we need a factor?
		return ceguiSys->getDefaultGUIContext().injectMouseWheelChange( delta );
	}
	return false;
}

void idCEGUI::Update()
{
	UpdateTimePulse();
	
	RenderGUIContexts();
}

void idCEGUI::Destroy()
{
	if( IsInitialized() )
	{
		Shutdown();
		CEGUI::OpenGLRenderer* renderer = static_cast<CEGUI::OpenGLRenderer*>( CEGUI::System::getSingleton().getRenderer() );
		CEGUI::System::getSingleton().destroy();
		CEGUI::OpenGLRenderer::destroy( *renderer );
	}
}

bool idCEGUI::IsInitialized()
{
	// checks if cegui is up and running
	return CEGUI::System::getSingletonPtr() != NULL;
}
#endif // USE_CEGUI
