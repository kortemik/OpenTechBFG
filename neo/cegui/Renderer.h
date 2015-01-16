/*
 * Renderer.h
 *
 *  Created on: Jan 16, 2015
 *      Author: kordex
 */

#ifndef RENDERER_H_
#define RENDERER_H_

namespace CEGUIRenderer {

class Renderer {
public:
	Renderer();
	virtual ~Renderer();

	void draw();
private:
	GLint glProgId;
	GLint glVertexArray;
	GLint glArrayBuffer;
};

} /* namespace CEGUIRenderer */

#endif /* RENDERER_H_ */
