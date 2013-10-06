/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.

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
** QGL_ES.H
*/

#ifndef __QGL_ES_H__
#define __QGL_ES_H__

#ifdef ID_OPENGL_ES
#include <EGL/egl.h>
#else
#error Trying to use OpenGL ES utility header in non-OpenGL ES program
#endif
#ifdef ID_OPENGL_ES_3
#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>
#elif defined(ID_OPENGL_ES_2)
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#else
#error Unknown OpenGL ES version
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifndef GL_APIENTRYP
#   define GL_APIENTRYP GL_APIENTRY*
#endif

//Some workarounds for OpenGL ES functionality

//=============
//General defines
//=============
#define GL_GLEXT_VERSION 1 //So that glext.h isn't included

//=============
//Types
//=============
#ifndef GLdouble
	typedef khronos_float_t GLdouble;
#endif
#ifndef GLclampd
	typedef khronos_float_t GLclampd;
#endif

//=============
//Defines for functions that are defined but may not actually exist
//=============
#ifndef PFNGLBINDMULTITEXTUREEXTPROC
	typedef void (GL_APIENTRYP PFNGLBINDMULTITEXTUREEXTPROC) (GLenum texunit, GLenum target, GLuint texture);
#endif

#ifndef PFNGLCOMPRESSEDTEXIMAGE2DARBPROC
	typedef void (GL_APIENTRYP PFNGLCOMPRESSEDTEXIMAGE2DARBPROC) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid *data);
#endif
#ifndef PFNGLCOMPRESSEDTEXSUBIMAGE2DARBPROC
	typedef void (GL_APIENTRYP PFNGLCOMPRESSEDTEXSUBIMAGE2DARBPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid *data);
#endif
#ifndef PFNGLGETCOMPRESSEDTEXIMAGEARBPROC
	typedef void (GL_APIENTRYP PFNGLGETCOMPRESSEDTEXIMAGEARBPROC) (GLenum target, GLint level, GLvoid *img);
#endif

#ifndef GL_ELEMENT_ARRAY_BUFFER_ARB
	#define GL_ELEMENT_ARRAY_BUFFER_ARB GL_ELEMENT_ARRAY_BUFFER
#endif
#ifndef GL_ARRAY_BUFFER_ARB
	#define GL_ARRAY_BUFFER_ARB GL_ARRAY_BUFFER
#endif
#ifndef PFNGLBINDBUFFERARBPROC
	typedef void (GL_APIENTRYP PFNGLBINDBUFFERARBPROC) (GLenum target, GLuint buffer);
#endif
#ifndef PFNGLBINDBUFFERRANGEPROC
	typedef void (GL_APIENTRYP PFNGLBINDBUFFERRANGEPROC) (GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size);
#endif
#ifndef PFNGLDELETEBUFFERSARBPROC
	typedef void (GL_APIENTRYP PFNGLDELETEBUFFERSARBPROC) (GLsizei n, const GLuint *buffers);
#endif
#ifndef PFNGLGENBUFFERSARBPROC
	typedef void (GL_APIENTRYP PFNGLGENBUFFERSARBPROC) (GLsizei n, GLuint *buffers);
#endif
#ifndef PFNGLISBUFFERARBPROC
	typedef GLboolean (GL_APIENTRYP PFNGLISBUFFERARBPROC) (GLuint buffer);
#endif
#ifndef PFNGLBUFFERDATAARBPROC
	typedef void (GL_APIENTRYP PFNGLBUFFERDATAARBPROC) (GLenum target, GLsizeiptr size, const GLvoid *data, GLenum usage);
#endif
#ifndef PFNGLBUFFERSUBDATAARBPROC
	typedef void (GL_APIENTRYP PFNGLBUFFERSUBDATAARBPROC) (GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid *data);
#endif
#ifndef PFNGLGETBUFFERSUBDATAARBPROC
	typedef void (GL_APIENTRYP PFNGLGETBUFFERSUBDATAARBPROC) (GLenum target, GLintptr offset, GLsizeiptr size, GLvoid *data);
#endif
#ifndef PFNGLMAPBUFFERARBPROC
	typedef GLvoid* (GL_APIENTRYP PFNGLMAPBUFFERARBPROC) (GLenum target, GLenum access);
#endif
#ifndef PFNGLUNMAPBUFFERARBPROC
	#ifdef PFNGLUNMAPBUFFEROESPROC
		typedef PFNGLUNMAPBUFFEROESPROC PFNGLUNMAPBUFFERARBPROC;
	#else
		typedef GLboolean (GL_APIENTRYP PFNGLUNMAPBUFFERARBPROC) (GLenum target);
	#endif
#endif
#ifndef PFNGLGETBUFFERPARAMETERIVARBPROC
	typedef void (GL_APIENTRYP PFNGLGETBUFFERPARAMETERIVARBPROC) (GLenum target, GLenum pname, GLint *params);
#endif
#ifndef PFNGLGETBUFFERPOINTERVARBPROC
	#ifdef PFNGLGETBUFFERPOINTERVOESPROC
		typedef PFNGLGETBUFFERPOINTERVOESPROC PFNGLGETBUFFERPOINTERVARBPROC;
	#else
		typedef void (GL_APIENTRYP PFNGLGETBUFFERPOINTERVARBPROC) (GLenum target, GLenum pname, GLvoid* *params);
	#endif
#endif

#ifndef PFNGLMAPBUFFERRANGEPROC
	#ifdef PFNGLMAPBUFFERRANGEEXTPROC
		typedef PFNGLMAPBUFFERRANGEEXTPROC PFNGLMAPBUFFERRANGEPROC;
	#else
		typedef GLvoid* (GL_APIENTRYP PFNGLMAPBUFFERRANGEPROC) (GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access);
	#endif
#endif

#ifndef PFNGLDRAWELEMENTSBASEVERTEXPROC
	typedef void (GL_APIENTRYP PFNGLDRAWELEMENTSBASEVERTEXPROC) (GLenum mode, GLsizei count, GLenum type, const GLvoid *indices, GLint basevertex);
#endif

#ifndef PFNGLGENVERTEXARRAYSPROC
	#ifdef PFNGLGENVERTEXARRAYSOESPROC
		typedef PFNGLGENVERTEXARRAYSOESPROC PFNGLGENVERTEXARRAYSPROC;
	#else
		typedef void (GL_APIENTRYP PFNGLGENVERTEXARRAYSPROC) (GLsizei n, GLuint *arrays);
	#endif
#endif
#ifndef PFNGLBINDVERTEXARRAYPROC
	#ifdef PFNGLBINDVERTEXARRAYOESPROC
		typedef PFNGLBINDVERTEXARRAYOESPROC PFNGLBINDVERTEXARRAYPROC;
	#else
		typedef void (GL_APIENTRYP PFNGLBINDVERTEXARRAYPROC) (GLuint array);
	#endif
#endif
#ifndef PFNGLDELETEVERTEXARRAYSPROC
	#ifdef PFNGLDELETEVERTEXARRAYSOESPROC
		typedef PFNGLDELETEVERTEXARRAYSOESPROC PFNGLDELETEVERTEXARRAYSPROC;
	#else
		typedef void (GL_APIENTRYP PFNGLDELETEVERTEXARRAYSPROC) (GLsizei n, const GLuint *arrays);
	#endif
#endif

#ifndef PFNGLVERTEXATTRIBPOINTERARBPROC
	typedef void (GL_APIENTRYP PFNGLVERTEXATTRIBPOINTERARBPROC) (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid *pointer);
#endif
#ifndef PFNGLENABLEVERTEXATTRIBARRAYARBPROC
	typedef void (GL_APIENTRYP PFNGLENABLEVERTEXATTRIBARRAYARBPROC) (GLuint index);
#endif
#ifndef PFNGLDISABLEVERTEXATTRIBARRAYARBPROC
	typedef void (GL_APIENTRYP PFNGLDISABLEVERTEXATTRIBARRAYARBPROC) (GLuint index);
#endif
#ifndef PFNGLPROGRAMSTRINGARBPROC
	typedef void (GL_APIENTRYP PFNGLPROGRAMSTRINGARBPROC) (GLenum target, GLenum format, GLsizei len, const GLvoid *string);
#endif
#ifndef PFNGLBINDPROGRAMARBPROC
	typedef void (GL_APIENTRYP PFNGLBINDPROGRAMARBPROC) (GLenum target, GLuint program);
#endif
#ifndef PFNGLGENPROGRAMSARBPROC
	typedef void (GL_APIENTRYP PFNGLGENPROGRAMSARBPROC) (GLsizei n, GLuint *programs);
#endif
#ifndef PFNGLDELETEPROGRAMSARBPROC
	typedef void (GL_APIENTRYP PFNGLDELETEPROGRAMSARBPROC) (GLsizei n, const GLuint *programs);
#endif
#ifndef PFNGLPROGRAMENVPARAMETER4FVARBPROC
	typedef void (GL_APIENTRYP PFNGLPROGRAMENVPARAMETER4FVARBPROC) (GLenum target, GLuint index, const GLfloat *params);
#endif
#ifndef PFNGLPROGRAMLOCALPARAMETER4FVARBPROC
	typedef void (GL_APIENTRYP PFNGLPROGRAMLOCALPARAMETER4FVARBPROC) (GLenum target, GLuint index, const GLfloat *params);
#endif

#ifndef PFNGLCREATESHADERPROC
	typedef GLuint (GL_APIENTRYP PFNGLCREATESHADERPROC) (GLenum type);
#endif
#ifndef PFNGLDELETESHADERPROC
	typedef void (GL_APIENTRYP PFNGLDELETESHADERPROC) (GLuint shader);
#endif
#ifndef PFNGLSHADERSOURCEPROC
	typedef void (GL_APIENTRYP PFNGLSHADERSOURCEPROC) (GLuint shader, GLsizei count, const GLchar* *string, const GLint *length);
#endif
#ifndef PFNGLCOMPILESHADERPROC
	typedef void (GL_APIENTRYP PFNGLCOMPILESHADERPROC) (GLuint shader);
#endif
#ifndef PFNGLGETSHADERIVPROC
	typedef void (GL_APIENTRYP PFNGLGETSHADERIVPROC) (GLuint shader, GLenum pname, GLint *params);
#endif
#ifndef PFNGLGETSHADERINFOLOGPROC
	typedef void (GL_APIENTRYP PFNGLGETSHADERINFOLOGPROC) (GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
#endif
#ifndef PFNGLCREATEPROGRAMPROC
	typedef GLuint (GL_APIENTRYP PFNGLCREATEPROGRAMPROC) (void);
#endif
#ifndef PFNGLDELETEPROGRAMPROC
	typedef void (GL_APIENTRYP PFNGLDELETEPROGRAMPROC) (GLuint program);
#endif
#ifndef PFNGLATTACHSHADERPROC
	typedef void (GL_APIENTRYP PFNGLATTACHSHADERPROC) (GLuint program, GLuint shader);
#endif
#ifndef PFNGLDETACHSHADERPROC
	typedef void (GL_APIENTRYP PFNGLDETACHSHADERPROC) (GLuint program, GLuint shader);
#endif
#ifndef PFNGLLINKPROGRAMPROC
	typedef void (GL_APIENTRYP PFNGLLINKPROGRAMPROC) (GLuint program);
#endif
#ifndef PFNGLUSEPROGRAMPROC
	typedef void (GL_APIENTRYP PFNGLUSEPROGRAMPROC) (GLuint program);
#endif
#ifndef PFNGLGETPROGRAMIVPROC
	typedef void (GL_APIENTRYP PFNGLGETPROGRAMIVPROC) (GLuint program, GLenum pname, GLint *params);
#endif
#ifndef PFNGLGETPROGRAMINFOLOGPROC
	typedef void (GL_APIENTRYP PFNGLGETPROGRAMINFOLOGPROC) (GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
#endif
#ifndef PFNGLPROGRAMPARAMETERIPROC
	typedef void (GL_APIENTRYP PFNGLPROGRAMPARAMETERIPROC) (GLuint program, GLenum pname, GLint value);
#endif
#ifndef PFNGLBINDATTRIBLOCATIONPROC
	typedef void (GL_APIENTRYP PFNGLBINDATTRIBLOCATIONPROC) (GLuint program, GLuint index, const GLchar *name);
#endif
#ifndef PFNGLGETUNIFORMLOCATIONPROC
	typedef GLint (GL_APIENTRYP PFNGLGETUNIFORMLOCATIONPROC) (GLuint program, const GLchar *name);
#endif
#ifndef PFNGLUNIFORM1IPROC
	typedef void (GL_APIENTRYP PFNGLUNIFORM1IPROC) (GLint location, GLint v0);
#endif
#ifndef PFNGLUNIFORM4FVPROC
	typedef void (GL_APIENTRYP PFNGLUNIFORM4FVPROC) (GLint location, GLsizei count, const GLfloat *value);
#endif

#ifndef PFNGLGETUNIFORMBLOCKINDEXPROC
	typedef GLuint (GL_APIENTRYP PFNGLGETUNIFORMBLOCKINDEXPROC) (GLuint program, const GLchar *uniformBlockName);
#endif
#ifndef PFNGLUNIFORMBLOCKBINDINGPROC
	typedef void (GL_APIENTRYP PFNGLUNIFORMBLOCKBINDINGPROC) (GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding);
#endif

#ifndef PFNGLSTENCILOPSEPARATEATIPROC
	typedef void (GL_APIENTRYP PFNGLSTENCILOPSEPARATEATIPROC) (GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass);
#endif
#ifndef PFNGLSTENCILFUNCSEPARATEATIPROC
	typedef void (GL_APIENTRYP PFNGLSTENCILFUNCSEPARATEATIPROC) (GLenum frontfunc, GLenum backfunc, GLint ref, GLuint mask);
#endif

#ifndef PFNGLDEPTHBOUNDSEXTPROC
	typedef void (GL_APIENTRYP PFNGLDEPTHBOUNDSEXTPROC) (GLclampd zmin, GLclampd zmax);
#endif

#ifndef PFNGLFENCESYNCPROC
	typedef GLsync (GL_APIENTRYP PFNGLFENCESYNCPROC) (GLenum condition, GLbitfield flags);
#endif
#ifndef PFNGLISSYNCPROC
	typedef GLboolean (GL_APIENTRYP PFNGLISSYNCPROC) (GLsync sync);
#endif
#ifndef PFNGLCLIENTWAITSYNCPROC
	typedef GLenum (GL_APIENTRYP PFNGLCLIENTWAITSYNCPROC) (GLsync sync, GLbitfield flags, GLuint64 timeout);
#endif
#ifndef PFNGLDELETESYNCPROC
	typedef void (GL_APIENTRYP PFNGLDELETESYNCPROC) (GLsync sync);
#endif

#ifndef PFNGLGENQUERIESARBPROC
	typedef void (GL_APIENTRYP PFNGLGENQUERIESARBPROC) (GLsizei n, GLuint *ids);
#endif
#ifndef PFNGLDELETEQUERIESARBPROC
	typedef void (GL_APIENTRYP PFNGLDELETEQUERIESARBPROC) (GLsizei n, const GLuint *ids);
#endif
#ifndef PFNGLISQUERYARBPROC
	typedef GLboolean (GL_APIENTRYP PFNGLISQUERYARBPROC) (GLuint id);
#endif
#ifndef PFNGLBEGINQUERYARBPROC
	typedef void (GL_APIENTRYP PFNGLBEGINQUERYARBPROC) (GLenum target, GLuint id);
#endif
#ifndef PFNGLENDQUERYARBPROC
	typedef void (GL_APIENTRYP PFNGLENDQUERYARBPROC) (GLenum target);
#endif
#ifndef PFNGLGETQUERYIVARBPROC
	typedef void (GL_APIENTRYP PFNGLGETQUERYIVARBPROC) (GLenum target, GLenum pname, GLint *params);
#endif
#ifndef PFNGLGETQUERYOBJECTIVARBPROC
	typedef void (GL_APIENTRYP PFNGLGETQUERYOBJECTIVARBPROC) (GLuint id, GLenum pname, GLint *params);
#endif
#ifndef PFNGLGETQUERYOBJECTUIVARBPROC
	typedef void (GL_APIENTRYP PFNGLGETQUERYOBJECTUIVARBPROC) (GLuint id, GLenum pname, GLuint *params);
#endif

#ifndef PFNGLGETQUERYOBJECTUI64VEXTPROC
	typedef void (GL_APIENTRYP PFNGLGETQUERYOBJECTUI64VEXTPROC) (GLuint id, GLenum pname, GLuint64 *params);
#endif

#ifndef PFNGLDEBUGMESSAGECONTROLARBPROC
	#ifdef PFNGLDEBUGMESSAGECONTROLPROC
		typedef PFNGLDEBUGMESSAGECONTROLPROC PFNGLDEBUGMESSAGECONTROLARBPROC;
	#else
		typedef void (GL_APIENTRYP PFNGLDEBUGMESSAGECONTROLARBPROC) (GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint *ids, GLboolean enabled);
	#endif
#endif
#ifndef PFNGLDEBUGMESSAGEINSERTARBPROC
	#ifdef PFNGLDEBUGMESSAGEINSERTPROC
		typedef PFNGLDEBUGMESSAGEINSERTPROC PFNGLDEBUGMESSAGEINSERTARBPROC;
	#else
		typedef void (GL_APIENTRYP PFNGLDEBUGMESSAGEINSERTARBPROC) (GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *buf);
	#endif
#endif
#ifndef PFNGLDEBUGMESSAGECALLBACKARBPROC
	#ifndef GLDEBUGPROC
		typedef void (GL_APIENTRYP GLDEBUGPROCARB)(GLenum source,GLenum type,GLuint id,GLenum severity,GLsizei length,const GLchar *message,GLvoid *userParam);
	#endif
	#ifdef PFNGLDEBUGMESSAGECALLBACKPROC
		typedef PFNGLDEBUGMESSAGECALLBACKPROC PFNGLDEBUGMESSAGECALLBACKARBPROC;
	#else
		typedef void (GL_APIENTRYP PFNGLDEBUGMESSAGECALLBACKARBPROC) (GLDEBUGPROCARB callback, const GLvoid *userParam);
	#endif
#endif
#ifndef PFNGLGETDEBUGMESSAGELOGARBPROC
	#ifdef PFNGLGETDEBUGMESSAGELOGPROC
		typedef PFNGLGETDEBUGMESSAGELOGPROC PFNGLGETDEBUGMESSAGELOGARBPROC;
	#else
		typedef GLuint (GL_APIENTRYP PFNGLGETDEBUGMESSAGELOGARBPROC) (GLuint count, GLsizei bufsize, GLenum *sources, GLenum *types, GLuint *ids, GLenum *severities, GLsizei *lengths, GLchar *messageLog);
	#endif
#endif

#ifdef __cplusplus
}
#endif

#endif
