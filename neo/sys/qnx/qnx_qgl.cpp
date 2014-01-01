/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.
Copyright (C) 2013 Vincent Simonetti

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
/*
** This file implements the operating system binding of GL to QGL function
** pointers.  When doing a port of Doom you must implement the following
** two functions:
**
** QGL_Init() - loads libraries, assigns function pointers, etc.
** QGL_Shutdown() - unloads libraries, NULLs function pointers
*/
#pragma hdrstop
#include "../../idlib/precompiled.h"

#include <dlfcn.h>
#include "qnx_local.h"
#include "../../renderer/tr_local.h"

// -----------------------------------
// Links
//
// Store a direct link to a GLES or EGL function due to the
// possibility that the function will be accessed through
// GLimp_ExtensionPointer and later QGL_GetSym because
// eglGetProcAddress does not support, nor is required,
// to return non-extension functions.
//
// Implemented as seen below to facilitate NOT direct linking
// which could break due to ID_HARDLINK or "extension" functions
// that are built in to GLES but were written as extension functions
// because desktop OpenGL doesn't have them otherwise.
// -----------------------------------

// what is efficiency?...

typedef struct {
	bool		eglLink;
	float		glVersion;
	const char	*name;
	void		*link;
} glSymLink_t;

#define GL_LINK_COUNT 282
static glSymLink_t glLinks[GL_LINK_COUNT];
static int glSymCount = 0;

static void glLinkSet( const char *name, void *link, bool isEgl, float version ) {
	// attempt to find the link
	for ( int i = 0; i < glSymCount; i++ ) {
		if ( glLinks[i].name && strcmp( glLinks[i].name, name ) == 0 ) {
			glLinks[i].eglLink = isEgl;
			glLinks[i].glVersion = version;
			glLinks[i].name = name;
			glLinks[i].link = link;
			return;
		}
	}
	// add the link if it doesn't exist
	for ( int i = 0; i < GL_LINK_COUNT; i++ ) {
		if ( glLinks[i].link == NULL ) {
			glLinks[i].eglLink = isEgl;
			glLinks[i].glVersion = version;
			glLinks[i].name = name;
			glLinks[i].link = link;
			glSymCount++;
			return;
		}
	}
	assert( 0 ); // should never reach here
}

static void *glLinkGet( const char *name, bool isEgl, float maxGLVersion ) {
	for ( int i = 0; i < glSymCount; i++ ) {
		if ( glLinks[i].name && glLinks[i].eglLink == isEgl &&
				( isEgl || glLinks[i].glVersion <= maxGLVersion ) && strcmp( glLinks[i].name, name ) == 0 ) {
			return glLinks[i].link;
		}
	}
	return NULL;
}

// -----------------------------------
// Definitions
// -----------------------------------

void ( APIENTRY * qeglDebugEnableLoggingFunc )(EGLBoolean logging);

//Not actual GLES commands (glReadBuffer exists on GLES 3.0 but not the way it's needed)
void ( APIENTRY * qglReadBuffer )(GLenum mode);
void ( APIENTRY * qglDrawBuffer )(GLenum mode);
void ( APIENTRY * qglReadPixels )(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid* pixels);
void ( APIENTRY * qglCopyTexImage2D )(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
void ( APIENTRY * qglCopyTexSubImage2D )(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);

#ifdef GL_ES_VERSION_3_0

void ( APIENTRY * qglCopyTexSubImage3D )(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);

#endif

#ifndef ID_GL_HARDLINK

EGLint ( APIENTRY * qeglGetError )(void);
EGLDisplay ( APIENTRY * qeglGetDisplay )(EGLNativeDisplayType display_id);
EGLBoolean ( APIENTRY * qeglInitialize )(EGLDisplay dpy, EGLint *major, EGLint *minor);
EGLBoolean ( APIENTRY * qeglTerminate )(EGLDisplay dpy);
const char * ( APIENTRY * qeglQueryString )(EGLDisplay dpy, EGLint name);
EGLBoolean ( APIENTRY * qeglGetConfigs )(EGLDisplay dpy, EGLConfig *configs, EGLint config_size, EGLint *num_config);
EGLBoolean ( APIENTRY * qeglChooseConfig )(EGLDisplay dpy, const EGLint *attrib_list, EGLConfig *configs, EGLint config_size, EGLint *num_config);
EGLBoolean ( APIENTRY * qeglGetConfigAttrib )(EGLDisplay dpy, EGLConfig config, EGLint attribute, EGLint *value);
EGLSurface ( APIENTRY * qeglCreateWindowSurface )(EGLDisplay dpy, EGLConfig config, EGLNativeWindowType win, const EGLint *attrib_list);
EGLSurface ( APIENTRY * qeglCreatePbufferSurface )(EGLDisplay dpy, EGLConfig config, const EGLint *attrib_list);
EGLSurface ( APIENTRY * qeglCreatePixmapSurface )(EGLDisplay dpy, EGLConfig config, EGLNativePixmapType pixmap, const EGLint *attrib_list);
EGLBoolean ( APIENTRY * qeglDestroySurface )(EGLDisplay dpy, EGLSurface surface);
EGLBoolean ( APIENTRY * qeglQuerySurface )(EGLDisplay dpy, EGLSurface surface, EGLint attribute, EGLint *value);
EGLBoolean ( APIENTRY * qeglBindAPI )(EGLenum api);
EGLenum ( APIENTRY * qeglQueryAPI )(void);
EGLBoolean ( APIENTRY * qeglWaitClient )(void);
EGLBoolean ( APIENTRY * qeglReleaseThread )(void);
EGLSurface ( APIENTRY * qeglCreatePbufferFromClientBuffer )(EGLDisplay dpy, EGLenum buftype, EGLClientBuffer buffer, EGLConfig config, const EGLint *attrib_list);
EGLBoolean ( APIENTRY * qeglSurfaceAttrib )(EGLDisplay dpy, EGLSurface surface, EGLint attribute, EGLint value);
EGLBoolean ( APIENTRY * qeglBindTexImage )(EGLDisplay dpy, EGLSurface surface, EGLint buffer);
EGLBoolean ( APIENTRY * qeglReleaseTexImage )(EGLDisplay dpy, EGLSurface surface, EGLint buffer);
EGLBoolean ( APIENTRY * qeglSwapInterval )(EGLDisplay dpy, EGLint interval);
EGLContext ( APIENTRY * qeglCreateContext )(EGLDisplay dpy, EGLConfig config, EGLContext share_context, const EGLint *attrib_list);
EGLBoolean ( APIENTRY * qeglDestroyContext )(EGLDisplay dpy, EGLContext ctx);
EGLBoolean ( APIENTRY * qeglMakeCurrent )(EGLDisplay dpy, EGLSurface draw, EGLSurface read, EGLContext ctx);
EGLContext ( APIENTRY * qeglGetCurrentContext )(void);
EGLSurface ( APIENTRY * qeglGetCurrentSurface )(EGLint readdraw);
EGLDisplay ( APIENTRY * qeglGetCurrentDisplay )(void);
EGLBoolean ( APIENTRY * qeglQueryContext )(EGLDisplay dpy, EGLContext ctx, EGLint attribute, EGLint *value);
EGLBoolean ( APIENTRY * qeglWaitGL )(void);
EGLBoolean ( APIENTRY * qeglWaitNative )(EGLint engine);
EGLBoolean ( APIENTRY * qeglSwapBuffers )(EGLDisplay dpy, EGLSurface surface);
EGLBoolean ( APIENTRY * qeglCopyBuffers )(EGLDisplay dpy, EGLSurface surface, EGLNativePixmapType target);
GLExtension_t ( APIENTRY * qeglGetProcAddress )(const char *procname);

void ( APIENTRY * qglActiveTexture )(GLenum texture);
//void ( APIENTRY * qglAttachShader )(GLuint program, GLuint shader);
//void ( APIENTRY * qglBindAttribLocation )(GLuint program, GLuint index, const GLchar* name);
void ( APIENTRY * qglBindBuffer )(GLenum target, GLuint buffer);
void ( APIENTRY * qglBindFramebuffer )(GLenum target, GLuint framebuffer);
void ( APIENTRY * qglBindRenderbuffer )(GLenum target, GLuint renderbuffer);
void ( APIENTRY * qglBindTexture )(GLenum target, GLuint texture);
void ( APIENTRY * qglBlendColor )(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
void ( APIENTRY * qglBlendEquation )(GLenum mode);
void ( APIENTRY * qglBlendEquationSeparate )(GLenum modeRGB, GLenum modeAlpha);
void ( APIENTRY * qglBlendFunc )(GLenum sfactor, GLenum dfactor);
void ( APIENTRY * qglBlendFuncSeparate )(GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha);
void ( APIENTRY * qglBufferData )(GLenum target, GLsizeiptr size, const GLvoid* data, GLenum usage);
void ( APIENTRY * qglBufferSubData )(GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid* data);
GLenum ( APIENTRY * qglCheckFramebufferStatus )(GLenum target);
void ( APIENTRY * qglClear )(GLbitfield mask);
void ( APIENTRY * qglClearColor )(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
void ( APIENTRY * qglClearDepth )(GLclampd depth);
void ( APIENTRY * qglClearStencil )(GLint s);
void ( APIENTRY * qglColorMask )(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
//void ( APIENTRY * qglCompileShader )(GLuint shader);
void ( APIENTRY * qglCompressedTexImage2D )(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid* data);
void ( APIENTRY * qglCompressedTexSubImage2D )(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid* data);
//void ( APIENTRY * qglCopyTexImage2D )(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
//void ( APIENTRY * qglCopyTexSubImage2D )(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
//GLuint ( APIENTRY * qglCreateProgram )(void);
//GLuint ( APIENTRY * qglCreateShader )(GLenum type);
void ( APIENTRY * qglCullFace )(GLenum mode);
void ( APIENTRY * qglDeleteBuffers )(GLsizei n, const GLuint* buffers);
void ( APIENTRY * qglDeleteFramebuffers )(GLsizei n, const GLuint* framebuffers);
//void ( APIENTRY * qglDeleteProgram )(GLuint program);
void ( APIENTRY * qglDeleteRenderbuffers )(GLsizei n, const GLuint* renderbuffers);
//void ( APIENTRY * qglDeleteShader )(GLuint shader);
void ( APIENTRY * qglDeleteTextures )(GLsizei n, const GLuint* textures);
void ( APIENTRY * qglDepthFunc )(GLenum func);
void ( APIENTRY * qglDepthMask )(GLboolean flag);
void ( APIENTRY * qglDepthRange )(GLclampd n, GLclampd f);
//void ( APIENTRY * qglDetachShader )(GLuint program, GLuint shader);
void ( APIENTRY * qglDisable )(GLenum cap);
void ( APIENTRY * qglDisableVertexAttribArray )(GLuint index);
void ( APIENTRY * qglDrawArrays )(GLenum mode, GLint first, GLsizei count);
void ( APIENTRY * qglDrawElements )(GLenum mode, GLsizei count, GLenum type, const GLvoid* indices);
void ( APIENTRY * qglEnable )(GLenum cap);
void ( APIENTRY * qglEnableVertexAttribArray )(GLuint index);
void ( APIENTRY * qglFinish )(void);
void ( APIENTRY * qglFlush )(void);
void ( APIENTRY * qglFramebufferRenderbuffer )(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
void ( APIENTRY * qglFramebufferTexture2D )(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
void ( APIENTRY * qglFrontFace )(GLenum mode);
void ( APIENTRY * qglGenBuffers )(GLsizei n, GLuint* buffers);
void ( APIENTRY * qglGenerateMipmap )(GLenum target);
void ( APIENTRY * qglGenFramebuffers )(GLsizei n, GLuint* framebuffers);
void ( APIENTRY * qglGenRenderbuffers )(GLsizei n, GLuint* renderbuffers);
void ( APIENTRY * qglGenTextures )(GLsizei n, GLuint* textures);
void ( APIENTRY * qglGetActiveAttrib )(GLuint program, GLuint index, GLsizei bufsize, GLsizei* length, GLint* size, GLenum* type, GLchar* name);
void ( APIENTRY * qglGetActiveUniform )(GLuint program, GLuint index, GLsizei bufsize, GLsizei* length, GLint* size, GLenum* type, GLchar* name);
void ( APIENTRY * qglGetAttachedShaders )(GLuint program, GLsizei maxcount, GLsizei* count, GLuint* shaders);
GLint ( APIENTRY * qglGetAttribLocation )(GLuint program, const GLchar* name);
void ( APIENTRY * qglGetBooleanv )(GLenum pname, GLboolean* params);
void ( APIENTRY * qglGetBufferParameteriv )(GLenum target, GLenum pname, GLint* params);
GLenum ( APIENTRY * qglGetError )(void);
void ( APIENTRY * qglGetFloatv )(GLenum pname, GLfloat* params);
void ( APIENTRY * qglGetFramebufferAttachmentParameteriv )(GLenum target, GLenum attachment, GLenum pname, GLint* params);
void ( APIENTRY * qglGetIntegerv )(GLenum pname, GLint* params);
//void ( APIENTRY * qglGetProgramiv )(GLuint program, GLenum pname, GLint* params);
//void ( APIENTRY * qglGetProgramInfoLog )(GLuint program, GLsizei bufsize, GLsizei* length, GLchar* infolog);
void ( APIENTRY * qglGetRenderbufferParameteriv )(GLenum target, GLenum pname, GLint* params);
//void ( APIENTRY * qglGetShaderiv )(GLuint shader, GLenum pname, GLint* params);
//void ( APIENTRY * qglGetShaderInfoLog )(GLuint shader, GLsizei bufsize, GLsizei* length, GLchar* infolog);
void ( APIENTRY * qglGetShaderPrecisionFormat )(GLenum shadertype, GLenum precisiontype, GLint* range, GLint* precision);
void ( APIENTRY * qglGetShaderSource )(GLuint shader, GLsizei bufsize, GLsizei* length, GLchar* source);
const GLubyte* ( APIENTRY * qglGetString )(GLenum name);
void ( APIENTRY * qglGetTexParameterfv )(GLenum target, GLenum pname, GLfloat* params);
void ( APIENTRY * qglGetTexParameteriv )(GLenum target, GLenum pname, GLint* params);
void ( APIENTRY * qglGetUniformfv )(GLuint program, GLint location, GLfloat* params);
void ( APIENTRY * qglGetUniformiv )(GLuint program, GLint location, GLint* params);
//GLint ( APIENTRY * qglGetUniformLocation )(GLuint program, const GLchar* name);
void ( APIENTRY * qglGetVertexAttribfv )(GLuint index, GLenum pname, GLfloat* params);
void ( APIENTRY * qglGetVertexAttribiv )(GLuint index, GLenum pname, GLint* params);
void ( APIENTRY * qglGetVertexAttribPointerv )(GLuint index, GLenum pname, GLvoid** pointer);
void ( APIENTRY * qglHint )(GLenum target, GLenum mode);
GLboolean ( APIENTRY * qglIsBuffer )(GLuint buffer);
GLboolean ( APIENTRY * qglIsEnabled )(GLenum cap);
GLboolean ( APIENTRY * qglIsFramebuffer )(GLuint framebuffer);
GLboolean ( APIENTRY * qglIsProgram )(GLuint program);
GLboolean ( APIENTRY * qglIsRenderbuffer )(GLuint renderbuffer);
GLboolean ( APIENTRY * qglIsShader )(GLuint shader);
GLboolean ( APIENTRY * qglIsTexture )(GLuint texture);
void ( APIENTRY * qglLineWidth )(GLfloat width);
//void ( APIENTRY * qglLinkProgram )(GLuint program);
void ( APIENTRY * qglPixelStorei )(GLenum pname, GLint param);
void ( APIENTRY * qglPolygonOffset )(GLfloat factor, GLfloat units);
//void ( APIENTRY * qglReadPixels )(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid* pixels);
void ( APIENTRY * qglReleaseShaderCompiler )(void);
void ( APIENTRY * qglRenderbufferStorage )(GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
void ( APIENTRY * qglSampleCoverage )(GLfloat value, GLboolean invert);
void ( APIENTRY * qglScissor )(GLint x, GLint y, GLsizei width, GLsizei height);
void ( APIENTRY * qglShaderBinary )(GLsizei n, const GLuint* shaders, GLenum binaryformat, const GLvoid* binary, GLsizei length);
//void ( APIENTRY * qglShaderSource )(GLuint shader, GLsizei count, const GLchar* const* string, const GLint* length);
void ( APIENTRY * qglStencilFunc )(GLenum func, GLint ref, GLuint mask);
//void ( APIENTRY * qglStencilFuncSeparate )(GLenum face, GLenum func, GLint ref, GLuint mask);
void ( APIENTRY * qglStencilMask )(GLuint mask);
void ( APIENTRY * qglStencilMaskSeparate )(GLenum face, GLuint mask);
void ( APIENTRY * qglStencilOp )(GLenum fail, GLenum zfail, GLenum zpass);
//void ( APIENTRY * qglStencilOpSeparate )(GLenum face, GLenum fail, GLenum zfail, GLenum zpass);
void ( APIENTRY * qglTexImage2D )(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid* pixels);
void ( APIENTRY * qglTexParameterf )(GLenum target, GLenum pname, GLfloat param);
void ( APIENTRY * qglTexParameterfv )(GLenum target, GLenum pname, const GLfloat* params);
void ( APIENTRY * qglTexParameteri )(GLenum target, GLenum pname, GLint param);
void ( APIENTRY * qglTexParameteriv )(GLenum target, GLenum pname, const GLint* params);
void ( APIENTRY * qglTexSubImage2D )(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid* pixels);
void ( APIENTRY * qglUniform1f )(GLint location, GLfloat x);
void ( APIENTRY * qglUniform1fv )(GLint location, GLsizei count, const GLfloat* v);
//void ( APIENTRY * qglUniform1i )(GLint location, GLint x);
void ( APIENTRY * qglUniform1iv )(GLint location, GLsizei count, const GLint* v);
void ( APIENTRY * qglUniform2f )(GLint location, GLfloat x, GLfloat y);
void ( APIENTRY * qglUniform2fv )(GLint location, GLsizei count, const GLfloat* v);
void ( APIENTRY * qglUniform2i )(GLint location, GLint x, GLint y);
void ( APIENTRY * qglUniform2iv )(GLint location, GLsizei count, const GLint* v);
void ( APIENTRY * qglUniform3f )(GLint location, GLfloat x, GLfloat y, GLfloat z);
void ( APIENTRY * qglUniform3fv )(GLint location, GLsizei count, const GLfloat* v);
void ( APIENTRY * qglUniform3i )(GLint location, GLint x, GLint y, GLint z);
void ( APIENTRY * qglUniform3iv )(GLint location, GLsizei count, const GLint* v);
void ( APIENTRY * qglUniform4f )(GLint location, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
//void ( APIENTRY * qglUniform4fv )(GLint location, GLsizei count, const GLfloat* v);
void ( APIENTRY * qglUniform4i )(GLint location, GLint x, GLint y, GLint z, GLint w);
void ( APIENTRY * qglUniform4iv )(GLint location, GLsizei count, const GLint* v);
void ( APIENTRY * qglUniformMatrix2fv )(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
void ( APIENTRY * qglUniformMatrix3fv )(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
void ( APIENTRY * qglUniformMatrix4fv )(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
//void ( APIENTRY * qglUseProgram )(GLuint program);
void ( APIENTRY * qglValidateProgram )(GLuint program);
void ( APIENTRY * qglVertexAttrib1f )(GLuint indx, GLfloat x);
void ( APIENTRY * qglVertexAttrib1fv )(GLuint indx, const GLfloat* values);
void ( APIENTRY * qglVertexAttrib2f )(GLuint indx, GLfloat x, GLfloat y);
void ( APIENTRY * qglVertexAttrib2fv )(GLuint indx, const GLfloat* values);
void ( APIENTRY * qglVertexAttrib3f )(GLuint indx, GLfloat x, GLfloat y, GLfloat z);
void ( APIENTRY * qglVertexAttrib3fv )(GLuint indx, const GLfloat* values);
void ( APIENTRY * qglVertexAttrib4f )(GLuint indx, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
void ( APIENTRY * qglVertexAttrib4fv )(GLuint indx, const GLfloat* values);
void ( APIENTRY * qglVertexAttribPointer )(GLuint indx, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* ptr);
void ( APIENTRY * qglViewport )(GLint x, GLint y, GLsizei width, GLsizei height);

#ifdef GL_ES_VERSION_3_0

//void ( APIENTRY * qglReadBuffer )(GLenum mode);
void ( APIENTRY * qglDrawRangeElements )(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid* indices);
void ( APIENTRY * qglTexImage3D )(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid* pixels);
void ( APIENTRY * qglTexSubImage3D )(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid* pixels);
//void ( APIENTRY * qglCopyTexSubImage3D )(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);
void ( APIENTRY * qglCompressedTexImage3D )(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid* data);
void ( APIENTRY * qglCompressedTexSubImage3D )(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid* data);
void ( APIENTRY * qglGenQueries )(GLsizei n, GLuint* ids);
void ( APIENTRY * qglDeleteQueries )(GLsizei n, const GLuint* ids);
GLboolean ( APIENTRY * qglIsQuery )(GLuint id);
void ( APIENTRY * qglBeginQuery )(GLenum target, GLuint id);
void ( APIENTRY * qglEndQuery )(GLenum target);
void ( APIENTRY * qglGetQueryiv )(GLenum target, GLenum pname, GLint* params);
void ( APIENTRY * qglGetQueryObjectuiv )(GLuint id, GLenum pname, GLuint* params);
GLboolean ( APIENTRY * qglUnmapBuffer )(GLenum target);
void ( APIENTRY * qglGetBufferPointerv )(GLenum target, GLenum pname, GLvoid** params);
void ( APIENTRY * qglDrawBuffers )(GLsizei n, const GLenum* bufs);
void ( APIENTRY * qglUniformMatrix2x3fv )(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
void ( APIENTRY * qglUniformMatrix3x2fv )(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
void ( APIENTRY * qglUniformMatrix2x4fv )(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
void ( APIENTRY * qglUniformMatrix4x2fv )(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
void ( APIENTRY * qglUniformMatrix3x4fv )(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
void ( APIENTRY * qglUniformMatrix4x3fv )(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
void ( APIENTRY * qglBlitFramebuffer )(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);
void ( APIENTRY * qglRenderbufferStorageMultisample )(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);
void ( APIENTRY * qglFramebufferTextureLayer )(GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer);
//GLvoid* ( APIENTRY * qglMapBufferRange )(GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access);
void ( APIENTRY * qglFlushMappedBufferRange )(GLenum target, GLintptr offset, GLsizeiptr length);
//void ( APIENTRY * qglBindVertexArray )(GLuint array);
//void ( APIENTRY * qglDeleteVertexArrays )(GLsizei n, const GLuint* arrays);
//void ( APIENTRY * qglGenVertexArrays )(GLsizei n, GLuint* arrays);
GLboolean ( APIENTRY * qglIsVertexArray )(GLuint array);
void ( APIENTRY * qglGetIntegeri_v )(GLenum target, GLuint index, GLint* data);
void ( APIENTRY * qglBeginTransformFeedback )(GLenum primitiveMode);
void ( APIENTRY * qglEndTransformFeedback )(void);
//void ( APIENTRY * qglBindBufferRange )(GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size);
void ( APIENTRY * qglBindBufferBase )(GLenum target, GLuint index, GLuint buffer);
void ( APIENTRY * qglTransformFeedbackVaryings )(GLuint program, GLsizei count, const GLchar* const* varyings, GLenum bufferMode);
void ( APIENTRY * qglGetTransformFeedbackVarying )(GLuint program, GLuint index, GLsizei bufSize, GLsizei* length, GLsizei* size, GLenum* type, GLchar* name);
void ( APIENTRY * qglVertexAttribIPointer )(GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid* pointer);
void ( APIENTRY * qglGetVertexAttribIiv )(GLuint index, GLenum pname, GLint* params);
void ( APIENTRY * qglGetVertexAttribIuiv )(GLuint index, GLenum pname, GLuint* params);
void ( APIENTRY * qglVertexAttribI4i )(GLuint index, GLint x, GLint y, GLint z, GLint w);
void ( APIENTRY * qglVertexAttribI4ui )(GLuint index, GLuint x, GLuint y, GLuint z, GLuint w);
void ( APIENTRY * qglVertexAttribI4iv )(GLuint index, const GLint* v);
void ( APIENTRY * qglVertexAttribI4uiv )(GLuint index, const GLuint* v);
void ( APIENTRY * qglGetUniformuiv )(GLuint program, GLint location, GLuint* params);
GLint ( APIENTRY * qglGetFragDataLocation )(GLuint program, const GLchar *name);
void ( APIENTRY * qglUniform1ui )(GLint location, GLuint v0);
void ( APIENTRY * qglUniform2ui )(GLint location, GLuint v0, GLuint v1);
void ( APIENTRY * qglUniform3ui )(GLint location, GLuint v0, GLuint v1, GLuint v2);
void ( APIENTRY * qglUniform4ui )(GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3);
void ( APIENTRY * qglUniform1uiv )(GLint location, GLsizei count, const GLuint* value);
void ( APIENTRY * qglUniform2uiv )(GLint location, GLsizei count, const GLuint* value);
void ( APIENTRY * qglUniform3uiv )(GLint location, GLsizei count, const GLuint* value);
void ( APIENTRY * qglUniform4uiv )(GLint location, GLsizei count, const GLuint* value);
void ( APIENTRY * qglClearBufferiv )(GLenum buffer, GLint drawbuffer, const GLint* value);
void ( APIENTRY * qglClearBufferuiv )(GLenum buffer, GLint drawbuffer, const GLuint* value);
void ( APIENTRY * qglClearBufferfv )(GLenum buffer, GLint drawbuffer, const GLfloat* value);
void ( APIENTRY * qglClearBufferfi )(GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil);
//const GLubyte* ( APIENTRY * qglGetStringi )(GLenum name, GLuint index);
void ( APIENTRY * qglCopyBufferSubData )(GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size);
void ( APIENTRY * qglGetUniformIndices )(GLuint program, GLsizei uniformCount, const GLchar* const* uniformNames, GLuint* uniformIndices);
void ( APIENTRY * qglGetActiveUniformsiv )(GLuint program, GLsizei uniformCount, const GLuint* uniformIndices, GLenum pname, GLint* params);
//GLuint ( APIENTRY * qglGetUniformBlockIndex )(GLuint program, const GLchar* uniformBlockName);
void ( APIENTRY * qglGetActiveUniformBlockiv )(GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint* params);
void ( APIENTRY * qglGetActiveUniformBlockName )(GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei* length, GLchar* uniformBlockName);
//void ( APIENTRY * qglUniformBlockBinding )(GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding);
void ( APIENTRY * qglDrawArraysInstanced )(GLenum mode, GLint first, GLsizei count, GLsizei instanceCount);
void ( APIENTRY * qglDrawElementsInstanced )(GLenum mode, GLsizei count, GLenum type, const GLvoid* indices, GLsizei instanceCount);
//GLsync ( APIENTRY * qglFenceSync )(GLenum condition, GLbitfield flags);
//GLboolean ( APIENTRY * qglIsSync )(GLsync sync);
//void ( APIENTRY * qglDeleteSync )(GLsync sync);
//GLenum ( APIENTRY * qglClientWaitSync )(GLsync sync, GLbitfield flags, GLuint64 timeout);
void ( APIENTRY * qglWaitSync )(GLsync sync, GLbitfield flags, GLuint64 timeout);
void ( APIENTRY * qglGetInteger64v )(GLenum pname, GLint64* params);
void ( APIENTRY * qglGetSynciv )(GLsync sync, GLenum pname, GLsizei bufSize, GLsizei* length, GLint* values);
void ( APIENTRY * qglGetInteger64i_v )(GLenum target, GLuint index, GLint64* data);
void ( APIENTRY * qglGetBufferParameteri64v )(GLenum target, GLenum pname, GLint64* params);
void ( APIENTRY * qglGenSamplers )(GLsizei count, GLuint* samplers);
void ( APIENTRY * qglDeleteSamplers )(GLsizei count, const GLuint* samplers);
GLboolean ( APIENTRY * qglIsSampler )(GLuint sampler);
void ( APIENTRY * qglBindSampler )(GLuint unit, GLuint sampler);
void ( APIENTRY * qglSamplerParameteri )(GLuint sampler, GLenum pname, GLint param);
void ( APIENTRY * qglSamplerParameteriv )(GLuint sampler, GLenum pname, const GLint* param);
void ( APIENTRY * qglSamplerParameterf )(GLuint sampler, GLenum pname, GLfloat param);
void ( APIENTRY * qglSamplerParameterfv )(GLuint sampler, GLenum pname, const GLfloat* param);
void ( APIENTRY * qglGetSamplerParameteriv )(GLuint sampler, GLenum pname, GLint* params);
void ( APIENTRY * qglGetSamplerParameterfv )(GLuint sampler, GLenum pname, GLfloat* params);
void ( APIENTRY * qglVertexAttribDivisor )(GLuint index, GLuint divisor);
void ( APIENTRY * qglBindTransformFeedback )(GLenum target, GLuint id);
void ( APIENTRY * qglDeleteTransformFeedbacks )(GLsizei n, const GLuint* ids);
void ( APIENTRY * qglGenTransformFeedbacks )(GLsizei n, GLuint* ids);
GLboolean ( APIENTRY * qglIsTransformFeedback )(GLuint id);
void ( APIENTRY * qglPauseTransformFeedback )(void);
void ( APIENTRY * qglResumeTransformFeedback )(void);
void ( APIENTRY * qglGetProgramBinary )(GLuint program, GLsizei bufSize, GLsizei* length, GLenum* binaryFormat, GLvoid* binary);
void ( APIENTRY * qglProgramBinary )(GLuint program, GLenum binaryFormat, const GLvoid* binary, GLsizei length);
//void ( APIENTRY * qglProgramParameteri )(GLuint program, GLenum pname, GLint value);
void ( APIENTRY * qglInvalidateFramebuffer )(GLenum target, GLsizei numAttachments, const GLenum* attachments);
void ( APIENTRY * qglInvalidateSubFramebuffer )(GLenum target, GLsizei numAttachments, const GLenum* attachments, GLint x, GLint y, GLsizei width, GLsizei height);
void ( APIENTRY * qglTexStorage2D )(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height);
void ( APIENTRY * qglTexStorage3D )(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth);
void ( APIENTRY * qglGetInternalformativ )(GLenum target, GLenum internalformat, GLenum pname, GLsizei bufSize, GLint* params);

#endif

#endif

#if 0 //This is never used. Ignore it to save executable space

typedef struct {
	GLenum	e;
	const char *name;
} glEnumName_t;

#define	DEF(x) { x, #x },

glEnumName_t	glEnumNames[] = {
	/* OpenGL ES 2.0 */

	/* ClearBufferMask */
	DEF(GL_DEPTH_BUFFER_BIT)
	DEF(GL_STENCIL_BUFFER_BIT)
	DEF(GL_COLOR_BUFFER_BIT)

	/* Boolean */
	DEF(GL_FALSE)
	DEF(GL_TRUE)

	/* BeginMode */
	DEF(GL_POINTS)
	DEF(GL_LINES)
	DEF(GL_LINE_LOOP)
	DEF(GL_LINE_STRIP)
	DEF(GL_TRIANGLES)
	DEF(GL_TRIANGLE_STRIP)
	DEF(GL_TRIANGLE_FAN)

	/* BlendingFactorDest */
	DEF(GL_ZERO)
	DEF(GL_ONE)
	DEF(GL_SRC_COLOR)
	DEF(GL_ONE_MINUS_SRC_COLOR)
	DEF(GL_SRC_ALPHA)
	DEF(GL_ONE_MINUS_SRC_ALPHA)
	DEF(GL_DST_ALPHA)
	DEF(GL_ONE_MINUS_DST_ALPHA)

	/* BlendingFactorSrc */
	/*      GL_ZERO */
	/*      GL_ONE */
	DEF(GL_DST_COLOR)
	DEF(GL_ONE_MINUS_DST_COLOR)
	DEF(GL_SRC_ALPHA_SATURATE)
	/*      GL_SRC_ALPHA */
	/*      GL_ONE_MINUS_SRC_ALPHA */
	/*      GL_DST_ALPHA */
	/*      GL_ONE_MINUS_DST_ALPHA */

	/* BlendEquationSeparate */
	DEF(GL_FUNC_ADD)
	DEF(GL_BLEND_EQUATION)
	DEF(GL_BLEND_EQUATION_RGB)
	DEF(GL_BLEND_EQUATION_ALPHA)

	/* BlendSubtract */
	DEF(GL_FUNC_SUBTRACT)
	DEF(GL_FUNC_REVERSE_SUBTRACT)

	/* Separate Blend Functions */
	DEF(GL_BLEND_DST_RGB)
	DEF(GL_BLEND_SRC_RGB)
	DEF(GL_BLEND_DST_ALPHA)
	DEF(GL_BLEND_SRC_ALPHA)
	DEF(GL_CONSTANT_COLOR)
	DEF(GL_ONE_MINUS_CONSTANT_COLOR)
	DEF(GL_CONSTANT_ALPHA)
	DEF(GL_ONE_MINUS_CONSTANT_ALPHA)
	DEF(GL_BLEND_COLOR)

	/* Buffer Objects */
	DEF(GL_ARRAY_BUFFER)
	DEF(GL_ELEMENT_ARRAY_BUFFER)
	DEF(GL_ARRAY_BUFFER_BINDING)
	DEF(GL_ELEMENT_ARRAY_BUFFER_BINDING)

	DEF(GL_STREAM_DRAW)
	DEF(GL_STATIC_DRAW)
	DEF(GL_DYNAMIC_DRAW)

	DEF(GL_BUFFER_SIZE)
	DEF(GL_BUFFER_USAGE)

	DEF(GL_CURRENT_VERTEX_ATTRIB)

	/* CullFaceMode */
	DEF(GL_FRONT)
	DEF(GL_BACK)
	DEF(GL_FRONT_AND_BACK)

	/* DepthFunction */
	/*      GL_NEVER */
	/*      GL_LESS */
	/*      GL_EQUAL */
	/*      GL_LEQUAL */
	/*      GL_GREATER */
	/*      GL_NOTEQUAL */
	/*      GL_GEQUAL */
	/*      GL_ALWAYS */

	/* EnableCap */
	DEF(GL_TEXTURE_2D)
	DEF(GL_CULL_FACE)
	DEF(GL_BLEND)
	DEF(GL_DITHER)
	DEF(GL_STENCIL_TEST)
	DEF(GL_DEPTH_TEST)
	DEF(GL_SCISSOR_TEST)
	DEF(GL_POLYGON_OFFSET_FILL)
	DEF(GL_SAMPLE_ALPHA_TO_COVERAGE)
	DEF(GL_SAMPLE_COVERAGE)

	/* ErrorCode */
	DEF(GL_NO_ERROR)
	DEF(GL_INVALID_ENUM)
	DEF(GL_INVALID_VALUE)
	DEF(GL_INVALID_OPERATION)
	DEF(GL_OUT_OF_MEMORY)

	/* FrontFaceDirection */
	DEF(GL_CW)
	DEF(GL_CCW)

	/* GetPName */
	DEF(GL_LINE_WIDTH)
	DEF(GL_ALIASED_POINT_SIZE_RANGE)
	DEF(GL_ALIASED_LINE_WIDTH_RANGE)
	DEF(GL_CULL_FACE_MODE)
	DEF(GL_FRONT_FACE)
	DEF(GL_DEPTH_RANGE)
	DEF(GL_DEPTH_WRITEMASK)
	DEF(GL_DEPTH_CLEAR_VALUE)
	DEF(GL_DEPTH_FUNC)
	DEF(GL_STENCIL_CLEAR_VALUE)
	DEF(GL_STENCIL_FUNC)
	DEF(GL_STENCIL_FAIL)
	DEF(GL_STENCIL_PASS_DEPTH_FAIL)
	DEF(GL_STENCIL_PASS_DEPTH_PASS)
	DEF(GL_STENCIL_REF)
	DEF(GL_STENCIL_VALUE_MASK)
	DEF(GL_STENCIL_WRITEMASK)
	DEF(GL_STENCIL_BACK_FUNC)
	DEF(GL_STENCIL_BACK_FAIL)
	DEF(GL_STENCIL_BACK_PASS_DEPTH_FAIL)
	DEF(GL_STENCIL_BACK_PASS_DEPTH_PASS)
	DEF(GL_STENCIL_BACK_REF)
	DEF(GL_STENCIL_BACK_VALUE_MASK)
	DEF(GL_STENCIL_BACK_WRITEMASK)
	DEF(GL_VIEWPORT)
	DEF(GL_SCISSOR_BOX)
	/*      GL_SCISSOR_TEST */
	DEF(GL_COLOR_CLEAR_VALUE)
	DEF(GL_COLOR_WRITEMASK)
	DEF(GL_UNPACK_ALIGNMENT)
	DEF(GL_PACK_ALIGNMENT)
	DEF(GL_MAX_TEXTURE_SIZE)
	DEF(GL_MAX_VIEWPORT_DIMS)
	DEF(GL_SUBPIXEL_BITS)
	DEF(GL_RED_BITS)
	DEF(GL_GREEN_BITS)
	DEF(GL_BLUE_BITS)
	DEF(GL_ALPHA_BITS)
	DEF(GL_DEPTH_BITS)
	DEF(GL_STENCIL_BITS)
	DEF(GL_POLYGON_OFFSET_UNITS)
	/*      GL_POLYGON_OFFSET_FILL */
	DEF(GL_POLYGON_OFFSET_FACTOR)
	DEF(GL_TEXTURE_BINDING_2D)
	DEF(GL_SAMPLE_BUFFERS)
	DEF(GL_SAMPLES)
	DEF(GL_SAMPLE_COVERAGE_VALUE)
	DEF(GL_SAMPLE_COVERAGE_INVERT)

	/* GetTextureParameter */
	/*      GL_TEXTURE_MAG_FILTER */
	/*      GL_TEXTURE_MIN_FILTER */
	/*      GL_TEXTURE_WRAP_S */
	/*      GL_TEXTURE_WRAP_T */

	DEF(GL_NUM_COMPRESSED_TEXTURE_FORMATS)
	DEF(GL_COMPRESSED_TEXTURE_FORMATS)

	/* HintMode */
	DEF(GL_DONT_CARE)
	DEF(GL_FASTEST)
	DEF(GL_NICEST)

	/* HintTarget */
	DEF(GL_GENERATE_MIPMAP_HINT)

	/* DataType */
	DEF(GL_BYTE)
	DEF(GL_UNSIGNED_BYTE)
	DEF(GL_SHORT)
	DEF(GL_UNSIGNED_SHORT)
	DEF(GL_INT)
	DEF(GL_UNSIGNED_INT)
	DEF(GL_FLOAT)
	DEF(GL_FIXED)

	/* PixelFormat */
	DEF(GL_DEPTH_COMPONENT)
	DEF(GL_ALPHA)
	DEF(GL_RGB)
	DEF(GL_RGBA)
	DEF(GL_LUMINANCE)
	DEF(GL_LUMINANCE_ALPHA)

	/* PixelType */
	/*      GL_UNSIGNED_BYTE */
	DEF(GL_UNSIGNED_SHORT_4_4_4_4)
	DEF(GL_UNSIGNED_SHORT_5_5_5_1)
	DEF(GL_UNSIGNED_SHORT_5_6_5)

	/* Shaders */
	DEF(GL_FRAGMENT_SHADER)
	DEF(GL_VERTEX_SHADER)
	DEF(GL_MAX_VERTEX_ATTRIBS)
	DEF(GL_MAX_VERTEX_UNIFORM_VECTORS)
	DEF(GL_MAX_VARYING_VECTORS)
	DEF(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS)
	DEF(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS)
	DEF(GL_MAX_TEXTURE_IMAGE_UNITS)
	DEF(GL_MAX_FRAGMENT_UNIFORM_VECTORS)
	DEF(GL_SHADER_TYPE)
	DEF(GL_DELETE_STATUS)
	DEF(GL_LINK_STATUS)
	DEF(GL_VALIDATE_STATUS)
	DEF(GL_ATTACHED_SHADERS)
	DEF(GL_ACTIVE_UNIFORMS)
	DEF(GL_ACTIVE_UNIFORM_MAX_LENGTH)
	DEF(GL_ACTIVE_ATTRIBUTES)
	DEF(GL_ACTIVE_ATTRIBUTE_MAX_LENGTH)
	DEF(GL_SHADING_LANGUAGE_VERSION)
	DEF(GL_CURRENT_PROGRAM)

	/* StencilFunction */
	DEF(GL_NEVER)
	DEF(GL_LESS)
	DEF(GL_EQUAL)
	DEF(GL_LEQUAL)
	DEF(GL_GREATER)
	DEF(GL_NOTEQUAL)
	DEF(GL_GEQUAL)
	DEF(GL_ALWAYS)

	/* StencilOp */
	/*      GL_ZERO */
	DEF(GL_KEEP)
	DEF(GL_REPLACE)
	DEF(GL_INCR)
	DEF(GL_DECR)
	DEF(GL_INVERT)
	DEF(GL_INCR_WRAP)
	DEF(GL_DECR_WRAP)

	/* StringName */
	DEF(GL_VENDOR)
	DEF(GL_RENDERER)
	DEF(GL_VERSION)
	DEF(GL_EXTENSIONS)

	/* TextureMagFilter */
	DEF(GL_NEAREST)
	DEF(GL_LINEAR)

	/* TextureMinFilter */
	/*      GL_NEAREST */
	/*      GL_LINEAR */
	DEF(GL_NEAREST_MIPMAP_NEAREST)
	DEF(GL_LINEAR_MIPMAP_NEAREST)
	DEF(GL_NEAREST_MIPMAP_LINEAR)
	DEF(GL_LINEAR_MIPMAP_LINEAR)

	/* TextureParameterName */
	DEF(GL_TEXTURE_MAG_FILTER)
	DEF(GL_TEXTURE_MIN_FILTER)
	DEF(GL_TEXTURE_WRAP_S)
	DEF(GL_TEXTURE_WRAP_T)

	/* TextureTarget */
	/*      GL_TEXTURE_2D */
	DEF(GL_TEXTURE)

	DEF(GL_TEXTURE_CUBE_MAP)
	DEF(GL_TEXTURE_BINDING_CUBE_MAP)
	DEF(GL_TEXTURE_CUBE_MAP_POSITIVE_X)
	DEF(GL_TEXTURE_CUBE_MAP_NEGATIVE_X)
	DEF(GL_TEXTURE_CUBE_MAP_POSITIVE_Y)
	DEF(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y)
	DEF(GL_TEXTURE_CUBE_MAP_POSITIVE_Z)
	DEF(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z)
	DEF(GL_MAX_CUBE_MAP_TEXTURE_SIZE)

	/* TextureUnit */
	DEF(GL_TEXTURE0)
	DEF(GL_TEXTURE1)
	DEF(GL_TEXTURE2)
	DEF(GL_TEXTURE3)
	DEF(GL_TEXTURE4)
	DEF(GL_TEXTURE5)
	DEF(GL_TEXTURE6)
	DEF(GL_TEXTURE7)
	DEF(GL_TEXTURE8)
	DEF(GL_TEXTURE9)
	DEF(GL_TEXTURE10)
	DEF(GL_TEXTURE11)
	DEF(GL_TEXTURE12)
	DEF(GL_TEXTURE13)
	DEF(GL_TEXTURE14)
	DEF(GL_TEXTURE15)
	DEF(GL_TEXTURE16)
	DEF(GL_TEXTURE17)
	DEF(GL_TEXTURE18)
	DEF(GL_TEXTURE19)
	DEF(GL_TEXTURE20)
	DEF(GL_TEXTURE21)
	DEF(GL_TEXTURE22)
	DEF(GL_TEXTURE23)
	DEF(GL_TEXTURE24)
	DEF(GL_TEXTURE25)
	DEF(GL_TEXTURE26)
	DEF(GL_TEXTURE27)
	DEF(GL_TEXTURE28)
	DEF(GL_TEXTURE29)
	DEF(GL_TEXTURE30)
	DEF(GL_TEXTURE31)
	DEF(GL_ACTIVE_TEXTURE)

	/* TextureWrapMode */
	DEF(GL_REPEAT)
	DEF(GL_CLAMP_TO_EDGE)
	DEF(GL_MIRRORED_REPEAT)

	/* Uniform Types */
	DEF(GL_FLOAT_VEC2)
	DEF(GL_FLOAT_VEC3)
	DEF(GL_FLOAT_VEC4)
	DEF(GL_INT_VEC2)
	DEF(GL_INT_VEC3)
	DEF(GL_INT_VEC4)
	DEF(GL_BOOL)
	DEF(GL_BOOL_VEC2)
	DEF(GL_BOOL_VEC3)
	DEF(GL_BOOL_VEC4)
	DEF(GL_FLOAT_MAT2)
	DEF(GL_FLOAT_MAT3)
	DEF(GL_FLOAT_MAT4)
	DEF(GL_SAMPLER_2D)
	DEF(GL_SAMPLER_CUBE)

	/* Vertex Arrays */
	DEF(GL_VERTEX_ATTRIB_ARRAY_ENABLED)
	DEF(GL_VERTEX_ATTRIB_ARRAY_SIZE)
	DEF(GL_VERTEX_ATTRIB_ARRAY_STRIDE)
	DEF(GL_VERTEX_ATTRIB_ARRAY_TYPE)
	DEF(GL_VERTEX_ATTRIB_ARRAY_NORMALIZED)
	DEF(GL_VERTEX_ATTRIB_ARRAY_POINTER)
	DEF(GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING)

	/* Read Format */
	DEF(GL_IMPLEMENTATION_COLOR_READ_TYPE)
	DEF(GL_IMPLEMENTATION_COLOR_READ_FORMAT)

	/* Shader Source */
	DEF(GL_COMPILE_STATUS)
	DEF(GL_INFO_LOG_LENGTH)
	DEF(GL_SHADER_SOURCE_LENGTH)
	DEF(GL_SHADER_COMPILER)

	/* Shader Binary */
	DEF(GL_SHADER_BINARY_FORMATS)
	DEF(GL_NUM_SHADER_BINARY_FORMATS)

	/* Shader Precision-Specified Types */
	DEF(GL_LOW_FLOAT)
	DEF(GL_MEDIUM_FLOAT)
	DEF(GL_HIGH_FLOAT)
	DEF(GL_LOW_INT)
	DEF(GL_MEDIUM_INT)
	DEF(GL_HIGH_INT)

	/* Framebuffer Object. */
	DEF(GL_FRAMEBUFFER)
	DEF(GL_RENDERBUFFER)

	DEF(GL_RGBA4)
	DEF(GL_RGB5_A1)
	DEF(GL_RGB565)
	DEF(GL_DEPTH_COMPONENT16)
	DEF(GL_STENCIL_INDEX8)

	DEF(GL_RENDERBUFFER_WIDTH)
	DEF(GL_RENDERBUFFER_HEIGHT)
	DEF(GL_RENDERBUFFER_INTERNAL_FORMAT)
	DEF(GL_RENDERBUFFER_RED_SIZE)
	DEF(GL_RENDERBUFFER_GREEN_SIZE)
	DEF(GL_RENDERBUFFER_BLUE_SIZE)
	DEF(GL_RENDERBUFFER_ALPHA_SIZE)
	DEF(GL_RENDERBUFFER_DEPTH_SIZE)
	DEF(GL_RENDERBUFFER_STENCIL_SIZE)

	DEF(GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE)
	DEF(GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME)
	DEF(GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL)
	DEF(GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE)

	DEF(GL_COLOR_ATTACHMENT0)
	DEF(GL_DEPTH_ATTACHMENT)
	DEF(GL_STENCIL_ATTACHMENT)

	DEF(GL_NONE)

	DEF(GL_FRAMEBUFFER_COMPLETE)
	DEF(GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT)
	DEF(GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT)
	DEF(GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS)
	DEF(GL_FRAMEBUFFER_UNSUPPORTED)

	DEF(GL_FRAMEBUFFER_BINDING)
	DEF(GL_RENDERBUFFER_BINDING)
	DEF(GL_MAX_RENDERBUFFER_SIZE)

	DEF(GL_INVALID_FRAMEBUFFER_OPERATION)

#ifdef GL_ES_VERSION_3_0
	/* OpenGL ES 3.0 */

	DEF(GL_READ_BUFFER)
	DEF(GL_UNPACK_ROW_LENGTH)
	DEF(GL_UNPACK_SKIP_ROWS)
	DEF(GL_UNPACK_SKIP_PIXELS)
	DEF(GL_PACK_ROW_LENGTH)
	DEF(GL_PACK_SKIP_ROWS)
	DEF(GL_PACK_SKIP_PIXELS)
	DEF(GL_COLOR)
	DEF(GL_DEPTH)
	DEF(GL_STENCIL)
	DEF(GL_RED)
	DEF(GL_RGB8)
	DEF(GL_RGBA8)
	DEF(GL_RGB10_A2)
	DEF(GL_TEXTURE_BINDING_3D)
	DEF(GL_UNPACK_SKIP_IMAGES)
	DEF(GL_UNPACK_IMAGE_HEIGHT)
	DEF(GL_TEXTURE_3D)
	DEF(GL_TEXTURE_WRAP_R)
	DEF(GL_MAX_3D_TEXTURE_SIZE)
	DEF(GL_UNSIGNED_INT_2_10_10_10_REV)
	DEF(GL_MAX_ELEMENTS_VERTICES)
	DEF(GL_MAX_ELEMENTS_INDICES)
	DEF(GL_TEXTURE_MIN_LOD)
	DEF(GL_TEXTURE_MAX_LOD)
	DEF(GL_TEXTURE_BASE_LEVEL)
	DEF(GL_TEXTURE_MAX_LEVEL)
	DEF(GL_MIN)
	DEF(GL_MAX)
	DEF(GL_DEPTH_COMPONENT24)
	DEF(GL_MAX_TEXTURE_LOD_BIAS)
	DEF(GL_TEXTURE_COMPARE_MODE)
	DEF(GL_TEXTURE_COMPARE_FUNC)
	DEF(GL_CURRENT_QUERY)
	DEF(GL_QUERY_RESULT)
	DEF(GL_QUERY_RESULT_AVAILABLE)
	DEF(GL_BUFFER_MAPPED)
	DEF(GL_BUFFER_MAP_POINTER)
	DEF(GL_STREAM_READ)
	DEF(GL_STREAM_COPY)
	DEF(GL_STATIC_READ)
	DEF(GL_STATIC_COPY)
	DEF(GL_DYNAMIC_READ)
	DEF(GL_DYNAMIC_COPY)
	DEF(GL_MAX_DRAW_BUFFERS)
	DEF(GL_DRAW_BUFFER0)
	DEF(GL_DRAW_BUFFER1)
	DEF(GL_DRAW_BUFFER2)
	DEF(GL_DRAW_BUFFER3)
	DEF(GL_DRAW_BUFFER4)
	DEF(GL_DRAW_BUFFER5)
	DEF(GL_DRAW_BUFFER6)
	DEF(GL_DRAW_BUFFER7)
	DEF(GL_DRAW_BUFFER8)
	DEF(GL_DRAW_BUFFER9)
	DEF(GL_DRAW_BUFFER10)
	DEF(GL_DRAW_BUFFER11)
	DEF(GL_DRAW_BUFFER12)
	DEF(GL_DRAW_BUFFER13)
	DEF(GL_DRAW_BUFFER14)
	DEF(GL_DRAW_BUFFER15)
	DEF(GL_MAX_FRAGMENT_UNIFORM_COMPONENTS)
	DEF(GL_MAX_VERTEX_UNIFORM_COMPONENTS)
	DEF(GL_SAMPLER_3D)
	DEF(GL_SAMPLER_2D_SHADOW)
	DEF(GL_FRAGMENT_SHADER_DERIVATIVE_HINT)
	DEF(GL_PIXEL_PACK_BUFFER)
	DEF(GL_PIXEL_UNPACK_BUFFER)
	DEF(GL_PIXEL_PACK_BUFFER_BINDING)
	DEF(GL_PIXEL_UNPACK_BUFFER_BINDING)
	DEF(GL_FLOAT_MAT2x3)
	DEF(GL_FLOAT_MAT2x4)
	DEF(GL_FLOAT_MAT3x2)
	DEF(GL_FLOAT_MAT3x4)
	DEF(GL_FLOAT_MAT4x2)
	DEF(GL_FLOAT_MAT4x3)
	DEF(GL_SRGB)
	DEF(GL_SRGB8)
	DEF(GL_SRGB8_ALPHA8)
	DEF(GL_COMPARE_REF_TO_TEXTURE)
	DEF(GL_MAJOR_VERSION)
	DEF(GL_MINOR_VERSION)
	DEF(GL_NUM_EXTENSIONS)
	DEF(GL_RGBA32F)
	DEF(GL_RGB32F)
	DEF(GL_RGBA16F)
	DEF(GL_RGB16F)
	DEF(GL_VERTEX_ATTRIB_ARRAY_INTEGER)
	DEF(GL_MAX_ARRAY_TEXTURE_LAYERS)
	DEF(GL_MIN_PROGRAM_TEXEL_OFFSET)
	DEF(GL_MAX_PROGRAM_TEXEL_OFFSET)
	DEF(GL_MAX_VARYING_COMPONENTS)
	DEF(GL_TEXTURE_2D_ARRAY)
	DEF(GL_TEXTURE_BINDING_2D_ARRAY)
	DEF(GL_R11F_G11F_B10F)
	DEF(GL_UNSIGNED_INT_10F_11F_11F_REV)
	DEF(GL_RGB9_E5)
	DEF(GL_UNSIGNED_INT_5_9_9_9_REV)
	DEF(GL_TRANSFORM_FEEDBACK_VARYING_MAX_LENGTH)
	DEF(GL_TRANSFORM_FEEDBACK_BUFFER_MODE)
	DEF(GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS)
	DEF(GL_TRANSFORM_FEEDBACK_VARYINGS)
	DEF(GL_TRANSFORM_FEEDBACK_BUFFER_START)
	DEF(GL_TRANSFORM_FEEDBACK_BUFFER_SIZE)
	DEF(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN)
	DEF(GL_RASTERIZER_DISCARD)
	DEF(GL_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS)
	DEF(GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS)
	DEF(GL_INTERLEAVED_ATTRIBS)
	DEF(GL_SEPARATE_ATTRIBS)
	DEF(GL_TRANSFORM_FEEDBACK_BUFFER)
	DEF(GL_TRANSFORM_FEEDBACK_BUFFER_BINDING)
	DEF(GL_RGBA32UI)
	DEF(GL_RGB32UI)
	DEF(GL_RGBA16UI)
	DEF(GL_RGB16UI)
	DEF(GL_RGBA8UI)
	DEF(GL_RGB8UI)
	DEF(GL_RGBA32I)
	DEF(GL_RGB32I)
	DEF(GL_RGBA16I)
	DEF(GL_RGB16I)
	DEF(GL_RGBA8I)
	DEF(GL_RGB8I)
	DEF(GL_RED_INTEGER)
	DEF(GL_RGB_INTEGER)
	DEF(GL_RGBA_INTEGER)
	DEF(GL_SAMPLER_2D_ARRAY)
	DEF(GL_SAMPLER_2D_ARRAY_SHADOW)
	DEF(GL_SAMPLER_CUBE_SHADOW)
	DEF(GL_UNSIGNED_INT_VEC2)
	DEF(GL_UNSIGNED_INT_VEC3)
	DEF(GL_UNSIGNED_INT_VEC4)
	DEF(GL_INT_SAMPLER_2D)
	DEF(GL_INT_SAMPLER_3D)
	DEF(GL_INT_SAMPLER_CUBE)
	DEF(GL_INT_SAMPLER_2D_ARRAY)
	DEF(GL_UNSIGNED_INT_SAMPLER_2D)
	DEF(GL_UNSIGNED_INT_SAMPLER_3D)
	DEF(GL_UNSIGNED_INT_SAMPLER_CUBE)
	DEF(GL_UNSIGNED_INT_SAMPLER_2D_ARRAY)
	DEF(GL_BUFFER_ACCESS_FLAGS)
	DEF(GL_BUFFER_MAP_LENGTH)
	DEF(GL_BUFFER_MAP_OFFSET)
	DEF(GL_DEPTH_COMPONENT32F)
	DEF(GL_DEPTH32F_STENCIL8)
	DEF(GL_FLOAT_32_UNSIGNED_INT_24_8_REV)
	DEF(GL_FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING)
	DEF(GL_FRAMEBUFFER_ATTACHMENT_COMPONENT_TYPE)
	DEF(GL_FRAMEBUFFER_ATTACHMENT_RED_SIZE)
	DEF(GL_FRAMEBUFFER_ATTACHMENT_GREEN_SIZE)
	DEF(GL_FRAMEBUFFER_ATTACHMENT_BLUE_SIZE)
	DEF(GL_FRAMEBUFFER_ATTACHMENT_ALPHA_SIZE)
	DEF(GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE)
	DEF(GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE)
	DEF(GL_FRAMEBUFFER_DEFAULT)
	DEF(GL_FRAMEBUFFER_UNDEFINED)
	DEF(GL_DEPTH_STENCIL_ATTACHMENT)
	DEF(GL_DEPTH_STENCIL)
	DEF(GL_UNSIGNED_INT_24_8)
	DEF(GL_DEPTH24_STENCIL8)
	DEF(GL_UNSIGNED_NORMALIZED)
	DEF(GL_DRAW_FRAMEBUFFER_BINDING)
	DEF(GL_READ_FRAMEBUFFER)
	DEF(GL_DRAW_FRAMEBUFFER)
	DEF(GL_READ_FRAMEBUFFER_BINDING)
	DEF(GL_RENDERBUFFER_SAMPLES)
	DEF(GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LAYER)
	DEF(GL_MAX_COLOR_ATTACHMENTS)
	DEF(GL_COLOR_ATTACHMENT1)
	DEF(GL_COLOR_ATTACHMENT2)
	DEF(GL_COLOR_ATTACHMENT3)
	DEF(GL_COLOR_ATTACHMENT4)
	DEF(GL_COLOR_ATTACHMENT5)
	DEF(GL_COLOR_ATTACHMENT6)
	DEF(GL_COLOR_ATTACHMENT7)
	DEF(GL_COLOR_ATTACHMENT8)
	DEF(GL_COLOR_ATTACHMENT9)
	DEF(GL_COLOR_ATTACHMENT10)
	DEF(GL_COLOR_ATTACHMENT11)
	DEF(GL_COLOR_ATTACHMENT12)
	DEF(GL_COLOR_ATTACHMENT13)
	DEF(GL_COLOR_ATTACHMENT14)
	DEF(GL_COLOR_ATTACHMENT15)
	DEF(GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE)
	DEF(GL_MAX_SAMPLES)
	DEF(GL_HALF_FLOAT)
	DEF(GL_MAP_READ_BIT)
	DEF(GL_MAP_WRITE_BIT)
	DEF(GL_MAP_INVALIDATE_RANGE_BIT)
	DEF(GL_MAP_INVALIDATE_BUFFER_BIT)
	DEF(GL_MAP_FLUSH_EXPLICIT_BIT)
	DEF(GL_MAP_UNSYNCHRONIZED_BIT)
	DEF(GL_RG)
	DEF(GL_RG_INTEGER)
	DEF(GL_R8)
	DEF(GL_RG8)
	DEF(GL_R16F)
	DEF(GL_R32F)
	DEF(GL_RG16F)
	DEF(GL_RG32F)
	DEF(GL_R8I)
	DEF(GL_R8UI)
	DEF(GL_R16I)
	DEF(GL_R16UI)
	DEF(GL_R32I)
	DEF(GL_R32UI)
	DEF(GL_RG8I)
	DEF(GL_RG8UI)
	DEF(GL_RG16I)
	DEF(GL_RG16UI)
	DEF(GL_RG32I)
	DEF(GL_RG32UI)
	DEF(GL_VERTEX_ARRAY_BINDING)
	DEF(GL_R8_SNORM)
	DEF(GL_RG8_SNORM)
	DEF(GL_RGB8_SNORM)
	DEF(GL_RGBA8_SNORM)
	DEF(GL_SIGNED_NORMALIZED)
	DEF(GL_PRIMITIVE_RESTART_FIXED_INDEX)
	DEF(GL_COPY_READ_BUFFER)
	DEF(GL_COPY_WRITE_BUFFER)
	DEF(GL_COPY_READ_BUFFER_BINDING)
	DEF(GL_COPY_WRITE_BUFFER_BINDING)
	DEF(GL_UNIFORM_BUFFER)
	DEF(GL_UNIFORM_BUFFER_BINDING)
	DEF(GL_UNIFORM_BUFFER_START)
	DEF(GL_UNIFORM_BUFFER_SIZE)
	DEF(GL_MAX_VERTEX_UNIFORM_BLOCKS)
	DEF(GL_MAX_FRAGMENT_UNIFORM_BLOCKS)
	DEF(GL_MAX_COMBINED_UNIFORM_BLOCKS)
	DEF(GL_MAX_UNIFORM_BUFFER_BINDINGS)
	DEF(GL_MAX_UNIFORM_BLOCK_SIZE)
	DEF(GL_MAX_COMBINED_VERTEX_UNIFORM_COMPONENTS)
	DEF(GL_MAX_COMBINED_FRAGMENT_UNIFORM_COMPONENTS)
	DEF(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT)
	DEF(GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH)
	DEF(GL_ACTIVE_UNIFORM_BLOCKS)
	DEF(GL_UNIFORM_TYPE)
	DEF(GL_UNIFORM_SIZE)
	DEF(GL_UNIFORM_NAME_LENGTH)
	DEF(GL_UNIFORM_BLOCK_INDEX)
	DEF(GL_UNIFORM_OFFSET)
	DEF(GL_UNIFORM_ARRAY_STRIDE)
	DEF(GL_UNIFORM_MATRIX_STRIDE)
	DEF(GL_UNIFORM_IS_ROW_MAJOR)
	DEF(GL_UNIFORM_BLOCK_BINDING)
	DEF(GL_UNIFORM_BLOCK_DATA_SIZE)
	DEF(GL_UNIFORM_BLOCK_NAME_LENGTH)
	DEF(GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS)
	DEF(GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES)
	DEF(GL_UNIFORM_BLOCK_REFERENCED_BY_VERTEX_SHADER)
	DEF(GL_UNIFORM_BLOCK_REFERENCED_BY_FRAGMENT_SHADER)
	DEF(GL_INVALID_INDEX)
	DEF(GL_MAX_VERTEX_OUTPUT_COMPONENTS)
	DEF(GL_MAX_FRAGMENT_INPUT_COMPONENTS)
	DEF(GL_MAX_SERVER_WAIT_TIMEOUT)
	DEF(GL_OBJECT_TYPE)
	DEF(GL_SYNC_CONDITION)
	DEF(GL_SYNC_STATUS)
	DEF(GL_SYNC_FLAGS)
	DEF(GL_SYNC_FENCE)
	DEF(GL_SYNC_GPU_COMMANDS_COMPLETE)
	DEF(GL_UNSIGNALED)
	DEF(GL_SIGNALED)
	DEF(GL_ALREADY_SIGNALED)
	DEF(GL_TIMEOUT_EXPIRED)
	DEF(GL_CONDITION_SATISFIED)
	DEF(GL_WAIT_FAILED)
	DEF(GL_SYNC_FLUSH_COMMANDS_BIT)
	//DEF(GL_TIMEOUT_IGNORED) // Value is too large
	DEF(GL_VERTEX_ATTRIB_ARRAY_DIVISOR)
	DEF(GL_ANY_SAMPLES_PASSED)
	DEF(GL_ANY_SAMPLES_PASSED_CONSERVATIVE)
	DEF(GL_SAMPLER_BINDING)
	DEF(GL_RGB10_A2UI)
	DEF(GL_TEXTURE_SWIZZLE_R)
	DEF(GL_TEXTURE_SWIZZLE_G)
	DEF(GL_TEXTURE_SWIZZLE_B)
	DEF(GL_TEXTURE_SWIZZLE_A)
	DEF(GL_GREEN)
	DEF(GL_BLUE)
	DEF(GL_INT_2_10_10_10_REV)
	DEF(GL_TRANSFORM_FEEDBACK)
	DEF(GL_TRANSFORM_FEEDBACK_PAUSED)
	DEF(GL_TRANSFORM_FEEDBACK_ACTIVE)
	DEF(GL_TRANSFORM_FEEDBACK_BINDING)
	DEF(GL_PROGRAM_BINARY_RETRIEVABLE_HINT)
	DEF(GL_PROGRAM_BINARY_LENGTH)
	DEF(GL_NUM_PROGRAM_BINARY_FORMATS)
	DEF(GL_PROGRAM_BINARY_FORMATS)
	DEF(GL_COMPRESSED_R11_EAC)
	DEF(GL_COMPRESSED_SIGNED_R11_EAC)
	DEF(GL_COMPRESSED_RG11_EAC)
	DEF(GL_COMPRESSED_SIGNED_RG11_EAC)
	DEF(GL_COMPRESSED_RGB8_ETC2)
	DEF(GL_COMPRESSED_SRGB8_ETC2)
	DEF(GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2)
	DEF(GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2)
	DEF(GL_COMPRESSED_RGBA8_ETC2_EAC)
	DEF(GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC)
	DEF(GL_TEXTURE_IMMUTABLE_FORMAT)
	DEF(GL_MAX_ELEMENT_INDEX)
	DEF(GL_NUM_SAMPLE_COUNTS)
	DEF(GL_TEXTURE_IMMUTABLE_LEVELS)

#endif

	{ 0, NULL }
};

#endif

//---
// Must... constrain... gag reflex... Big abuse of macros
//---

// Macros that work regardless of ID_GL_HARDLINK
#define SYM_DIR_SET_ALWAYS( a, v ) q##a = v
#if 0 //#ifdef _DEBUG
	#define SYM_LINK_ALWAYS( a ) q##a = ( ?? )dlsym( qnx.openGLLib, #a )
	#define ESYM_LINK_ALWAYS( a ) q##a = ( ?? )dlsym( qnx.eglLib, #a )
#else
	#define SYM_LINK_ALWAYS( a ) SYM_DIR_SET_ALWAYS( a, a )
	#define ESYM_LINK_ALWAYS( a ) SYM_LINK_ALWAYS( a )
#endif

// Macros dependent on if ID_GL_HARDLINK exists or not
#ifdef ID_GL_HARDLINK
	#define SET_SYM( a, egl ) glLinkSet( #a, ( void* )a, egl, glVersion )
	#define SYM_LINK( a )
	#define ESYM_LINK( a )
	#define SYM_DIR_SET( a, v )
#else
	#define SET_SYM( a, egl ) glLinkSet( #a, ( void* )q##a, egl, glVersion )
	#define SYM_LINK( a ) SYM_LINK_ALWAYS( a );
	#define ESYM_LINK( a ) ESYM_LINK_ALWAYS( a );
	#define SYM_DIR_SET( a, v ) SYM_DIR_SET_ALWAYS( a, v );
#endif

// Macros for getting GL/EGL functions ([] is normal, [S] is "skip link", [R] is "replacement"
#define GLSYM( a ) \
	SYM_LINK( a ) \
	SET_SYM( a, false )
#define GLSYMS( a ) SET_SYM( a, false )
#define GLSYMR_NULL( a, rep, op ) \
	q##a = (rep).a##Impl; \
	if ( q##a == NULL ) { \
		op; \
	} \
	GLSYMS( a )
#define GLSYMR( a, rep ) GLSYMR_NULL( a, rep, SYM_LINK_ALWAYS( a ) )
#define GLDIRECTSYMS( a ) glLinkSet( #a, ( void* )a, false, glVersion )
#define EGLSYM( a ) \
	ESYM_LINK( a ) \
	SET_SYM( a, true )

// Macros for nulling GL/EGL functions
#define GLNULL( a ) \
	SYM_DIR_SET( a, NULL ) \
	glLinkSet( #a, NULL, false, glVersion )
#define GLNULLR( a ) \
	SYM_DIR_SET_ALWAYS( a, NULL ); \
	glLinkSet( #a, NULL, false, glVersion )
#define EGLNULL( a ) \
	SYM_DIR_SET( a, NULL ) \
	glLinkSet( #a, NULL, true, glVersion )
#define GLDIRECTNULL( a ) glLinkSet( #a, NULL, false, glVersion )

/*
** QGL_Shutdown
**
** Unloads the specified DLL then nulls out all the proc pointers.  This
** is only called during a hard shutdown of the OGL subsystem (e.g. vid_restart).
*/
void QGL_Shutdown( void )
{
	common->Printf( "...shutting down QGL\n" );

	if ( qnx.openGLLib )
	{
		common->Printf( "...unloading OpenGL ES DLL\n" );
		dlclose( qnx.openGLLib );
	}

	if ( qnx.eglLib )
	{
		common->Printf( "...unloading EGL DLL\n" );
		dlclose( qnx.eglLib );
	}

	qnx.openGLLib = NULL;
	qnx.eglLib = NULL;

	float glVersion = 0.0f;

	GLNULLR( glReadBuffer );
	GLNULLR( glDrawBuffer );
	GLNULLR( glReadPixels );
	GLNULLR( glCopyTexImage2D );
	GLNULLR( glCopyTexSubImage2D );

#ifdef GL_ES_VERSION_3_0

	GLNULLR( glCopyTexSubImage3D );

#endif

	GLNULL( glActiveTexture );
	GLDIRECTNULL( glAttachShader );
	GLDIRECTNULL( glBindAttribLocation );
	GLNULL( glBindBuffer );
	GLNULL( glBindFramebuffer );
	GLNULL( glBindRenderbuffer );
	GLNULL( glBindTexture );
	GLNULL( glBlendColor );
	GLNULL( glBlendEquation );
	GLNULL( glBlendEquationSeparate );
	GLNULL( glBlendFunc );
	GLNULL( glBlendFuncSeparate );
	GLNULL( glBufferData );
	GLNULL( glBufferSubData );
	GLNULL( glCheckFramebufferStatus );
	GLNULL( glClear );
	GLNULL( glClearColor );
	GLNULL( glClearDepth );
	GLNULL( glClearStencil );
	GLNULL( glColorMask );
	GLDIRECTNULL( glCompileShader );
	GLNULL( glCompressedTexImage2D );
	GLNULL( glCompressedTexSubImage2D );
	//GLNULL( glCopyTexImage2D );
	//GLNULL( glCopyTexSubImage2D );
	GLDIRECTNULL( glCreateProgram );
	GLDIRECTNULL( glCreateShader );
	GLNULL( glCullFace );
	GLNULL( glDeleteBuffers );
	GLNULL( glDeleteFramebuffers );
	GLDIRECTNULL( glDeleteProgram );
	GLNULL( glDeleteRenderbuffers );
	GLDIRECTNULL( glDeleteShader );
	GLNULL( glDeleteTextures );
	GLNULL( glDepthFunc );
	GLNULL( glDepthMask );
	GLNULL( glDepthRange );
	GLDIRECTNULL( glDetachShader );
	GLNULL( glDisable );
	GLNULL( glDisableVertexAttribArray );
	GLNULL( glDrawArrays );
	GLNULL( glDrawElements );
	GLNULL( glEnable );
	GLNULL( glEnableVertexAttribArray );
	GLNULL( glFinish );
	GLNULL( glFlush );
	GLNULL( glFramebufferRenderbuffer );
	GLNULL( glFramebufferTexture2D );
	GLNULL( glFrontFace );
	GLNULL( glGenBuffers );
	GLNULL( glGenerateMipmap );
	GLNULL( glGenFramebuffers );
	GLNULL( glGenRenderbuffers );
	GLNULL( glGenTextures );
	GLNULL( glGetActiveAttrib );
	GLNULL( glGetActiveUniform );
	GLNULL( glGetAttachedShaders );
	GLNULL( glGetAttribLocation );
	GLNULL( glGetBooleanv );
	GLNULL( glGetBufferParameteriv );
	GLNULL( glGetError );
	GLNULL( glGetFloatv );
	GLNULL( glGetFramebufferAttachmentParameteriv );
	GLNULL( glGetIntegerv );
	GLDIRECTNULL( glGetProgramiv );
	GLDIRECTNULL( glGetProgramInfoLog );
	GLNULL( glGetRenderbufferParameteriv );
	GLDIRECTNULL( glGetShaderiv );
	GLDIRECTNULL( glGetShaderInfoLog );
	GLNULL( glGetShaderPrecisionFormat );
	GLNULL( glGetShaderSource );
	GLNULL( glGetString );
	GLNULL( glGetTexParameterfv );
	GLNULL( glGetTexParameteriv );
	GLNULL( glGetUniformfv );
	GLNULL( glGetUniformiv );
	GLDIRECTNULL( glGetUniformLocation );
	GLNULL( glGetVertexAttribfv );
	GLNULL( glGetVertexAttribiv );
	GLNULL( glGetVertexAttribPointerv );
	GLNULL( glHint );
	GLNULL( glIsBuffer );
	GLNULL( glIsEnabled );
	GLNULL( glIsFramebuffer );
	GLNULL( glIsProgram );
	GLNULL( glIsRenderbuffer );
	GLNULL( glIsShader );
	GLNULL( glIsTexture );
	GLNULL( glLineWidth );
	GLDIRECTNULL( glLinkProgram );
	GLNULL( glPixelStorei );
	GLNULL( glPolygonOffset );
	//GLNULL( glReadPixels );
	GLNULL( glReleaseShaderCompiler );
	GLNULL( glRenderbufferStorage );
	GLNULL( glSampleCoverage );
	GLNULL( glScissor );
	GLNULL( glShaderBinary );
	GLDIRECTNULL( glShaderSource );
	GLNULL( glStencilFunc );
	GLDIRECTNULL( glStencilFuncSeparate );
	GLNULL( glStencilMask );
	GLNULL( glStencilMaskSeparate );
	GLNULL( glStencilOp );
	GLDIRECTNULL( glStencilOpSeparate );
	GLNULL( glTexImage2D );
	GLNULL( glTexParameterf );
	GLNULL( glTexParameterfv );
	GLNULL( glTexParameteri );
	GLNULL( glTexParameteriv );
	GLNULL( glTexSubImage2D );
	GLNULL( glUniform1f );
	GLNULL( glUniform1fv );
	GLDIRECTNULL( glUniform1i );
	GLNULL( glUniform1iv );
	GLNULL( glUniform2f );
	GLNULL( glUniform2fv );
	GLNULL( glUniform2i );
	GLNULL( glUniform2iv );
	GLNULL( glUniform3f );
	GLNULL( glUniform3fv );
	GLNULL( glUniform3i );
	GLNULL( glUniform3iv );
	GLNULL( glUniform4f );
	GLDIRECTNULL( glUniform4fv );
	GLNULL( glUniform4i );
	GLNULL( glUniform4iv );
	GLNULL( glUniformMatrix2fv );
	GLNULL( glUniformMatrix3fv );
	GLNULL( glUniformMatrix4fv );
	GLDIRECTNULL( glUseProgram );
	GLNULL( glValidateProgram );
	GLNULL( glVertexAttrib1f );
	GLNULL( glVertexAttrib1fv );
	GLNULL( glVertexAttrib2f );
	GLNULL( glVertexAttrib2fv );
	GLNULL( glVertexAttrib3f );
	GLNULL( glVertexAttrib3fv );
	GLNULL( glVertexAttrib4f );
	GLNULL( glVertexAttrib4fv );
	GLNULL( glVertexAttribPointer );
	GLNULL( glViewport );

#ifdef GL_ES_VERSION_3_0

	//GLDIRECTNULL( glReadBuffer );
	GLNULL( glDrawRangeElements );
	GLNULL( glTexImage3D );
	GLNULL( glTexSubImage3D );
	//GLNULL( glCopyTexSubImage3D );
	GLNULL( glCompressedTexImage3D );
	GLNULL( glCompressedTexSubImage3D );
	GLNULL( glGenQueries );
	GLNULL( glDeleteQueries );
	GLNULL( glIsQuery );
	GLNULL( glBeginQuery );
	GLNULL( glEndQuery );
	GLNULL( glGetQueryiv );
	GLNULL( glGetQueryObjectuiv );
	GLNULL( glUnmapBuffer );
	GLNULL( glGetBufferPointerv );
	GLNULL( glDrawBuffers );
	GLNULL( glUniformMatrix2x3fv );
	GLNULL( glUniformMatrix3x2fv );
	GLNULL( glUniformMatrix2x4fv );
	GLNULL( glUniformMatrix4x2fv );
	GLNULL( glUniformMatrix3x4fv );
	GLNULL( glUniformMatrix4x3fv );
	GLNULL( glBlitFramebuffer );
	GLNULL( glRenderbufferStorageMultisample );
	GLNULL( glFramebufferTextureLayer );
	GLDIRECTNULL( glMapBufferRange );
	GLNULL( glFlushMappedBufferRange );
	GLDIRECTNULL( glBindVertexArray );
	GLDIRECTNULL( glDeleteVertexArrays );
	GLDIRECTNULL( glGenVertexArrays );
	GLNULL( glIsVertexArray );
	GLNULL( glGetIntegeri_v );
	GLNULL( glBeginTransformFeedback );
	GLNULL( glEndTransformFeedback );
	GLDIRECTNULL( glBindBufferRange );
	GLNULL( glBindBufferBase );
	GLNULL( glTransformFeedbackVaryings );
	GLNULL( glGetTransformFeedbackVarying );
	GLNULL( glVertexAttribIPointer );
	GLNULL( glGetVertexAttribIiv );
	GLNULL( glGetVertexAttribIuiv );
	GLNULL( glVertexAttribI4i );
	GLNULL( glVertexAttribI4ui );
	GLNULL( glVertexAttribI4iv );
	GLNULL( glVertexAttribI4uiv );
	GLNULL( glGetUniformuiv );
	GLNULL( glGetFragDataLocation );
	GLNULL( glUniform1ui );
	GLNULL( glUniform2ui );
	GLNULL( glUniform3ui );
	GLNULL( glUniform4ui );
	GLNULL( glUniform1uiv );
	GLNULL( glUniform2uiv );
	GLNULL( glUniform3uiv );
	GLNULL( glUniform4uiv );
	GLNULL( glClearBufferiv );
	GLNULL( glClearBufferuiv );
	GLNULL( glClearBufferfv );
	GLNULL( glClearBufferfi );
	GLDIRECTNULL( glGetStringi );
	GLNULL( glCopyBufferSubData );
	GLNULL( glGetUniformIndices );
	GLNULL( glGetActiveUniformsiv );
	GLDIRECTNULL( glGetUniformBlockIndex );
	GLNULL( glGetActiveUniformBlockiv );
	GLNULL( glGetActiveUniformBlockName );
	GLDIRECTNULL( glUniformBlockBinding );
	GLNULL( glDrawArraysInstanced );
	GLNULL( glDrawElementsInstanced );
	GLDIRECTNULL( glFenceSync );
	GLDIRECTNULL( glIsSync );
	GLDIRECTNULL( glDeleteSync );
	GLDIRECTNULL( glClientWaitSync );
	GLNULL( glWaitSync );
	GLNULL( glGetInteger64v );
	GLNULL( glGetSynciv );
	GLNULL( glGetInteger64i_v );
	GLNULL( glGetBufferParameteri64v );
	GLNULL( glGenSamplers );
	GLNULL( glDeleteSamplers );
	GLNULL( glIsSampler );
	GLNULL( glBindSampler );
	GLNULL( glSamplerParameteri );
	GLNULL( glSamplerParameteriv );
	GLNULL( glSamplerParameterf );
	GLNULL( glSamplerParameterfv );
	GLNULL( glGetSamplerParameteriv );
	GLNULL( glGetSamplerParameterfv );
	GLNULL( glVertexAttribDivisor );
	GLNULL( glBindTransformFeedback );
	GLNULL( glDeleteTransformFeedbacks );
	GLNULL( glGenTransformFeedbacks );
	GLNULL( glIsTransformFeedback );
	GLNULL( glPauseTransformFeedback );
	GLNULL( glResumeTransformFeedback );
	GLNULL( glGetProgramBinary );
	GLNULL( glProgramBinary );
	GLDIRECTNULL( glProgramParameteri );
	GLNULL( glInvalidateFramebuffer );
	GLNULL( glInvalidateSubFramebuffer );
	GLNULL( glTexStorage2D );
	GLNULL( glTexStorage3D );
	GLNULL( glGetInternalformativ );

#endif

	EGLNULL( eglGetError );
	EGLNULL( eglGetDisplay );
	EGLNULL( eglInitialize );
	EGLNULL( eglTerminate );
	EGLNULL( eglQueryString );
	EGLNULL( eglGetConfigs );
	EGLNULL( eglChooseConfig );
	EGLNULL( eglGetConfigAttrib );
	EGLNULL( eglCreateWindowSurface );
	EGLNULL( eglCreatePbufferSurface );
	EGLNULL( eglCreatePixmapSurface );
	EGLNULL( eglDestroySurface );
	EGLNULL( eglQuerySurface );
	EGLNULL( eglBindAPI );
	EGLNULL( eglQueryAPI );
	EGLNULL( eglWaitClient );
	EGLNULL( eglReleaseThread );
	EGLNULL( eglCreatePbufferFromClientBuffer );
	EGLNULL( eglSurfaceAttrib );
	EGLNULL( eglBindTexImage );
	EGLNULL( eglReleaseTexImage );
	EGLNULL( eglSwapInterval );
	EGLNULL( eglCreateContext );
	EGLNULL( eglDestroyContext );
	EGLNULL( eglMakeCurrent );
	EGLNULL( eglGetCurrentContext );
	EGLNULL( eglGetCurrentSurface );
	EGLNULL( eglGetCurrentDisplay );
	EGLNULL( eglQueryContext );
	EGLNULL( eglWaitGL );
	EGLNULL( eglWaitNative );
	EGLNULL( eglSwapBuffers );
	EGLNULL( eglCopyBuffers );
	EGLNULL( eglGetProcAddress );
}

/*
** QGL_GetSym
*/
void* QGL_GetSym( const char *function, bool egl ) {
	if ( function == NULL ) {
		return NULL;
	}
	return glLinkGet( function, egl, glConfig.glVersion );
}

/*
** QGL_Init
**
** This is responsible for binding our qgl function pointers to
** the appropriate GL stuff.
*/
bool QGL_Init( const char *dllname, const EGLFunctionReplacements_t & replacements )
{
	assert( qnx.eglLib == NULL );
	assert( qnx.openGLLib == NULL );

	common->Printf( "...initializing QGL\n" );

	// EGL
	extern idCVar r_eglDriver;
	const char *eglname = r_eglDriver.GetString()[0] ? r_eglDriver.GetString() : "libEGL.so";

	common->Printf( "...calling dlopen( '%s' ): ", eglname );

	if ( ( qnx.eglLib = dlopen( eglname, RTLD_LAZY ) ) == NULL )
	{
		common->Printf( "failed\n" );
		return false;
	}
	common->Printf( "succeeded\n" );

	// OpenGL
	common->Printf( "...calling dlopen( '%s' ): ", dllname );

	if ( ( qnx.openGLLib = dlopen( dllname, RTLD_LAZY ) ) == NULL )
	{
		common->Printf( "failed\n" );
		dlclose( qnx.eglLib );
		qnx.eglLib = NULL;
		return false;
	}
	common->Printf( "succeeded\n" );

	// Setup replaceable items
	void OGL_UpdateReplacements( const EGLFunctionReplacements_t & replacements );
	OGL_UpdateReplacements( replacements );

	float glVersion = 2.0f;

	GLSYM( glActiveTexture );
	GLDIRECTSYMS( glAttachShader );
	GLDIRECTSYMS( glBindAttribLocation );
	GLSYM( glBindBuffer );
	GLSYM( glBindFramebuffer );
	GLSYM( glBindRenderbuffer );
	GLSYM( glBindTexture );
	GLSYM( glBlendColor );
	GLSYM( glBlendEquation );
	GLSYM( glBlendEquationSeparate );
	GLSYM( glBlendFunc );
	GLSYM( glBlendFuncSeparate );
	GLSYM( glBufferData );
	GLSYM( glBufferSubData );
	GLSYM( glCheckFramebufferStatus );
	GLSYM( glClear );
	GLSYM( glClearColor );
#ifdef glClearDepth
	GLSYM( glClearDepth );
#else
#if 0 //#ifdef _DEBUG
	qglClearDepth = dlsym( qnx.openGLLib, "glClearDepthf" );
#else
	qglClearDepth = glClearDepthf;
#endif
	GLSYMS( glClearDepth );
#endif
	GLSYM( glClearStencil );
	GLSYM( glColorMask );
	GLDIRECTSYMS( glCompileShader );
	GLSYM( glCompressedTexImage2D );
	GLSYM( glCompressedTexSubImage2D );
	//GLSYM( glCopyTexImage2D );
	//GLSYM( glCopyTexSubImage2D );
	GLDIRECTSYMS( glCreateProgram );
	GLDIRECTSYMS( glCreateShader );
	GLSYM( glCullFace );
	GLSYM( glDeleteBuffers );
	GLSYM( glDeleteFramebuffers );
	GLDIRECTSYMS( glDeleteProgram );
	GLSYM( glDeleteRenderbuffers );
	GLDIRECTSYMS( glDeleteShader );
	GLSYM( glDeleteTextures );
	GLSYM( glDepthFunc );
	GLSYM( glDepthMask );
#ifdef glDepthRange
	GLSYM( glDepthRange );
#else
#if 0 //#ifdef _DEBUG
	qglDepthRange = dlsym( qnx.openGLLib, "glDepthRangef" );
#else
	qglDepthRange = glDepthRangef;
#endif
	GLSYMS( glDepthRange );
#endif
	GLDIRECTSYMS( glDetachShader );
	GLSYM( glDisable );
	GLSYM( glDisableVertexAttribArray );
	GLSYM( glDrawArrays );
	GLSYM( glDrawElements );
	GLSYM( glEnable );
	GLSYM( glEnableVertexAttribArray );
	GLSYM( glFinish );
	GLSYM( glFlush );
	GLSYM( glFramebufferRenderbuffer );
	GLSYM( glFramebufferTexture2D );
	GLSYM( glFrontFace );
	GLSYM( glGenBuffers );
	GLSYM( glGenerateMipmap );
	GLSYM( glGenFramebuffers );
	GLSYM( glGenRenderbuffers );
	GLSYM( glGenTextures );
	GLSYM( glGetActiveAttrib );
	GLSYM( glGetActiveUniform );
	GLSYM( glGetAttachedShaders );
	GLSYM( glGetAttribLocation );
	GLSYM( glGetBooleanv );
	GLSYM( glGetBufferParameteriv );
	GLSYM( glGetError );
	GLSYM( glGetFloatv );
	GLSYM( glGetFramebufferAttachmentParameteriv );
	GLSYM( glGetIntegerv );
	GLDIRECTSYMS( glGetProgramiv );
	GLDIRECTSYMS( glGetProgramInfoLog );
	GLSYM( glGetRenderbufferParameteriv );
	GLDIRECTSYMS( glGetShaderiv );
	GLDIRECTSYMS( glGetShaderInfoLog );
	GLSYM( glGetShaderPrecisionFormat );
	GLSYM( glGetShaderSource );
	GLSYM( glGetString );
	GLSYM( glGetTexParameterfv );
	GLSYM( glGetTexParameteriv );
	GLSYM( glGetUniformfv );
	GLSYM( glGetUniformiv );
	GLDIRECTSYMS( glGetUniformLocation );
	GLSYM( glGetVertexAttribfv );
	GLSYM( glGetVertexAttribiv );
	GLSYM( glGetVertexAttribPointerv );
	GLSYM( glHint );
	GLSYM( glIsBuffer );
	GLSYM( glIsEnabled );
	GLSYM( glIsFramebuffer );
	GLSYM( glIsProgram );
	GLSYM( glIsRenderbuffer );
	GLSYM( glIsShader );
	GLSYM( glIsTexture );
	GLSYM( glLineWidth );
	GLDIRECTSYMS( glLinkProgram );
	GLSYM( glPixelStorei );
	GLSYM( glPolygonOffset );
	//GLSYM( glReadPixels );
	GLSYM( glReleaseShaderCompiler );
	GLSYM( glRenderbufferStorage );
	GLSYM( glSampleCoverage );
	GLSYM( glScissor );
	GLSYM( glShaderBinary );
	GLDIRECTSYMS( glShaderSource );
	GLSYM( glStencilFunc );
	GLDIRECTSYMS( glStencilFuncSeparate );
	GLSYM( glStencilMask );
	GLSYM( glStencilMaskSeparate );
	GLSYM( glStencilOp );
	GLDIRECTSYMS( glStencilOpSeparate );
	GLSYM( glTexImage2D );
	GLSYM( glTexParameterf );
	GLSYM( glTexParameterfv );
	GLSYM( glTexParameteri );
	GLSYM( glTexParameteriv );
	GLSYM( glTexSubImage2D );
	GLSYM( glUniform1f );
	GLSYM( glUniform1fv );
	GLDIRECTSYMS( glUniform1i );
	GLSYM( glUniform1iv );
	GLSYM( glUniform2f );
	GLSYM( glUniform2fv );
	GLSYM( glUniform2i );
	GLSYM( glUniform2iv );
	GLSYM( glUniform3f );
	GLSYM( glUniform3fv );
	GLSYM( glUniform3i );
	GLSYM( glUniform3iv );
	GLSYM( glUniform4f );
	GLDIRECTSYMS( glUniform4fv );
	GLSYM( glUniform4i );
	GLSYM( glUniform4iv );
	GLSYM( glUniformMatrix2fv );
	GLSYM( glUniformMatrix3fv );
	GLSYM( glUniformMatrix4fv );
	GLDIRECTSYMS( glUseProgram );
	GLSYM( glValidateProgram );
	GLSYM( glVertexAttrib1f );
	GLSYM( glVertexAttrib1fv );
	GLSYM( glVertexAttrib2f );
	GLSYM( glVertexAttrib2fv );
	GLSYM( glVertexAttrib3f );
	GLSYM( glVertexAttrib3fv );
	GLSYM( glVertexAttrib4f );
	GLSYM( glVertexAttrib4fv );
	GLSYM( glVertexAttribPointer );
	GLSYM( glViewport );

#ifdef GL_ES_VERSION_3_0

	glVersion = 3.0f;

	//GLDIRECTSYMS( glReadBuffer );
	GLSYM( glDrawRangeElements );
	GLSYM( glTexImage3D );
	GLSYM( glTexSubImage3D );
	GLSYM( glCopyTexSubImage3D );
	GLSYM( glCompressedTexImage3D );
	GLSYM( glCompressedTexSubImage3D );
	GLSYM( glGenQueries );
	GLSYM( glDeleteQueries );
	GLSYM( glIsQuery );
	GLSYM( glBeginQuery );
	GLSYM( glEndQuery );
	GLSYM( glGetQueryiv );
	GLSYM( glGetQueryObjectuiv );
	GLSYM( glUnmapBuffer );
	GLSYM( glGetBufferPointerv );
	GLSYM( glDrawBuffers );
	GLSYM( glUniformMatrix2x3fv );
	GLSYM( glUniformMatrix3x2fv );
	GLSYM( glUniformMatrix2x4fv );
	GLSYM( glUniformMatrix4x2fv );
	GLSYM( glUniformMatrix3x4fv );
	GLSYM( glUniformMatrix4x3fv );
	GLSYM( glBlitFramebuffer );
	GLSYM( glRenderbufferStorageMultisample );
	GLSYM( glFramebufferTextureLayer );
	GLDIRECTSYMS( glMapBufferRange );
	GLSYM( glFlushMappedBufferRange );
	GLDIRECTSYMS( glBindVertexArray );
	GLDIRECTSYMS( glDeleteVertexArrays );
	GLDIRECTSYMS( glGenVertexArrays );
	GLSYM( glIsVertexArray );
	GLSYM( glGetIntegeri_v );
	GLSYM( glBeginTransformFeedback );
	GLSYM( glEndTransformFeedback );
	GLDIRECTSYMS( glBindBufferRange );
	GLSYM( glBindBufferBase );
	GLSYM( glTransformFeedbackVaryings );
	GLSYM( glGetTransformFeedbackVarying );
	GLSYM( glVertexAttribIPointer );
	GLSYM( glGetVertexAttribIiv );
	GLSYM( glGetVertexAttribIuiv );
	GLSYM( glVertexAttribI4i );
	GLSYM( glVertexAttribI4ui );
	GLSYM( glVertexAttribI4iv );
	GLSYM( glVertexAttribI4uiv );
	GLSYM( glGetUniformuiv );
	GLSYM( glGetFragDataLocation );
	GLSYM( glUniform1ui );
	GLSYM( glUniform2ui );
	GLSYM( glUniform3ui );
	GLSYM( glUniform4ui );
	GLSYM( glUniform1uiv );
	GLSYM( glUniform2uiv );
	GLSYM( glUniform3uiv );
	GLSYM( glUniform4uiv );
	GLSYM( glClearBufferiv );
	GLSYM( glClearBufferuiv );
	GLSYM( glClearBufferfv );
	GLSYM( glClearBufferfi );
	GLDIRECTSYMS( glGetStringi );
	GLSYM( glCopyBufferSubData );
	GLSYM( glGetUniformIndices );
	GLSYM( glGetActiveUniformsiv );
	GLDIRECTSYMS( glGetUniformBlockIndex );
	GLSYM( glGetActiveUniformBlockiv );
	GLSYM( glGetActiveUniformBlockName );
	GLDIRECTSYMS( glUniformBlockBinding );
	GLSYM( glDrawArraysInstanced );
	GLSYM( glDrawElementsInstanced );
	GLDIRECTSYMS( glFenceSync );
	GLDIRECTSYMS( glIsSync );
	GLDIRECTSYMS( glDeleteSync );
	GLDIRECTSYMS( glClientWaitSync );
	GLSYM( glWaitSync );
	GLSYM( glGetInteger64v );
	GLSYM( glGetSynciv );
	GLSYM( glGetInteger64i_v );
	GLSYM( glGetBufferParameteri64v );
	GLSYM( glGenSamplers );
	GLSYM( glDeleteSamplers );
	GLSYM( glIsSampler );
	GLSYM( glBindSampler );
	GLSYM( glSamplerParameteri );
	GLSYM( glSamplerParameteriv );
	GLSYM( glSamplerParameterf );
	GLSYM( glSamplerParameterfv );
	GLSYM( glGetSamplerParameteriv );
	GLSYM( glGetSamplerParameterfv );
	GLSYM( glVertexAttribDivisor );
	GLSYM( glBindTransformFeedback );
	GLSYM( glDeleteTransformFeedbacks );
	GLSYM( glGenTransformFeedbacks );
	GLSYM( glIsTransformFeedback );
	GLSYM( glPauseTransformFeedback );
	GLSYM( glResumeTransformFeedback );
	GLSYM( glGetProgramBinary );
	GLSYM( glProgramBinary );
	GLDIRECTSYMS( glProgramParameteri );
	GLSYM( glInvalidateFramebuffer );
	GLSYM( glInvalidateSubFramebuffer );
	GLSYM( glTexStorage2D );
	GLSYM( glTexStorage3D );
	GLSYM( glGetInternalformativ );

#endif

	glVersion = 0.0f;

	EGLSYM( eglGetError );
	EGLSYM( eglGetDisplay );
	EGLSYM( eglInitialize );
	EGLSYM( eglTerminate );
	EGLSYM( eglQueryString );
	EGLSYM( eglGetConfigs );
	EGLSYM( eglChooseConfig );
	EGLSYM( eglGetConfigAttrib );
	EGLSYM( eglCreateWindowSurface );
	EGLSYM( eglCreatePbufferSurface );
	EGLSYM( eglCreatePixmapSurface );
	EGLSYM( eglDestroySurface );
	EGLSYM( eglQuerySurface );
	EGLSYM( eglBindAPI );
	EGLSYM( eglQueryAPI );
	EGLSYM( eglWaitClient );
	EGLSYM( eglReleaseThread );
	EGLSYM( eglCreatePbufferFromClientBuffer );
	EGLSYM( eglSurfaceAttrib );
	EGLSYM( eglBindTexImage );
	EGLSYM( eglReleaseTexImage );
	EGLSYM( eglSwapInterval );
	EGLSYM( eglCreateContext );
	EGLSYM( eglDestroyContext );
	EGLSYM( eglMakeCurrent );
	EGLSYM( eglGetCurrentContext );
	EGLSYM( eglGetCurrentSurface );
	EGLSYM( eglGetCurrentDisplay );
	EGLSYM( eglQueryContext );
	EGLSYM( eglWaitGL );
	EGLSYM( eglWaitNative );
	EGLSYM( eglSwapBuffers );
	EGLSYM( eglCopyBuffers );
	EGLSYM( eglGetProcAddress );

	return true;
}

/*
==================
OGL_UpdateReplacements
==================
*/
void OGL_UpdateReplacements( const EGLFunctionReplacements_t & replacements ) {

	assert( replacements.glReadBufferImpl && replacements.glDrawBufferImpl );

	float glVersion = 2.0f;

	GLSYMR_NULL( glReadBuffer, replacements, ((void)qglReadBuffer) );
	GLSYMR_NULL( glDrawBuffer, replacements, ((void)qglDrawBuffer) );
	GLSYMR( glReadPixels, replacements );
	GLSYMR( glCopyTexImage2D, replacements );
	GLSYMR( glCopyTexSubImage2D, replacements );

#ifdef GL_ES_VERSION_3_0

	glVersion = 3.0f;

	GLSYMR( glCopyTexSubImage3D, replacements );

#endif
}

/*
==================
GLimp_EnableLogging
==================
*/
void GLimp_EnableLogging( bool enable ) {
	// From EGL debug library: https://github.com/rcmaniac25/EGLdb
	if ( !qnx.glLoggingInit ) {
		qeglDebugEnableLoggingFunc = ( void (*)(EGLBoolean) )qeglGetProcAddress( "eglDebugEnableLogging" );
		qnx.glLoggingInit = true;
	}
	if ( qeglDebugEnableLoggingFunc ) {
		qeglDebugEnableLoggingFunc( enable );
	}
}
