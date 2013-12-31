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
** QGL.H
*/

#ifndef __QGL_H__
#define __QGL_H__

#ifdef ID_OPENGL_ES
#include "qgl_es.h"
#elif defined(ID_OPENGL)
#include <gl/gl.h>
#else
#error Unknown OpenGL type
#endif


#ifndef APIENTRY
#define APIENTRY
#endif
#ifndef WINAPI
#define WINAPI
#endif

// only use local glext.h if we are not using the system one already
// http://oss.sgi.com/projects/ogl-sample/ABI/
#ifndef GL_GLEXT_VERSION

#include "glext.h"

#endif

typedef void (*GLExtension_t)(void);

#ifdef __cplusplus
	extern "C" {
#endif

GLExtension_t GLimp_ExtensionPointer( const char *name );

#ifdef __cplusplus
	}
#endif

// GL_EXT_direct_state_access
extern PFNGLBINDMULTITEXTUREEXTPROC			qglBindMultiTextureEXT;

// GL_ARB_texture_compression
extern PFNGLCOMPRESSEDTEXIMAGE2DARBPROC		qglCompressedTexImage2DARB;
extern PFNGLCOMPRESSEDTEXSUBIMAGE2DARBPROC	qglCompressedTexSubImage2DARB;
extern PFNGLGETCOMPRESSEDTEXIMAGEARBPROC	qglGetCompressedTexImageARB;

// GL_ARB_vertex_buffer_object
extern PFNGLBINDBUFFERARBPROC				qglBindBufferARB;
extern PFNGLBINDBUFFERRANGEPROC				qglBindBufferRange;
extern PFNGLDELETEBUFFERSARBPROC			qglDeleteBuffersARB;
extern PFNGLGENBUFFERSARBPROC				qglGenBuffersARB;
extern PFNGLISBUFFERARBPROC					qglIsBufferARB;
extern PFNGLBUFFERDATAARBPROC				qglBufferDataARB;
extern PFNGLBUFFERSUBDATAARBPROC			qglBufferSubDataARB;
extern PFNGLGETBUFFERSUBDATAARBPROC			qglGetBufferSubDataARB;
extern PFNGLMAPBUFFERARBPROC				qglMapBufferARB;
extern PFNGLUNMAPBUFFERARBPROC				qglUnmapBufferARB;
extern PFNGLGETBUFFERPARAMETERIVARBPROC		qglGetBufferParameterivARB;
extern PFNGLGETBUFFERPOINTERVARBPROC		qglGetBufferPointervARB;

// GL_ARB_map_buffer_Range
extern PFNGLMAPBUFFERRANGEPROC				qglMapBufferRange;

// GL_ARB_draw_elements_base_vertex
extern PFNGLDRAWELEMENTSBASEVERTEXPROC		qglDrawElementsBaseVertex;

// GL_ARB_vertex_array_object
extern PFNGLGENVERTEXARRAYSPROC				qglGenVertexArrays;
extern PFNGLBINDVERTEXARRAYPROC				qglBindVertexArray;
extern PFNGLDELETEVERTEXARRAYSPROC			qglDeleteVertexArrays;

// GL_ARB_vertex_program / GL_ARB_fragment_program
extern PFNGLVERTEXATTRIBPOINTERARBPROC		qglVertexAttribPointerARB;
extern PFNGLENABLEVERTEXATTRIBARRAYARBPROC	qglEnableVertexAttribArrayARB;
extern PFNGLDISABLEVERTEXATTRIBARRAYARBPROC	qglDisableVertexAttribArrayARB;
extern PFNGLPROGRAMSTRINGARBPROC			qglProgramStringARB;
extern PFNGLBINDPROGRAMARBPROC				qglBindProgramARB;
extern PFNGLGENPROGRAMSARBPROC				qglGenProgramsARB;
extern PFNGLDELETEPROGRAMSARBPROC			qglDeleteProgramsARB;
extern PFNGLPROGRAMENVPARAMETER4FVARBPROC	qglProgramEnvParameter4fvARB;
extern PFNGLPROGRAMLOCALPARAMETER4FVARBPROC	qglProgramLocalParameter4fvARB;

// GLSL / OpenGL 2.0
extern PFNGLCREATESHADERPROC				qglCreateShader;
extern PFNGLDELETESHADERPROC				qglDeleteShader;
extern PFNGLSHADERSOURCEPROC				qglShaderSource;
extern PFNGLCOMPILESHADERPROC				qglCompileShader;
extern PFNGLGETSHADERIVPROC					qglGetShaderiv;
extern PFNGLGETSHADERINFOLOGPROC			qglGetShaderInfoLog;
extern PFNGLCREATEPROGRAMPROC				qglCreateProgram;
extern PFNGLDELETEPROGRAMPROC				qglDeleteProgram;
extern PFNGLATTACHSHADERPROC				qglAttachShader;
extern PFNGLDETACHSHADERPROC				qglDetachShader;
extern PFNGLLINKPROGRAMPROC					qglLinkProgram;
extern PFNGLUSEPROGRAMPROC					qglUseProgram;
extern PFNGLGETPROGRAMIVPROC				qglGetProgramiv;
extern PFNGLGETPROGRAMINFOLOGPROC			qglGetProgramInfoLog;
extern PFNGLPROGRAMPARAMETERIPROC			qglProgramParameteri;
extern PFNGLBINDATTRIBLOCATIONPROC			qglBindAttribLocation;
extern PFNGLGETUNIFORMLOCATIONPROC			qglGetUniformLocation;
extern PFNGLUNIFORM1IPROC					qglUniform1i;
extern PFNGLUNIFORM4FVPROC					qglUniform4fv;

// GL_ARB_uniform_buffer_object
extern PFNGLGETUNIFORMBLOCKINDEXPROC		qglGetUniformBlockIndex;
extern PFNGLUNIFORMBLOCKBINDINGPROC			qglUniformBlockBinding;

// GL_ATI_separate_stencil / OpenGL 2.0 separate stencil
extern PFNGLSTENCILOPSEPARATEATIPROC		qglStencilOpSeparate;
extern PFNGLSTENCILFUNCSEPARATEATIPROC		qglStencilFuncSeparate;

// GL_EXT_depth_bounds_test
extern PFNGLDEPTHBOUNDSEXTPROC              qglDepthBoundsEXT;

// GL_ARB_sync
extern PFNGLFENCESYNCPROC					qglFenceSync;
extern PFNGLISSYNCPROC						qglIsSync;
extern PFNGLCLIENTWAITSYNCPROC				qglClientWaitSync;
extern PFNGLDELETESYNCPROC					qglDeleteSync;

// GL_ARB_occlusion_query
extern PFNGLGENQUERIESARBPROC				qglGenQueriesARB;
extern PFNGLDELETEQUERIESARBPROC			qglDeleteQueriesARB;
extern PFNGLISQUERYARBPROC					qglIsQueryARB;
extern PFNGLBEGINQUERYARBPROC				qglBeginQueryARB;
extern PFNGLENDQUERYARBPROC					qglEndQueryARB;
extern PFNGLGETQUERYIVARBPROC				qglGetQueryivARB;
extern PFNGLGETQUERYOBJECTIVARBPROC			qglGetQueryObjectivARB;
extern PFNGLGETQUERYOBJECTUIVARBPROC		qglGetQueryObjectuivARB;

// GL_ARB_timer_query / GL_EXT_timer_query
extern PFNGLGETQUERYOBJECTUI64VEXTPROC		qglGetQueryObjectui64vEXT;

// GL_ARB_debug_output
extern PFNGLDEBUGMESSAGECONTROLARBPROC		qglDebugMessageControlARB;
extern PFNGLDEBUGMESSAGEINSERTARBPROC		qglDebugMessageInsertARB;
extern PFNGLDEBUGMESSAGECALLBACKARBPROC		qglDebugMessageCallbackARB;
extern PFNGLGETDEBUGMESSAGELOGARBPROC		qglGetDebugMessageLogARB;

#ifdef GL_ES_VERSION_2_0

// Special functions that will always be function pointers
extern  void ( APIENTRY * qglReadBuffer )(GLenum mode);
extern  void ( APIENTRY * qglDrawBuffer )(GLenum mode);
extern  void ( APIENTRY * qglReadPixels )(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid* pixels);
extern  void ( APIENTRY * qglCopyTexImage2D )(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
extern  void ( APIENTRY * qglCopyTexSubImage2D )(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);

#ifdef GL_ES_VERSION_3_0

extern  void ( APIENTRY * qglCopyTexSubImage3D )(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);

#endif

#endif

//===========================================================================

// non-windows systems will just redefine qgl* to gl*
#if defined( __APPLE__ ) || defined( ID_GL_HARDLINK )

#include "qgl_linked.h"

#else

#ifndef GL_ES_VERSION_2_0

// windows systems use a function pointer for each call so we can do our log file intercepts

extern  void ( APIENTRY * qglAccum )(GLenum op, GLfloat value);
extern  void ( APIENTRY * qglAlphaFunc )(GLenum func, GLclampf ref);
extern  GLboolean ( APIENTRY * qglAreTexturesResident )(GLsizei n, const GLuint *textures, GLboolean *residences);
extern  void ( APIENTRY * qglArrayElement )(GLint i);
extern  void ( APIENTRY * qglBegin )(GLenum mode);
extern  void ( APIENTRY * qglBindTexture )(GLenum target, GLuint texture);
extern  void ( APIENTRY * qglBitmap )(GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove, const GLubyte *bitmap);
extern  void ( APIENTRY * qglBlendFunc )(GLenum sfactor, GLenum dfactor);
extern  void ( APIENTRY * qglCallList )(GLuint list);
extern  void ( APIENTRY * qglCallLists )(GLsizei n, GLenum type, const GLvoid *lists);
extern  void ( APIENTRY * qglClear )(GLbitfield mask);
extern  void ( APIENTRY * qglClearAccum )(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
extern  void ( APIENTRY * qglClearColor )(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
extern  void ( APIENTRY * qglClearDepth )(GLclampd depth);
extern  void ( APIENTRY * qglClearIndex )(GLfloat c);
extern  void ( APIENTRY * qglClearStencil )(GLint s);
extern  void ( APIENTRY * qglClipPlane )(GLenum plane, const GLdouble *equation);
extern  void ( APIENTRY * qglColor3b )(GLbyte red, GLbyte green, GLbyte blue);
extern  void ( APIENTRY * qglColor3bv )(const GLbyte *v);
extern  void ( APIENTRY * qglColor3d )(GLdouble red, GLdouble green, GLdouble blue);
extern  void ( APIENTRY * qglColor3dv )(const GLdouble *v);
extern  void ( APIENTRY * qglColor3f )(GLfloat red, GLfloat green, GLfloat blue);
extern  void ( APIENTRY * qglColor3fv )(const GLfloat *v);
extern  void ( APIENTRY * qglColor3i )(GLint red, GLint green, GLint blue);
extern  void ( APIENTRY * qglColor3iv )(const GLint *v);
extern  void ( APIENTRY * qglColor3s )(GLshort red, GLshort green, GLshort blue);
extern  void ( APIENTRY * qglColor3sv )(const GLshort *v);
extern  void ( APIENTRY * qglColor3ub )(GLubyte red, GLubyte green, GLubyte blue);
extern  void ( APIENTRY * qglColor3ubv )(const GLubyte *v);
extern  void ( APIENTRY * qglColor3ui )(GLuint red, GLuint green, GLuint blue);
extern  void ( APIENTRY * qglColor3uiv )(const GLuint *v);
extern  void ( APIENTRY * qglColor3us )(GLushort red, GLushort green, GLushort blue);
extern  void ( APIENTRY * qglColor3usv )(const GLushort *v);
extern  void ( APIENTRY * qglColor4b )(GLbyte red, GLbyte green, GLbyte blue, GLbyte alpha);
extern  void ( APIENTRY * qglColor4bv )(const GLbyte *v);
extern  void ( APIENTRY * qglColor4d )(GLdouble red, GLdouble green, GLdouble blue, GLdouble alpha);
extern  void ( APIENTRY * qglColor4dv )(const GLdouble *v);
extern  void ( APIENTRY * qglColor4f )(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
extern  void ( APIENTRY * qglColor4fv )(const GLfloat *v);
extern  void ( APIENTRY * qglColor4i )(GLint red, GLint green, GLint blue, GLint alpha);
extern  void ( APIENTRY * qglColor4iv )(const GLint *v);
extern  void ( APIENTRY * qglColor4s )(GLshort red, GLshort green, GLshort blue, GLshort alpha);
extern  void ( APIENTRY * qglColor4sv )(const GLshort *v);
extern  void ( APIENTRY * qglColor4ub )(GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha);
extern  void ( APIENTRY * qglColor4ubv )(const GLubyte *v);
extern  void ( APIENTRY * qglColor4ui )(GLuint red, GLuint green, GLuint blue, GLuint alpha);
extern  void ( APIENTRY * qglColor4uiv )(const GLuint *v);
extern  void ( APIENTRY * qglColor4us )(GLushort red, GLushort green, GLushort blue, GLushort alpha);
extern  void ( APIENTRY * qglColor4usv )(const GLushort *v);
extern  void ( APIENTRY * qglColorMask )(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
extern  void ( APIENTRY * qglColorMaterial )(GLenum face, GLenum mode);
extern  void ( APIENTRY * qglColorPointer )(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
extern  void ( APIENTRY * qglCopyPixels )(GLint x, GLint y, GLsizei width, GLsizei height, GLenum type);
extern  void ( APIENTRY * qglCopyTexImage1D )(GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLint border);
extern  void ( APIENTRY * qglCopyTexImage2D )(GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
extern  void ( APIENTRY * qglCopyTexSubImage1D )(GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);
extern  void ( APIENTRY * qglCopyTexSubImage2D )(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
extern  void ( APIENTRY * qglCullFace )(GLenum mode);
extern  void ( APIENTRY * qglDeleteLists )(GLuint list, GLsizei range);
extern  void ( APIENTRY * qglDeleteTextures )(GLsizei n, const GLuint *textures);
extern  void ( APIENTRY * qglDepthFunc )(GLenum func);
extern  void ( APIENTRY * qglDepthMask )(GLboolean flag);
extern  void ( APIENTRY * qglDepthRange )(GLclampd zNear, GLclampd zFar);
extern  void ( APIENTRY * qglDisable )(GLenum cap);
extern  void ( APIENTRY * qglDisableClientState )(GLenum array);
extern  void ( APIENTRY * qglDrawArrays )(GLenum mode, GLint first, GLsizei count);
extern  void ( APIENTRY * qglDrawBuffer )(GLenum mode);
extern  void ( APIENTRY * qglDrawElements )(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices);
extern  void ( APIENTRY * qglDrawPixels )(GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
extern  void ( APIENTRY * qglEdgeFlag )(GLboolean flag);
extern  void ( APIENTRY * qglEdgeFlagPointer )(GLsizei stride, const GLvoid *pointer);
extern  void ( APIENTRY * qglEdgeFlagv )(const GLboolean *flag);
extern  void ( APIENTRY * qglEnable )(GLenum cap);
extern  void ( APIENTRY * qglEnableClientState )(GLenum array);
extern  void ( APIENTRY * qglEnd )(void);
extern  void ( APIENTRY * qglEndList )(void);
extern  void ( APIENTRY * qglEvalCoord1d )(GLdouble u);
extern  void ( APIENTRY * qglEvalCoord1dv )(const GLdouble *u);
extern  void ( APIENTRY * qglEvalCoord1f )(GLfloat u);
extern  void ( APIENTRY * qglEvalCoord1fv )(const GLfloat *u);
extern  void ( APIENTRY * qglEvalCoord2d )(GLdouble u, GLdouble v);
extern  void ( APIENTRY * qglEvalCoord2dv )(const GLdouble *u);
extern  void ( APIENTRY * qglEvalCoord2f )(GLfloat u, GLfloat v);
extern  void ( APIENTRY * qglEvalCoord2fv )(const GLfloat *u);
extern  void ( APIENTRY * qglEvalMesh1 )(GLenum mode, GLint i1, GLint i2);
extern  void ( APIENTRY * qglEvalMesh2 )(GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2);
extern  void ( APIENTRY * qglEvalPoint1 )(GLint i);
extern  void ( APIENTRY * qglEvalPoint2 )(GLint i, GLint j);
extern  void ( APIENTRY * qglFeedbackBuffer )(GLsizei size, GLenum type, GLfloat *buffer);
extern  void ( APIENTRY * qglFinish )(void);
extern  void ( APIENTRY * qglFlush )(void);
extern  void ( APIENTRY * qglFogf )(GLenum pname, GLfloat param);
extern  void ( APIENTRY * qglFogfv )(GLenum pname, const GLfloat *params);
extern  void ( APIENTRY * qglFogi )(GLenum pname, GLint param);
extern  void ( APIENTRY * qglFogiv )(GLenum pname, const GLint *params);
extern  void ( APIENTRY * qglFrontFace )(GLenum mode);
extern  void ( APIENTRY * qglFrustum )(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
extern  GLuint ( APIENTRY * qglGenLists )(GLsizei range);
extern  void ( APIENTRY * qglGenTextures )(GLsizei n, GLuint *textures);
extern  void ( APIENTRY * qglGetBooleanv )(GLenum pname, GLboolean *params);
extern  void ( APIENTRY * qglGetClipPlane )(GLenum plane, GLdouble *equation);
extern  void ( APIENTRY * qglGetDoublev )(GLenum pname, GLdouble *params);
extern  GLenum ( APIENTRY * qglGetError )(void);
extern  void ( APIENTRY * qglGetFloatv )(GLenum pname, GLfloat *params);
extern  void ( APIENTRY * qglGetIntegerv )(GLenum pname, GLint *params);
extern  void ( APIENTRY * qglGetLightfv )(GLenum light, GLenum pname, GLfloat *params);
extern  void ( APIENTRY * qglGetLightiv )(GLenum light, GLenum pname, GLint *params);
extern  void ( APIENTRY * qglGetMapdv )(GLenum target, GLenum query, GLdouble *v);
extern  void ( APIENTRY * qglGetMapfv )(GLenum target, GLenum query, GLfloat *v);
extern  void ( APIENTRY * qglGetMapiv )(GLenum target, GLenum query, GLint *v);
extern  void ( APIENTRY * qglGetMaterialfv )(GLenum face, GLenum pname, GLfloat *params);
extern  void ( APIENTRY * qglGetMaterialiv )(GLenum face, GLenum pname, GLint *params);
extern  void ( APIENTRY * qglGetPixelMapfv )(GLenum map, GLfloat *values);
extern  void ( APIENTRY * qglGetPixelMapuiv )(GLenum map, GLuint *values);
extern  void ( APIENTRY * qglGetPixelMapusv )(GLenum map, GLushort *values);
extern  void ( APIENTRY * qglGetPointerv )(GLenum pname, GLvoid* *params);
extern  void ( APIENTRY * qglGetPolygonStipple )(GLubyte *mask);
extern  const GLubyte * ( APIENTRY * qglGetString )(GLenum name);
extern  void ( APIENTRY * qglGetTexEnvfv )(GLenum target, GLenum pname, GLfloat *params);
extern  void ( APIENTRY * qglGetTexEnviv )(GLenum target, GLenum pname, GLint *params);
extern  void ( APIENTRY * qglGetTexGendv )(GLenum coord, GLenum pname, GLdouble *params);
extern  void ( APIENTRY * qglGetTexGenfv )(GLenum coord, GLenum pname, GLfloat *params);
extern  void ( APIENTRY * qglGetTexGeniv )(GLenum coord, GLenum pname, GLint *params);
extern  void ( APIENTRY * qglGetTexImage )(GLenum target, GLint level, GLenum format, GLenum type, GLvoid *pixels);
extern  void ( APIENTRY * qglGetTexLevelParameterfv )(GLenum target, GLint level, GLenum pname, GLfloat *params);
extern  void ( APIENTRY * qglGetTexLevelParameteriv )(GLenum target, GLint level, GLenum pname, GLint *params);
extern  void ( APIENTRY * qglGetTexParameterfv )(GLenum target, GLenum pname, GLfloat *params);
extern  void ( APIENTRY * qglGetTexParameteriv )(GLenum target, GLenum pname, GLint *params);
extern  void ( APIENTRY * qglHint )(GLenum target, GLenum mode);
extern  void ( APIENTRY * qglIndexMask )(GLuint mask);
extern  void ( APIENTRY * qglIndexPointer )(GLenum type, GLsizei stride, const GLvoid *pointer);
extern  void ( APIENTRY * qglIndexd )(GLdouble c);
extern  void ( APIENTRY * qglIndexdv )(const GLdouble *c);
extern  void ( APIENTRY * qglIndexf )(GLfloat c);
extern  void ( APIENTRY * qglIndexfv )(const GLfloat *c);
extern  void ( APIENTRY * qglIndexi )(GLint c);
extern  void ( APIENTRY * qglIndexiv )(const GLint *c);
extern  void ( APIENTRY * qglIndexs )(GLshort c);
extern  void ( APIENTRY * qglIndexsv )(const GLshort *c);
extern  void ( APIENTRY * qglIndexub )(GLubyte c);
extern  void ( APIENTRY * qglIndexubv )(const GLubyte *c);
extern  void ( APIENTRY * qglInitNames )(void);
extern  void ( APIENTRY * qglInterleavedArrays )(GLenum format, GLsizei stride, const GLvoid *pointer);
extern  GLboolean ( APIENTRY * qglIsEnabled )(GLenum cap);
extern  GLboolean ( APIENTRY * qglIsList )(GLuint list);
extern  GLboolean ( APIENTRY * qglIsTexture )(GLuint texture);
extern  void ( APIENTRY * qglLightModelf )(GLenum pname, GLfloat param);
extern  void ( APIENTRY * qglLightModelfv )(GLenum pname, const GLfloat *params);
extern  void ( APIENTRY * qglLightModeli )(GLenum pname, GLint param);
extern  void ( APIENTRY * qglLightModeliv )(GLenum pname, const GLint *params);
extern  void ( APIENTRY * qglLightf )(GLenum light, GLenum pname, GLfloat param);
extern  void ( APIENTRY * qglLightfv )(GLenum light, GLenum pname, const GLfloat *params);
extern  void ( APIENTRY * qglLighti )(GLenum light, GLenum pname, GLint param);
extern  void ( APIENTRY * qglLightiv )(GLenum light, GLenum pname, const GLint *params);
extern  void ( APIENTRY * qglLineStipple )(GLint factor, GLushort pattern);
extern  void ( APIENTRY * qglLineWidth )(GLfloat width);
extern  void ( APIENTRY * qglListBase )(GLuint base);
extern  void ( APIENTRY * qglLoadIdentity )(void);
extern  void ( APIENTRY * qglLoadMatrixd )(const GLdouble *m);
extern  void ( APIENTRY * qglLoadMatrixf )(const GLfloat *m);
extern  void ( APIENTRY * qglLoadName )(GLuint name);
extern  void ( APIENTRY * qglLogicOp )(GLenum opcode);
extern  void ( APIENTRY * qglMap1d )(GLenum target, GLdouble u1, GLdouble u2, GLint stride, GLint order, const GLdouble *points);
extern  void ( APIENTRY * qglMap1f )(GLenum target, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat *points);
extern  void ( APIENTRY * qglMap2d )(GLenum target, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, const GLdouble *points);
extern  void ( APIENTRY * qglMap2f )(GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, const GLfloat *points);
extern  void ( APIENTRY * qglMapGrid1d )(GLint un, GLdouble u1, GLdouble u2);
extern  void ( APIENTRY * qglMapGrid1f )(GLint un, GLfloat u1, GLfloat u2);
extern  void ( APIENTRY * qglMapGrid2d )(GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2);
extern  void ( APIENTRY * qglMapGrid2f )(GLint un, GLfloat u1, GLfloat u2, GLint vn, GLfloat v1, GLfloat v2);
extern  void ( APIENTRY * qglMaterialf )(GLenum face, GLenum pname, GLfloat param);
extern  void ( APIENTRY * qglMaterialfv )(GLenum face, GLenum pname, const GLfloat *params);
extern  void ( APIENTRY * qglMateriali )(GLenum face, GLenum pname, GLint param);
extern  void ( APIENTRY * qglMaterialiv )(GLenum face, GLenum pname, const GLint *params);
extern  void ( APIENTRY * qglMatrixMode )(GLenum mode);
extern  void ( APIENTRY * qglMultMatrixd )(const GLdouble *m);
extern  void ( APIENTRY * qglMultMatrixf )(const GLfloat *m);
extern  void ( APIENTRY * qglNewList )(GLuint list, GLenum mode);
extern  void ( APIENTRY * qglNormal3b )(GLbyte nx, GLbyte ny, GLbyte nz);
extern  void ( APIENTRY * qglNormal3bv )(const GLbyte *v);
extern  void ( APIENTRY * qglNormal3d )(GLdouble nx, GLdouble ny, GLdouble nz);
extern  void ( APIENTRY * qglNormal3dv )(const GLdouble *v);
extern  void ( APIENTRY * qglNormal3f )(GLfloat nx, GLfloat ny, GLfloat nz);
extern  void ( APIENTRY * qglNormal3fv )(const GLfloat *v);
extern  void ( APIENTRY * qglNormal3i )(GLint nx, GLint ny, GLint nz);
extern  void ( APIENTRY * qglNormal3iv )(const GLint *v);
extern  void ( APIENTRY * qglNormal3s )(GLshort nx, GLshort ny, GLshort nz);
extern  void ( APIENTRY * qglNormal3sv )(const GLshort *v);
extern  void ( APIENTRY * qglNormalPointer )(GLenum type, GLsizei stride, const GLvoid *pointer);
extern  void ( APIENTRY * qglOrtho )(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
extern  void ( APIENTRY * qglPassThrough )(GLfloat token);
extern  void ( APIENTRY * qglPixelMapfv )(GLenum map, GLsizei mapsize, const GLfloat *values);
extern  void ( APIENTRY * qglPixelMapuiv )(GLenum map, GLsizei mapsize, const GLuint *values);
extern  void ( APIENTRY * qglPixelMapusv )(GLenum map, GLsizei mapsize, const GLushort *values);
extern  void ( APIENTRY * qglPixelStoref )(GLenum pname, GLfloat param);
extern  void ( APIENTRY * qglPixelStorei )(GLenum pname, GLint param);
extern  void ( APIENTRY * qglPixelTransferf )(GLenum pname, GLfloat param);
extern  void ( APIENTRY * qglPixelTransferi )(GLenum pname, GLint param);
extern  void ( APIENTRY * qglPixelZoom )(GLfloat xfactor, GLfloat yfactor);
extern  void ( APIENTRY * qglPointSize )(GLfloat size);
extern  void ( APIENTRY * qglPolygonMode )(GLenum face, GLenum mode);
extern  void ( APIENTRY * qglPolygonOffset )(GLfloat factor, GLfloat units);
extern  void ( APIENTRY * qglPolygonStipple )(const GLubyte *mask);
extern  void ( APIENTRY * qglPopAttrib )(void);
extern  void ( APIENTRY * qglPopClientAttrib )(void);
extern  void ( APIENTRY * qglPopMatrix )(void);
extern  void ( APIENTRY * qglPopName )(void);
extern  void ( APIENTRY * qglPrioritizeTextures )(GLsizei n, const GLuint *textures, const GLclampf *priorities);
extern  void ( APIENTRY * qglPushAttrib )(GLbitfield mask);
extern  void ( APIENTRY * qglPushClientAttrib )(GLbitfield mask);
extern  void ( APIENTRY * qglPushMatrix )(void);
extern  void ( APIENTRY * qglPushName )(GLuint name);
extern  void ( APIENTRY * qglRasterPos2d )(GLdouble x, GLdouble y);
extern  void ( APIENTRY * qglRasterPos2dv )(const GLdouble *v);
extern  void ( APIENTRY * qglRasterPos2f )(GLfloat x, GLfloat y);
extern  void ( APIENTRY * qglRasterPos2fv )(const GLfloat *v);
extern  void ( APIENTRY * qglRasterPos2i )(GLint x, GLint y);
extern  void ( APIENTRY * qglRasterPos2iv )(const GLint *v);
extern  void ( APIENTRY * qglRasterPos2s )(GLshort x, GLshort y);
extern  void ( APIENTRY * qglRasterPos2sv )(const GLshort *v);
extern  void ( APIENTRY * qglRasterPos3d )(GLdouble x, GLdouble y, GLdouble z);
extern  void ( APIENTRY * qglRasterPos3dv )(const GLdouble *v);
extern  void ( APIENTRY * qglRasterPos3f )(GLfloat x, GLfloat y, GLfloat z);
extern  void ( APIENTRY * qglRasterPos3fv )(const GLfloat *v);
extern  void ( APIENTRY * qglRasterPos3i )(GLint x, GLint y, GLint z);
extern  void ( APIENTRY * qglRasterPos3iv )(const GLint *v);
extern  void ( APIENTRY * qglRasterPos3s )(GLshort x, GLshort y, GLshort z);
extern  void ( APIENTRY * qglRasterPos3sv )(const GLshort *v);
extern  void ( APIENTRY * qglRasterPos4d )(GLdouble x, GLdouble y, GLdouble z, GLdouble w);
extern  void ( APIENTRY * qglRasterPos4dv )(const GLdouble *v);
extern  void ( APIENTRY * qglRasterPos4f )(GLfloat x, GLfloat y, GLfloat z, GLfloat w);
extern  void ( APIENTRY * qglRasterPos4fv )(const GLfloat *v);
extern  void ( APIENTRY * qglRasterPos4i )(GLint x, GLint y, GLint z, GLint w);
extern  void ( APIENTRY * qglRasterPos4iv )(const GLint *v);
extern  void ( APIENTRY * qglRasterPos4s )(GLshort x, GLshort y, GLshort z, GLshort w);
extern  void ( APIENTRY * qglRasterPos4sv )(const GLshort *v);
extern  void ( APIENTRY * qglReadBuffer )(GLenum mode);
extern  void ( APIENTRY * qglReadPixels )(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels);
extern  void ( APIENTRY * qglRectd )(GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2);
extern  void ( APIENTRY * qglRectdv )(const GLdouble *v1, const GLdouble *v2);
extern  void ( APIENTRY * qglRectf )(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2);
extern  void ( APIENTRY * qglRectfv )(const GLfloat *v1, const GLfloat *v2);
extern  void ( APIENTRY * qglRecti )(GLint x1, GLint y1, GLint x2, GLint y2);
extern  void ( APIENTRY * qglRectiv )(const GLint *v1, const GLint *v2);
extern  void ( APIENTRY * qglRects )(GLshort x1, GLshort y1, GLshort x2, GLshort y2);
extern  void ( APIENTRY * qglRectsv )(const GLshort *v1, const GLshort *v2);
extern  GLint ( APIENTRY * qglRenderMode )(GLenum mode);
extern  void ( APIENTRY * qglRotated )(GLdouble angle, GLdouble x, GLdouble y, GLdouble z);
extern  void ( APIENTRY * qglRotatef )(GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
extern  void ( APIENTRY * qglScaled )(GLdouble x, GLdouble y, GLdouble z);
extern  void ( APIENTRY * qglScalef )(GLfloat x, GLfloat y, GLfloat z);
extern  void ( APIENTRY * qglScissor )(GLint x, GLint y, GLsizei width, GLsizei height);
extern  void ( APIENTRY * qglSelectBuffer )(GLsizei size, GLuint *buffer);
extern  void ( APIENTRY * qglShadeModel )(GLenum mode);
extern  void ( APIENTRY * qglStencilFunc )(GLenum func, GLint ref, GLuint mask);
extern  void ( APIENTRY * qglStencilMask )(GLuint mask);
extern  void ( APIENTRY * qglStencilOp )(GLenum fail, GLenum zfail, GLenum zpass);
extern  void ( APIENTRY * qglTexCoord1d )(GLdouble s);
extern  void ( APIENTRY * qglTexCoord1dv )(const GLdouble *v);
extern  void ( APIENTRY * qglTexCoord1f )(GLfloat s);
extern  void ( APIENTRY * qglTexCoord1fv )(const GLfloat *v);
extern  void ( APIENTRY * qglTexCoord1i )(GLint s);
extern  void ( APIENTRY * qglTexCoord1iv )(const GLint *v);
extern  void ( APIENTRY * qglTexCoord1s )(GLshort s);
extern  void ( APIENTRY * qglTexCoord1sv )(const GLshort *v);
extern  void ( APIENTRY * qglTexCoord2d )(GLdouble s, GLdouble t);
extern  void ( APIENTRY * qglTexCoord2dv )(const GLdouble *v);
extern  void ( APIENTRY * qglTexCoord2f )(GLfloat s, GLfloat t);
extern  void ( APIENTRY * qglTexCoord2fv )(const GLfloat *v);
extern  void ( APIENTRY * qglTexCoord2i )(GLint s, GLint t);
extern  void ( APIENTRY * qglTexCoord2iv )(const GLint *v);
extern  void ( APIENTRY * qglTexCoord2s )(GLshort s, GLshort t);
extern  void ( APIENTRY * qglTexCoord2sv )(const GLshort *v);
extern  void ( APIENTRY * qglTexCoord3d )(GLdouble s, GLdouble t, GLdouble r);
extern  void ( APIENTRY * qglTexCoord3dv )(const GLdouble *v);
extern  void ( APIENTRY * qglTexCoord3f )(GLfloat s, GLfloat t, GLfloat r);
extern  void ( APIENTRY * qglTexCoord3fv )(const GLfloat *v);
extern  void ( APIENTRY * qglTexCoord3i )(GLint s, GLint t, GLint r);
extern  void ( APIENTRY * qglTexCoord3iv )(const GLint *v);
extern  void ( APIENTRY * qglTexCoord3s )(GLshort s, GLshort t, GLshort r);
extern  void ( APIENTRY * qglTexCoord3sv )(const GLshort *v);
extern  void ( APIENTRY * qglTexCoord4d )(GLdouble s, GLdouble t, GLdouble r, GLdouble q);
extern  void ( APIENTRY * qglTexCoord4dv )(const GLdouble *v);
extern  void ( APIENTRY * qglTexCoord4f )(GLfloat s, GLfloat t, GLfloat r, GLfloat q);
extern  void ( APIENTRY * qglTexCoord4fv )(const GLfloat *v);
extern  void ( APIENTRY * qglTexCoord4i )(GLint s, GLint t, GLint r, GLint q);
extern  void ( APIENTRY * qglTexCoord4iv )(const GLint *v);
extern  void ( APIENTRY * qglTexCoord4s )(GLshort s, GLshort t, GLshort r, GLshort q);
extern  void ( APIENTRY * qglTexCoord4sv )(const GLshort *v);
extern  void ( APIENTRY * qglTexCoordPointer )(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
extern  void ( APIENTRY * qglTexEnvf )(GLenum target, GLenum pname, GLfloat param);
extern  void ( APIENTRY * qglTexEnvfv )(GLenum target, GLenum pname, const GLfloat *params);
extern  void ( APIENTRY * qglTexEnvi )(GLenum target, GLenum pname, GLint param);
extern  void ( APIENTRY * qglTexEnviv )(GLenum target, GLenum pname, const GLint *params);
extern  void ( APIENTRY * qglTexGend )(GLenum coord, GLenum pname, GLdouble param);
extern  void ( APIENTRY * qglTexGendv )(GLenum coord, GLenum pname, const GLdouble *params);
extern  void ( APIENTRY * qglTexGenf )(GLenum coord, GLenum pname, GLfloat param);
extern  void ( APIENTRY * qglTexGenfv )(GLenum coord, GLenum pname, const GLfloat *params);
extern  void ( APIENTRY * qglTexGeni )(GLenum coord, GLenum pname, GLint param);
extern  void ( APIENTRY * qglTexGeniv )(GLenum coord, GLenum pname, const GLint *params);
extern  void ( APIENTRY * qglTexImage1D )(GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
extern  void ( APIENTRY * qglTexImage2D )(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
extern  void ( APIENTRY * qglTexParameterf )(GLenum target, GLenum pname, GLfloat param);
extern  void ( APIENTRY * qglTexParameterfv )(GLenum target, GLenum pname, const GLfloat *params);
extern  void ( APIENTRY * qglTexParameteri )(GLenum target, GLenum pname, GLint param);
extern  void ( APIENTRY * qglTexParameteriv )(GLenum target, GLenum pname, const GLint *params);
extern  void ( APIENTRY * qglTexSubImage1D )(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid *pixels);
extern  void ( APIENTRY * qglTexSubImage2D )(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
extern  void ( APIENTRY * qglTranslated )(GLdouble x, GLdouble y, GLdouble z);
extern  void ( APIENTRY * qglTranslatef )(GLfloat x, GLfloat y, GLfloat z);
extern  void ( APIENTRY * qglVertex2d )(GLdouble x, GLdouble y);
extern  void ( APIENTRY * qglVertex2dv )(const GLdouble *v);
extern  void ( APIENTRY * qglVertex2f )(GLfloat x, GLfloat y);
extern  void ( APIENTRY * qglVertex2fv )(const GLfloat *v);
extern  void ( APIENTRY * qglVertex2i )(GLint x, GLint y);
extern  void ( APIENTRY * qglVertex2iv )(const GLint *v);
extern  void ( APIENTRY * qglVertex2s )(GLshort x, GLshort y);
extern  void ( APIENTRY * qglVertex2sv )(const GLshort *v);
extern  void ( APIENTRY * qglVertex3d )(GLdouble x, GLdouble y, GLdouble z);
extern  void ( APIENTRY * qglVertex3dv )(const GLdouble *v);
extern  void ( APIENTRY * qglVertex3f )(GLfloat x, GLfloat y, GLfloat z);
extern  void ( APIENTRY * qglVertex3fv )(const GLfloat *v);
extern  void ( APIENTRY * qglVertex3i )(GLint x, GLint y, GLint z);
extern  void ( APIENTRY * qglVertex3iv )(const GLint *v);
extern  void ( APIENTRY * qglVertex3s )(GLshort x, GLshort y, GLshort z);
extern  void ( APIENTRY * qglVertex3sv )(const GLshort *v);
extern  void ( APIENTRY * qglVertex4d )(GLdouble x, GLdouble y, GLdouble z, GLdouble w);
extern  void ( APIENTRY * qglVertex4dv )(const GLdouble *v);
extern  void ( APIENTRY * qglVertex4f )(GLfloat x, GLfloat y, GLfloat z, GLfloat w);
extern  void ( APIENTRY * qglVertex4fv )(const GLfloat *v);
extern  void ( APIENTRY * qglVertex4i )(GLint x, GLint y, GLint z, GLint w);
extern  void ( APIENTRY * qglVertex4iv )(const GLint *v);
extern  void ( APIENTRY * qglVertex4s )(GLshort x, GLshort y, GLshort z, GLshort w);
extern  void ( APIENTRY * qglVertex4sv )(const GLshort *v);
extern  void ( APIENTRY * qglVertexPointer )(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
extern  void ( APIENTRY * qglViewport )(GLint x, GLint y, GLsizei width, GLsizei height);

#else

extern  void ( APIENTRY * qglActiveTexture )(GLenum texture);
//extern  void ( APIENTRY * qglAttachShader )(GLuint program, GLuint shader);
//extern  void ( APIENTRY * qglBindAttribLocation )(GLuint program, GLuint index, const GLchar* name);
extern  void ( APIENTRY * qglBindBuffer )(GLenum target, GLuint buffer);
extern  void ( APIENTRY * qglBindFramebuffer )(GLenum target, GLuint framebuffer);
extern  void ( APIENTRY * qglBindRenderbuffer )(GLenum target, GLuint renderbuffer);
extern  void ( APIENTRY * qglBindTexture )(GLenum target, GLuint texture);
extern  void ( APIENTRY * qglBlendColor )(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
extern  void ( APIENTRY * qglBlendEquation )(GLenum mode);
extern  void ( APIENTRY * qglBlendEquationSeparate )(GLenum modeRGB, GLenum modeAlpha);
extern  void ( APIENTRY * qglBlendFunc )(GLenum sfactor, GLenum dfactor);
extern  void ( APIENTRY * qglBlendFuncSeparate )(GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha);
extern  void ( APIENTRY * qglBufferData )(GLenum target, GLsizeiptr size, const GLvoid* data, GLenum usage);
extern  void ( APIENTRY * qglBufferSubData )(GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid* data);
extern  GLenum ( APIENTRY * qglCheckFramebufferStatus )(GLenum target);
extern  void ( APIENTRY * qglClear )(GLbitfield mask);
extern  void ( APIENTRY * qglClearColor )(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
extern  void ( APIENTRY * qglClearDepth )(GLclampd depth);
extern  void ( APIENTRY * qglClearStencil )(GLint s);
extern  void ( APIENTRY * qglColorMask )(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
//extern  void ( APIENTRY * qglCompileShader )(GLuint shader);
extern  void ( APIENTRY * qglCompressedTexImage2D )(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid* data);
extern  void ( APIENTRY * qglCompressedTexSubImage2D )(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid* data);
//extern  void ( APIENTRY * qglCopyTexImage2D )(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
//extern  void ( APIENTRY * qglCopyTexSubImage2D )(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
//extern  GLuint ( APIENTRY * qglCreateProgram )(void);
//extern  GLuint ( APIENTRY * qglCreateShader )(GLenum type);
extern  void ( APIENTRY * qglCullFace )(GLenum mode);
extern  void ( APIENTRY * qglDeleteBuffers )(GLsizei n, const GLuint* buffers);
extern  void ( APIENTRY * qglDeleteFramebuffers )(GLsizei n, const GLuint* framebuffers);
//extern  void ( APIENTRY * qglDeleteProgram )(GLuint program);
extern  void ( APIENTRY * qglDeleteRenderbuffers )(GLsizei n, const GLuint* renderbuffers);
//extern  void ( APIENTRY * qglDeleteShader )(GLuint shader);
extern  void ( APIENTRY * qglDeleteTextures )(GLsizei n, const GLuint* textures);
extern  void ( APIENTRY * qglDepthFunc )(GLenum func);
extern  void ( APIENTRY * qglDepthMask )(GLboolean flag);
extern  void ( APIENTRY * qglDepthRange )(GLclampd n, GLclampd f);
//extern  void ( APIENTRY * qglDetachShader )(GLuint program, GLuint shader);
extern  void ( APIENTRY * qglDisable )(GLenum cap);
extern  void ( APIENTRY * qglDisableVertexAttribArray )(GLuint index);
extern  void ( APIENTRY * qglDrawArrays )(GLenum mode, GLint first, GLsizei count);
extern  void ( APIENTRY * qglDrawElements )(GLenum mode, GLsizei count, GLenum type, const GLvoid* indices);
extern  void ( APIENTRY * qglEnable )(GLenum cap);
extern  void ( APIENTRY * qglEnableVertexAttribArray )(GLuint index);
extern  void ( APIENTRY * qglFinish )(void);
extern  void ( APIENTRY * qglFlush )(void);
extern  void ( APIENTRY * qglFramebufferRenderbuffer )(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
extern  void ( APIENTRY * qglFramebufferTexture2D )(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
extern  void ( APIENTRY * qglFrontFace )(GLenum mode);
extern  void ( APIENTRY * qglGenBuffers )(GLsizei n, GLuint* buffers);
extern  void ( APIENTRY * qglGenerateMipmap )(GLenum target);
extern  void ( APIENTRY * qglGenFramebuffers )(GLsizei n, GLuint* framebuffers);
extern  void ( APIENTRY * qglGenRenderbuffers )(GLsizei n, GLuint* renderbuffers);
extern  void ( APIENTRY * qglGenTextures )(GLsizei n, GLuint* textures);
extern  void ( APIENTRY * qglGetActiveAttrib )(GLuint program, GLuint index, GLsizei bufsize, GLsizei* length, GLint* size, GLenum* type, GLchar* name);
extern  void ( APIENTRY * qglGetActiveUniform )(GLuint program, GLuint index, GLsizei bufsize, GLsizei* length, GLint* size, GLenum* type, GLchar* name);
extern  void ( APIENTRY * qglGetAttachedShaders )(GLuint program, GLsizei maxcount, GLsizei* count, GLuint* shaders);
extern  GLint ( APIENTRY * qglGetAttribLocation )(GLuint program, const GLchar* name);
extern  void ( APIENTRY * qglGetBooleanv )(GLenum pname, GLboolean* params);
extern  void ( APIENTRY * qglGetBufferParameteriv )(GLenum target, GLenum pname, GLint* params);
extern  GLenum ( APIENTRY * qglGetError )(void);
extern  void ( APIENTRY * qglGetFloatv )(GLenum pname, GLfloat* params);
extern  void ( APIENTRY * qglGetFramebufferAttachmentParameteriv )(GLenum target, GLenum attachment, GLenum pname, GLint* params);
extern  void ( APIENTRY * qglGetIntegerv )(GLenum pname, GLint* params);
//extern  void ( APIENTRY * qglGetProgramiv )(GLuint program, GLenum pname, GLint* params);
//extern  void ( APIENTRY * qglGetProgramInfoLog )(GLuint program, GLsizei bufsize, GLsizei* length, GLchar* infolog);
extern  void ( APIENTRY * qglGetRenderbufferParameteriv )(GLenum target, GLenum pname, GLint* params);
//extern  void ( APIENTRY * qglGetShaderiv )(GLuint shader, GLenum pname, GLint* params);
//extern  void ( APIENTRY * qglGetShaderInfoLog )(GLuint shader, GLsizei bufsize, GLsizei* length, GLchar* infolog);
extern  void ( APIENTRY * qglGetShaderPrecisionFormat )(GLenum shadertype, GLenum precisiontype, GLint* range, GLint* precision);
extern  void ( APIENTRY * qglGetShaderSource )(GLuint shader, GLsizei bufsize, GLsizei* length, GLchar* source);
extern  const GLubyte* ( APIENTRY * qglGetString )(GLenum name);
extern  void ( APIENTRY * qglGetTexParameterfv )(GLenum target, GLenum pname, GLfloat* params);
extern  void ( APIENTRY * qglGetTexParameteriv )(GLenum target, GLenum pname, GLint* params);
extern  void ( APIENTRY * qglGetUniformfv )(GLuint program, GLint location, GLfloat* params);
extern  void ( APIENTRY * qglGetUniformiv )(GLuint program, GLint location, GLint* params);
//extern  GLint ( APIENTRY * qglGetUniformLocation )(GLuint program, const GLchar* name);
extern  void ( APIENTRY * qglGetVertexAttribfv )(GLuint index, GLenum pname, GLfloat* params);
extern  void ( APIENTRY * qglGetVertexAttribiv )(GLuint index, GLenum pname, GLint* params);
extern  void ( APIENTRY * qglGetVertexAttribPointerv )(GLuint index, GLenum pname, GLvoid** pointer);
extern  void ( APIENTRY * qglHint )(GLenum target, GLenum mode);
extern  GLboolean ( APIENTRY * qglIsBuffer )(GLuint buffer);
extern  GLboolean ( APIENTRY * qglIsEnabled )(GLenum cap);
extern  GLboolean ( APIENTRY * qglIsFramebuffer )(GLuint framebuffer);
extern  GLboolean ( APIENTRY * qglIsProgram )(GLuint program);
extern  GLboolean ( APIENTRY * qglIsRenderbuffer )(GLuint renderbuffer);
extern  GLboolean ( APIENTRY * qglIsShader )(GLuint shader);
extern  GLboolean ( APIENTRY * qglIsTexture )(GLuint texture);
extern  void ( APIENTRY * qglLineWidth )(GLfloat width);
//extern  void ( APIENTRY * qglLinkProgram )(GLuint program);
extern  void ( APIENTRY * qglPixelStorei )(GLenum pname, GLint param);
extern  void ( APIENTRY * qglPolygonOffset )(GLfloat factor, GLfloat units);
//extern  void ( APIENTRY * qglReadPixels )(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid* pixels);
extern  void ( APIENTRY * qglReleaseShaderCompiler )(void);
extern  void ( APIENTRY * qglRenderbufferStorage )(GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
extern  void ( APIENTRY * qglSampleCoverage )(GLfloat value, GLboolean invert);
extern  void ( APIENTRY * qglScissor )(GLint x, GLint y, GLsizei width, GLsizei height);
extern  void ( APIENTRY * qglShaderBinary )(GLsizei n, const GLuint* shaders, GLenum binaryformat, const GLvoid* binary, GLsizei length);
//extern  void ( APIENTRY * qglShaderSource )(GLuint shader, GLsizei count, const GLchar* const* string, const GLint* length);
extern  void ( APIENTRY * qglStencilFunc )(GLenum func, GLint ref, GLuint mask);
//extern  void ( APIENTRY * qglStencilFuncSeparate )(GLenum face, GLenum func, GLint ref, GLuint mask);
extern  void ( APIENTRY * qglStencilMask )(GLuint mask);
extern  void ( APIENTRY * qglStencilMaskSeparate )(GLenum face, GLuint mask);
extern  void ( APIENTRY * qglStencilOp )(GLenum fail, GLenum zfail, GLenum zpass);
//extern  void ( APIENTRY * qglStencilOpSeparate )(GLenum face, GLenum fail, GLenum zfail, GLenum zpass);
extern  void ( APIENTRY * qglTexImage2D )(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid* pixels);
extern  void ( APIENTRY * qglTexParameterf )(GLenum target, GLenum pname, GLfloat param);
extern  void ( APIENTRY * qglTexParameterfv )(GLenum target, GLenum pname, const GLfloat* params);
extern  void ( APIENTRY * qglTexParameteri )(GLenum target, GLenum pname, GLint param);
extern  void ( APIENTRY * qglTexParameteriv )(GLenum target, GLenum pname, const GLint* params);
extern  void ( APIENTRY * qglTexSubImage2D )(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid* pixels);
extern  void ( APIENTRY * qglUniform1f )(GLint location, GLfloat x);
extern  void ( APIENTRY * qglUniform1fv )(GLint location, GLsizei count, const GLfloat* v);
//extern  void ( APIENTRY * qglUniform1i )(GLint location, GLint x);
extern  void ( APIENTRY * qglUniform1iv )(GLint location, GLsizei count, const GLint* v);
extern  void ( APIENTRY * qglUniform2f )(GLint location, GLfloat x, GLfloat y);
extern  void ( APIENTRY * qglUniform2fv )(GLint location, GLsizei count, const GLfloat* v);
extern  void ( APIENTRY * qglUniform2i )(GLint location, GLint x, GLint y);
extern  void ( APIENTRY * qglUniform2iv )(GLint location, GLsizei count, const GLint* v);
extern  void ( APIENTRY * qglUniform3f )(GLint location, GLfloat x, GLfloat y, GLfloat z);
extern  void ( APIENTRY * qglUniform3fv )(GLint location, GLsizei count, const GLfloat* v);
extern  void ( APIENTRY * qglUniform3i )(GLint location, GLint x, GLint y, GLint z);
extern  void ( APIENTRY * qglUniform3iv )(GLint location, GLsizei count, const GLint* v);
extern  void ( APIENTRY * qglUniform4f )(GLint location, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
//extern  void ( APIENTRY * qglUniform4fv )(GLint location, GLsizei count, const GLfloat* v);
extern  void ( APIENTRY * qglUniform4i )(GLint location, GLint x, GLint y, GLint z, GLint w);
extern  void ( APIENTRY * qglUniform4iv )(GLint location, GLsizei count, const GLint* v);
extern  void ( APIENTRY * qglUniformMatrix2fv )(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
extern  void ( APIENTRY * qglUniformMatrix3fv )(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
extern  void ( APIENTRY * qglUniformMatrix4fv )(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
//extern  void ( APIENTRY * qglUseProgram )(GLuint program);
extern  void ( APIENTRY * qglValidateProgram )(GLuint program);
extern  void ( APIENTRY * qglVertexAttrib1f )(GLuint indx, GLfloat x);
extern  void ( APIENTRY * qglVertexAttrib1fv )(GLuint indx, const GLfloat* values);
extern  void ( APIENTRY * qglVertexAttrib2f )(GLuint indx, GLfloat x, GLfloat y);
extern  void ( APIENTRY * qglVertexAttrib2fv )(GLuint indx, const GLfloat* values);
extern  void ( APIENTRY * qglVertexAttrib3f )(GLuint indx, GLfloat x, GLfloat y, GLfloat z);
extern  void ( APIENTRY * qglVertexAttrib3fv )(GLuint indx, const GLfloat* values);
extern  void ( APIENTRY * qglVertexAttrib4f )(GLuint indx, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
extern  void ( APIENTRY * qglVertexAttrib4fv )(GLuint indx, const GLfloat* values);
extern  void ( APIENTRY * qglVertexAttribPointer )(GLuint indx, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* ptr);
extern  void ( APIENTRY * qglViewport )(GLint x, GLint y, GLsizei width, GLsizei height);

#ifdef GL_ES_VERSION_3_0

//extern  void ( APIENTRY * qglReadBuffer )(GLenum mode);
extern  void ( APIENTRY * qglDrawRangeElements )(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid* indices);
extern  void ( APIENTRY * qglTexImage3D )(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid* pixels);
extern  void ( APIENTRY * qglTexSubImage3D )(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid* pixels);
//extern  void ( APIENTRY * qglCopyTexSubImage3D )(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);
extern  void ( APIENTRY * qglCompressedTexImage3D )(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid* data);
extern  void ( APIENTRY * qglCompressedTexSubImage3D )(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid* data);
extern  void ( APIENTRY * qglGenQueries )(GLsizei n, GLuint* ids);
extern  void ( APIENTRY * qglDeleteQueries )(GLsizei n, const GLuint* ids);
extern  GLboolean ( APIENTRY * qglIsQuery )(GLuint id);
extern  void ( APIENTRY * qglBeginQuery )(GLenum target, GLuint id);
extern  void ( APIENTRY * qglEndQuery )(GLenum target);
extern  void ( APIENTRY * qglGetQueryiv )(GLenum target, GLenum pname, GLint* params);
extern  void ( APIENTRY * qglGetQueryObjectuiv )(GLuint id, GLenum pname, GLuint* params);
extern  GLboolean ( APIENTRY * qglUnmapBuffer )(GLenum target);
extern  void ( APIENTRY * qglGetBufferPointerv )(GLenum target, GLenum pname, GLvoid** params);
extern  void ( APIENTRY * qglDrawBuffers )(GLsizei n, const GLenum* bufs);
extern  void ( APIENTRY * qglUniformMatrix2x3fv )(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
extern  void ( APIENTRY * qglUniformMatrix3x2fv )(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
extern  void ( APIENTRY * qglUniformMatrix2x4fv )(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
extern  void ( APIENTRY * qglUniformMatrix4x2fv )(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
extern  void ( APIENTRY * qglUniformMatrix3x4fv )(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
extern  void ( APIENTRY * qglUniformMatrix4x3fv )(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
extern  void ( APIENTRY * qglBlitFramebuffer )(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);
extern  void ( APIENTRY * qglRenderbufferStorageMultisample )(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);
extern  void ( APIENTRY * qglFramebufferTextureLayer )(GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer);
extern  GLvoid* ( APIENTRY * qglMapBufferRange )(GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access);
extern  void ( APIENTRY * qglFlushMappedBufferRange )(GLenum target, GLintptr offset, GLsizeiptr length);
extern  void ( APIENTRY * qglBindVertexArray )(GLuint array);
extern  void ( APIENTRY * qglDeleteVertexArrays )(GLsizei n, const GLuint* arrays);
extern  void ( APIENTRY * qglGenVertexArrays )(GLsizei n, GLuint* arrays);
extern  GLboolean ( APIENTRY * qglIsVertexArray )(GLuint array);
extern  void ( APIENTRY * qglGetIntegeri_v )(GLenum target, GLuint index, GLint* data);
extern  void ( APIENTRY * qglBeginTransformFeedback )(GLenum primitiveMode);
extern  void ( APIENTRY * qglEndTransformFeedback )(void);
extern  void ( APIENTRY * qglBindBufferRange )(GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size);
extern  void ( APIENTRY * qglBindBufferBase )(GLenum target, GLuint index, GLuint buffer);
extern  void ( APIENTRY * qglTransformFeedbackVaryings )(GLuint program, GLsizei count, const GLchar* const* varyings, GLenum bufferMode);
extern  void ( APIENTRY * qglGetTransformFeedbackVarying )(GLuint program, GLuint index, GLsizei bufSize, GLsizei* length, GLsizei* size, GLenum* type, GLchar* name);
extern  void ( APIENTRY * qglVertexAttribIPointer )(GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid* pointer);
extern  void ( APIENTRY * qglGetVertexAttribIiv )(GLuint index, GLenum pname, GLint* params);
extern  void ( APIENTRY * qglGetVertexAttribIuiv )(GLuint index, GLenum pname, GLuint* params);
extern  void ( APIENTRY * qglVertexAttribI4i )(GLuint index, GLint x, GLint y, GLint z, GLint w);
extern  void ( APIENTRY * qglVertexAttribI4ui )(GLuint index, GLuint x, GLuint y, GLuint z, GLuint w);
extern  void ( APIENTRY * qglVertexAttribI4iv )(GLuint index, const GLint* v);
extern  void ( APIENTRY * qglVertexAttribI4uiv )(GLuint index, const GLuint* v);
extern  void ( APIENTRY * qglGetUniformuiv )(GLuint program, GLint location, GLuint* params);
extern  GLint ( APIENTRY * qglGetFragDataLocation )(GLuint program, const GLchar *name);
extern  void ( APIENTRY * qglUniform1ui )(GLint location, GLuint v0);
extern  void ( APIENTRY * qglUniform2ui )(GLint location, GLuint v0, GLuint v1);
extern  void ( APIENTRY * qglUniform3ui )(GLint location, GLuint v0, GLuint v1, GLuint v2);
extern  void ( APIENTRY * qglUniform4ui )(GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3);
extern  void ( APIENTRY * qglUniform1uiv )(GLint location, GLsizei count, const GLuint* value);
extern  void ( APIENTRY * qglUniform2uiv )(GLint location, GLsizei count, const GLuint* value);
extern  void ( APIENTRY * qglUniform3uiv )(GLint location, GLsizei count, const GLuint* value);
extern  void ( APIENTRY * qglUniform4uiv )(GLint location, GLsizei count, const GLuint* value);
extern  void ( APIENTRY * qglClearBufferiv )(GLenum buffer, GLint drawbuffer, const GLint* value);
extern  void ( APIENTRY * qglClearBufferuiv )(GLenum buffer, GLint drawbuffer, const GLuint* value);
extern  void ( APIENTRY * qglClearBufferfv )(GLenum buffer, GLint drawbuffer, const GLfloat* value);
extern  void ( APIENTRY * qglClearBufferfi )(GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil);
extern  const GLubyte* ( APIENTRY * qglGetStringi )(GLenum name, GLuint index);
extern  void ( APIENTRY * qglCopyBufferSubData )(GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size);
extern  void ( APIENTRY * qglGetUniformIndices )(GLuint program, GLsizei uniformCount, const GLchar* const* uniformNames, GLuint* uniformIndices);
extern  void ( APIENTRY * qglGetActiveUniformsiv )(GLuint program, GLsizei uniformCount, const GLuint* uniformIndices, GLenum pname, GLint* params);
//extern  GLuint ( APIENTRY * qglGetUniformBlockIndex )(GLuint program, const GLchar* uniformBlockName);
extern  void ( APIENTRY * qglGetActiveUniformBlockiv )(GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint* params);
extern  void ( APIENTRY * qglGetActiveUniformBlockName )(GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei* length, GLchar* uniformBlockName);
//extern  void ( APIENTRY * qglUniformBlockBinding )(GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding);
extern  void ( APIENTRY * qglDrawArraysInstanced )(GLenum mode, GLint first, GLsizei count, GLsizei instanceCount);
extern  void ( APIENTRY * qglDrawElementsInstanced )(GLenum mode, GLsizei count, GLenum type, const GLvoid* indices, GLsizei instanceCount);
//extern  GLsync ( APIENTRY * qglFenceSync )(GLenum condition, GLbitfield flags);
//extern  GLboolean ( APIENTRY * qglIsSync )(GLsync sync);
//extern  void ( APIENTRY * qglDeleteSync )(GLsync sync);
//extern  GLenum ( APIENTRY * qglClientWaitSync )(GLsync sync, GLbitfield flags, GLuint64 timeout);
extern  void ( APIENTRY * qglWaitSync )(GLsync sync, GLbitfield flags, GLuint64 timeout);
extern  void ( APIENTRY * qglGetInteger64v )(GLenum pname, GLint64* params);
extern  void ( APIENTRY * qglGetSynciv )(GLsync sync, GLenum pname, GLsizei bufSize, GLsizei* length, GLint* values);
extern  void ( APIENTRY * qglGetInteger64i_v )(GLenum target, GLuint index, GLint64* data);
extern  void ( APIENTRY * qglGetBufferParameteri64v )(GLenum target, GLenum pname, GLint64* params);
extern  void ( APIENTRY * qglGenSamplers )(GLsizei count, GLuint* samplers);
extern  void ( APIENTRY * qglDeleteSamplers )(GLsizei count, const GLuint* samplers);
extern  GLboolean ( APIENTRY * qglIsSampler )(GLuint sampler);
extern  void ( APIENTRY * qglBindSampler )(GLuint unit, GLuint sampler);
extern  void ( APIENTRY * qglSamplerParameteri )(GLuint sampler, GLenum pname, GLint param);
extern  void ( APIENTRY * qglSamplerParameteriv )(GLuint sampler, GLenum pname, const GLint* param);
extern  void ( APIENTRY * qglSamplerParameterf )(GLuint sampler, GLenum pname, GLfloat param);
extern  void ( APIENTRY * qglSamplerParameterfv )(GLuint sampler, GLenum pname, const GLfloat* param);
extern  void ( APIENTRY * qglGetSamplerParameteriv )(GLuint sampler, GLenum pname, GLint* params);
extern  void ( APIENTRY * qglGetSamplerParameterfv )(GLuint sampler, GLenum pname, GLfloat* params);
extern  void ( APIENTRY * qglVertexAttribDivisor )(GLuint index, GLuint divisor);
extern  void ( APIENTRY * qglBindTransformFeedback )(GLenum target, GLuint id);
extern  void ( APIENTRY * qglDeleteTransformFeedbacks )(GLsizei n, const GLuint* ids);
extern  void ( APIENTRY * qglGenTransformFeedbacks )(GLsizei n, GLuint* ids);
extern  GLboolean ( APIENTRY * qglIsTransformFeedback )(GLuint id);
extern  void ( APIENTRY * qglPauseTransformFeedback )(void);
extern  void ( APIENTRY * qglResumeTransformFeedback )(void);
extern  void ( APIENTRY * qglGetProgramBinary )(GLuint program, GLsizei bufSize, GLsizei* length, GLenum* binaryFormat, GLvoid* binary);
extern  void ( APIENTRY * qglProgramBinary )(GLuint program, GLenum binaryFormat, const GLvoid* binary, GLsizei length);
//extern  void ( APIENTRY * qglProgramParameteri )(GLuint program, GLenum pname, GLint value);
extern  void ( APIENTRY * qglInvalidateFramebuffer )(GLenum target, GLsizei numAttachments, const GLenum* attachments);
extern  void ( APIENTRY * qglInvalidateSubFramebuffer )(GLenum target, GLsizei numAttachments, const GLenum* attachments, GLint x, GLint y, GLsizei width, GLsizei height);
extern  void ( APIENTRY * qglTexStorage2D )(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height);
extern  void ( APIENTRY * qglTexStorage3D )(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth);
extern  void ( APIENTRY * qglGetInternalformativ )(GLenum target, GLenum internalformat, GLenum pname, GLsizei bufSize, GLint* params);

#endif

#endif

#ifdef ID_WIN32

extern  int   ( WINAPI * qwglChoosePixelFormat )(HDC, CONST PIXELFORMATDESCRIPTOR *);
extern  int   ( WINAPI * qwglDescribePixelFormat) (HDC, int, UINT, LPPIXELFORMATDESCRIPTOR);
extern  int   ( WINAPI * qwglGetPixelFormat)(HDC);
extern  BOOL  ( WINAPI * qwglSetPixelFormat)(HDC, int, CONST PIXELFORMATDESCRIPTOR *);
extern  BOOL  ( WINAPI * qwglSwapBuffers)(HDC);

extern BOOL  ( WINAPI * qwglCopyContext)(HGLRC, HGLRC, UINT);
extern HGLRC ( WINAPI * qwglCreateContext)(HDC);
extern HGLRC ( WINAPI * qwglCreateLayerContext)(HDC, int);
extern BOOL  ( WINAPI * qwglDeleteContext)(HGLRC);
extern HGLRC ( WINAPI * qwglGetCurrentContext)(VOID);
extern HDC   ( WINAPI * qwglGetCurrentDC)(VOID);
extern PROC  ( WINAPI * qwglGetProcAddress)(LPCSTR);
extern BOOL  ( WINAPI * qwglMakeCurrent)(HDC, HGLRC);
extern BOOL  ( WINAPI * qwglShareLists)(HGLRC, HGLRC);
extern BOOL  ( WINAPI * qwglUseFontBitmaps)(HDC, DWORD, DWORD, DWORD);

extern BOOL  ( WINAPI * qwglUseFontOutlines)(HDC, DWORD, DWORD, DWORD, FLOAT,
                                           FLOAT, int, LPGLYPHMETRICSFLOAT);

extern BOOL ( WINAPI * qwglDescribeLayerPlane)(HDC, int, int, UINT,
                                            LPLAYERPLANEDESCRIPTOR);
extern int  ( WINAPI * qwglSetLayerPaletteEntries)(HDC, int, int, int,
                                                CONST COLORREF *);
extern int  ( WINAPI * qwglGetLayerPaletteEntries)(HDC, int, int, int,
                                                COLORREF *);
extern BOOL ( WINAPI * qwglRealizeLayerPalette)(HDC, int, BOOL);
extern BOOL ( WINAPI * qwglSwapLayerBuffers)(HDC, UINT);

#else

extern EGLint ( APIENTRY * qeglGetError )(void);
extern EGLDisplay ( APIENTRY * qeglGetDisplay )(EGLNativeDisplayType);
extern EGLBoolean ( APIENTRY * qeglInitialize )(EGLDisplay, EGLint *, EGLint *);
extern EGLBoolean ( APIENTRY * qeglTerminate )(EGLDisplay);
extern const char * ( APIENTRY * qeglQueryString )(EGLDisplay, EGLint);
extern EGLBoolean ( APIENTRY * qeglGetConfigs )(EGLDisplay, EGLConfig *, EGLint, EGLint *);
extern EGLBoolean ( APIENTRY * qeglChooseConfig )(EGLDisplay, const EGLint *, EGLConfig *, EGLint, EGLint *);
extern EGLBoolean ( APIENTRY * qeglGetConfigAttrib )(EGLDisplay, EGLConfig, EGLint, EGLint *);
extern EGLSurface ( APIENTRY * qeglCreateWindowSurface )(EGLDisplay, EGLConfig, EGLNativeWindowType, const EGLint *);
extern EGLSurface ( APIENTRY * qeglCreatePbufferSurface )(EGLDisplay, EGLConfig, const EGLint *);
extern EGLSurface ( APIENTRY * qeglCreatePixmapSurface )(EGLDisplay, EGLConfig, EGLNativePixmapType, const EGLint *);
extern EGLBoolean ( APIENTRY * qeglDestroySurface )(EGLDisplay, EGLSurface);
extern EGLBoolean ( APIENTRY * qeglQuerySurface )(EGLDisplay, EGLSurface, EGLint, EGLint *);
extern EGLBoolean ( APIENTRY * qeglBindAPI )(EGLenum);
extern EGLenum ( APIENTRY * qeglQueryAPI )(void);
extern EGLBoolean ( APIENTRY * qeglWaitClient )(void);
extern EGLBoolean ( APIENTRY * qeglReleaseThread )(void);
extern EGLSurface ( APIENTRY * qeglCreatePbufferFromClientBuffer )(EGLDisplay, EGLenum, EGLClientBuffer, EGLConfig, const EGLint *);
extern EGLBoolean ( APIENTRY * qeglSurfaceAttrib )(EGLDisplay, EGLSurface, EGLint, EGLint);
extern EGLBoolean ( APIENTRY * qeglBindTexImage )(EGLDisplay, EGLSurface, EGLint);
extern EGLBoolean ( APIENTRY * qeglReleaseTexImage )(EGLDisplay, EGLSurface, EGLint);
extern EGLBoolean ( APIENTRY * qeglSwapInterval )(EGLDisplay, EGLint);
extern EGLContext ( APIENTRY * qeglCreateContext )(EGLDisplay, EGLConfig, EGLContext, const EGLint *);
extern EGLBoolean ( APIENTRY * qeglDestroyContext )(EGLDisplay, EGLContext);
extern EGLBoolean ( APIENTRY * qeglMakeCurrent )(EGLDisplay, EGLSurface, EGLSurface, EGLContext);
extern EGLContext ( APIENTRY * qeglGetCurrentContext )(void);
extern EGLSurface ( APIENTRY * qeglGetCurrentSurface )(EGLint);
extern EGLDisplay ( APIENTRY * qeglGetCurrentDisplay )(void);
extern EGLBoolean ( APIENTRY * qeglQueryContext )(EGLDisplay, EGLContext, EGLint, EGLint *);
extern EGLBoolean ( APIENTRY * qeglWaitGL )(void);
extern EGLBoolean ( APIENTRY * qeglWaitNative )(EGLint);
extern EGLBoolean ( APIENTRY * qeglSwapBuffers )(EGLDisplay, EGLSurface);
extern EGLBoolean ( APIENTRY * qeglCopyBuffers )(EGLDisplay, EGLSurface, EGLNativePixmapType);
extern GLExtension_t ( APIENTRY * qeglGetProcAddress )(const char *);

#endif

#endif	// hardlink vs dlopen

#endif
