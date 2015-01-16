/*
 * Renderer.cpp
 *
 *  Created on: Jan 16, 2015
 *      Author: kordex
 */

#include <SDL.h>
#include <GL/glew.h>

#include "CEGUI_SDLHooks.h"
#include "Renderer.h"

namespace CEGUIRenderer {

Renderer::Renderer()
{
	glGetIntegerv(GL_CURRENT_PROGRAM, &glProgId);
	glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &glVertexArray);
	glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &glArrayBuffer);

	glUseProgram(0);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glActiveTexture(GL_TEXTURE0);

}


void Renderer::draw()
{
	cegui::getInstance().injectTimePulse(SDL_GetTicks());
	cegui::getInstance().renderAllGUIContexts();
}

Renderer::~Renderer() {
	glActiveTexture(GL_TEXTURE0);
	glBindBuffer(GL_ARRAY_BUFFER, glArrayBuffer);
	glBindVertexArray(glVertexArray);
	glUseProgram(glProgId);
}

} /* namespace CEGUIRenderer */
