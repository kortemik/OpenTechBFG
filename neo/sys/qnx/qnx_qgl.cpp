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

#include <float.h>
#include <dlfcn.h>
#include "qnx_local.h"
#include "../../renderer/tr_local.h"

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

//TODO
void ( APIENTRY * qglAccum )(GLenum op, GLfloat value);
void ( APIENTRY * qglAlphaFunc )(GLenum func, GLclampf ref);
GLboolean ( APIENTRY * qglAreTexturesResident )(GLsizei n, const GLuint *textures, GLboolean *residences);
void ( APIENTRY * qglArrayElement )(GLint i);
void ( APIENTRY * qglBegin )(GLenum mode);
void ( APIENTRY * qglBindTexture )(GLenum target, GLuint texture);
void ( APIENTRY * qglBitmap )(GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove, const GLubyte *bitmap);
void ( APIENTRY * qglBlendFunc )(GLenum sfactor, GLenum dfactor);
void ( APIENTRY * qglCallList )(GLuint list);
void ( APIENTRY * qglCallLists )(GLsizei n, GLenum type, const GLvoid *lists);
void ( APIENTRY * qglClear )(GLbitfield mask);
void ( APIENTRY * qglClearAccum )(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
void ( APIENTRY * qglClearColor )(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
void ( APIENTRY * qglClearDepth )(GLclampd depth);
void ( APIENTRY * qglClearIndex )(GLfloat c);
void ( APIENTRY * qglClearStencil )(GLint s);
void ( APIENTRY * qglClipPlane )(GLenum plane, const GLdouble *equation);
void ( APIENTRY * qglColor3b )(GLbyte red, GLbyte green, GLbyte blue);
void ( APIENTRY * qglColor3bv )(const GLbyte *v);
void ( APIENTRY * qglColor3d )(GLdouble red, GLdouble green, GLdouble blue);
void ( APIENTRY * qglColor3dv )(const GLdouble *v);
void ( APIENTRY * qglColor3f )(GLfloat red, GLfloat green, GLfloat blue);
void ( APIENTRY * qglColor3fv )(const GLfloat *v);
void ( APIENTRY * qglColor3i )(GLint red, GLint green, GLint blue);
void ( APIENTRY * qglColor3iv )(const GLint *v);
void ( APIENTRY * qglColor3s )(GLshort red, GLshort green, GLshort blue);
void ( APIENTRY * qglColor3sv )(const GLshort *v);
void ( APIENTRY * qglColor3ub )(GLubyte red, GLubyte green, GLubyte blue);
void ( APIENTRY * qglColor3ubv )(const GLubyte *v);
void ( APIENTRY * qglColor3ui )(GLuint red, GLuint green, GLuint blue);
void ( APIENTRY * qglColor3uiv )(const GLuint *v);
void ( APIENTRY * qglColor3us )(GLushort red, GLushort green, GLushort blue);
void ( APIENTRY * qglColor3usv )(const GLushort *v);
void ( APIENTRY * qglColor4b )(GLbyte red, GLbyte green, GLbyte blue, GLbyte alpha);
void ( APIENTRY * qglColor4bv )(const GLbyte *v);
void ( APIENTRY * qglColor4d )(GLdouble red, GLdouble green, GLdouble blue, GLdouble alpha);
void ( APIENTRY * qglColor4dv )(const GLdouble *v);
void ( APIENTRY * qglColor4f )(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
void ( APIENTRY * qglColor4fv )(const GLfloat *v);
void ( APIENTRY * qglColor4i )(GLint red, GLint green, GLint blue, GLint alpha);
void ( APIENTRY * qglColor4iv )(const GLint *v);
void ( APIENTRY * qglColor4s )(GLshort red, GLshort green, GLshort blue, GLshort alpha);
void ( APIENTRY * qglColor4sv )(const GLshort *v);
void ( APIENTRY * qglColor4ub )(GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha);
void ( APIENTRY * qglColor4ubv )(const GLubyte *v);
void ( APIENTRY * qglColor4ui )(GLuint red, GLuint green, GLuint blue, GLuint alpha);
void ( APIENTRY * qglColor4uiv )(const GLuint *v);
void ( APIENTRY * qglColor4us )(GLushort red, GLushort green, GLushort blue, GLushort alpha);
void ( APIENTRY * qglColor4usv )(const GLushort *v);
void ( APIENTRY * qglColorMask )(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
void ( APIENTRY * qglColorMaterial )(GLenum face, GLenum mode);
void ( APIENTRY * qglColorPointer )(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
void ( APIENTRY * qglCopyPixels )(GLint x, GLint y, GLsizei width, GLsizei height, GLenum type);
void ( APIENTRY * qglCopyTexImage1D )(GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLint border);
void ( APIENTRY * qglCopyTexImage2D )(GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
void ( APIENTRY * qglCopyTexSubImage1D )(GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);
void ( APIENTRY * qglCopyTexSubImage2D )(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
void ( APIENTRY * qglCullFace )(GLenum mode);
void ( APIENTRY * qglDeleteLists )(GLuint list, GLsizei range);
void ( APIENTRY * qglDeleteTextures )(GLsizei n, const GLuint *textures);
void ( APIENTRY * qglDepthFunc )(GLenum func);
void ( APIENTRY * qglDepthMask )(GLboolean flag);
void ( APIENTRY * qglDepthRange )(GLclampd zNear, GLclampd zFar);
void ( APIENTRY * qglDisable )(GLenum cap);
void ( APIENTRY * qglDisableClientState )(GLenum array);
void ( APIENTRY * qglDrawArrays )(GLenum mode, GLint first, GLsizei count);
void ( APIENTRY * qglDrawBuffer )(GLenum mode);
void ( APIENTRY * qglDrawElements )(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices);
void ( APIENTRY * qglDrawPixels )(GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
void ( APIENTRY * qglEdgeFlag )(GLboolean flag);
void ( APIENTRY * qglEdgeFlagPointer )(GLsizei stride, const GLvoid *pointer);
void ( APIENTRY * qglEdgeFlagv )(const GLboolean *flag);
void ( APIENTRY * qglEnable )(GLenum cap);
void ( APIENTRY * qglEnableClientState )(GLenum array);
void ( APIENTRY * qglEnd )(void);
void ( APIENTRY * qglEndList )(void);
void ( APIENTRY * qglEvalCoord1d )(GLdouble u);
void ( APIENTRY * qglEvalCoord1dv )(const GLdouble *u);
void ( APIENTRY * qglEvalCoord1f )(GLfloat u);
void ( APIENTRY * qglEvalCoord1fv )(const GLfloat *u);
void ( APIENTRY * qglEvalCoord2d )(GLdouble u, GLdouble v);
void ( APIENTRY * qglEvalCoord2dv )(const GLdouble *u);
void ( APIENTRY * qglEvalCoord2f )(GLfloat u, GLfloat v);
void ( APIENTRY * qglEvalCoord2fv )(const GLfloat *u);
void ( APIENTRY * qglEvalMesh1 )(GLenum mode, GLint i1, GLint i2);
void ( APIENTRY * qglEvalMesh2 )(GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2);
void ( APIENTRY * qglEvalPoint1 )(GLint i);
void ( APIENTRY * qglEvalPoint2 )(GLint i, GLint j);
void ( APIENTRY * qglFeedbackBuffer )(GLsizei size, GLenum type, GLfloat *buffer);
void ( APIENTRY * qglFinish )(void);
void ( APIENTRY * qglFlush )(void);
void ( APIENTRY * qglFogf )(GLenum pname, GLfloat param);
void ( APIENTRY * qglFogfv )(GLenum pname, const GLfloat *params);
void ( APIENTRY * qglFogi )(GLenum pname, GLint param);
void ( APIENTRY * qglFogiv )(GLenum pname, const GLint *params);
void ( APIENTRY * qglFrontFace )(GLenum mode);
void ( APIENTRY * qglFrustum )(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
GLuint ( APIENTRY * qglGenLists )(GLsizei range);
void ( APIENTRY * qglGenTextures )(GLsizei n, GLuint *textures);
void ( APIENTRY * qglGetBooleanv )(GLenum pname, GLboolean *params);
void ( APIENTRY * qglGetClipPlane )(GLenum plane, GLdouble *equation);
void ( APIENTRY * qglGetDoublev )(GLenum pname, GLdouble *params);
GLenum ( APIENTRY * qglGetError )(void);
void ( APIENTRY * qglGetFloatv )(GLenum pname, GLfloat *params);
void ( APIENTRY * qglGetIntegerv )(GLenum pname, GLint *params);
void ( APIENTRY * qglGetLightfv )(GLenum light, GLenum pname, GLfloat *params);
void ( APIENTRY * qglGetLightiv )(GLenum light, GLenum pname, GLint *params);
void ( APIENTRY * qglGetMapdv )(GLenum target, GLenum query, GLdouble *v);
void ( APIENTRY * qglGetMapfv )(GLenum target, GLenum query, GLfloat *v);
void ( APIENTRY * qglGetMapiv )(GLenum target, GLenum query, GLint *v);
void ( APIENTRY * qglGetMaterialfv )(GLenum face, GLenum pname, GLfloat *params);
void ( APIENTRY * qglGetMaterialiv )(GLenum face, GLenum pname, GLint *params);
void ( APIENTRY * qglGetPixelMapfv )(GLenum map, GLfloat *values);
void ( APIENTRY * qglGetPixelMapuiv )(GLenum map, GLuint *values);
void ( APIENTRY * qglGetPixelMapusv )(GLenum map, GLushort *values);
void ( APIENTRY * qglGetPointerv )(GLenum pname, GLvoid* *params);
void ( APIENTRY * qglGetPolygonStipple )(GLubyte *mask);
const GLubyte * ( APIENTRY * qglGetString )(GLenum name);
void ( APIENTRY * qglGetTexEnvfv )(GLenum target, GLenum pname, GLfloat *params);
void ( APIENTRY * qglGetTexEnviv )(GLenum target, GLenum pname, GLint *params);
void ( APIENTRY * qglGetTexGendv )(GLenum coord, GLenum pname, GLdouble *params);
void ( APIENTRY * qglGetTexGenfv )(GLenum coord, GLenum pname, GLfloat *params);
void ( APIENTRY * qglGetTexGeniv )(GLenum coord, GLenum pname, GLint *params);
void ( APIENTRY * qglGetTexImage )(GLenum target, GLint level, GLenum format, GLenum type, GLvoid *pixels);
void ( APIENTRY * qglGetTexLevelParameterfv )(GLenum target, GLint level, GLenum pname, GLfloat *params);
void ( APIENTRY * qglGetTexLevelParameteriv )(GLenum target, GLint level, GLenum pname, GLint *params);
void ( APIENTRY * qglGetTexParameterfv )(GLenum target, GLenum pname, GLfloat *params);
void ( APIENTRY * qglGetTexParameteriv )(GLenum target, GLenum pname, GLint *params);
void ( APIENTRY * qglHint )(GLenum target, GLenum mode);
void ( APIENTRY * qglIndexMask )(GLuint mask);
void ( APIENTRY * qglIndexPointer )(GLenum type, GLsizei stride, const GLvoid *pointer);
void ( APIENTRY * qglIndexd )(GLdouble c);
void ( APIENTRY * qglIndexdv )(const GLdouble *c);
void ( APIENTRY * qglIndexf )(GLfloat c);
void ( APIENTRY * qglIndexfv )(const GLfloat *c);
void ( APIENTRY * qglIndexi )(GLint c);
void ( APIENTRY * qglIndexiv )(const GLint *c);
void ( APIENTRY * qglIndexs )(GLshort c);
void ( APIENTRY * qglIndexsv )(const GLshort *c);
void ( APIENTRY * qglIndexub )(GLubyte c);
void ( APIENTRY * qglIndexubv )(const GLubyte *c);
void ( APIENTRY * qglInitNames )(void);
void ( APIENTRY * qglInterleavedArrays )(GLenum format, GLsizei stride, const GLvoid *pointer);
GLboolean ( APIENTRY * qglIsEnabled )(GLenum cap);
GLboolean ( APIENTRY * qglIsList )(GLuint list);
GLboolean ( APIENTRY * qglIsTexture )(GLuint texture);
void ( APIENTRY * qglLightModelf )(GLenum pname, GLfloat param);
void ( APIENTRY * qglLightModelfv )(GLenum pname, const GLfloat *params);
void ( APIENTRY * qglLightModeli )(GLenum pname, GLint param);
void ( APIENTRY * qglLightModeliv )(GLenum pname, const GLint *params);
void ( APIENTRY * qglLightf )(GLenum light, GLenum pname, GLfloat param);
void ( APIENTRY * qglLightfv )(GLenum light, GLenum pname, const GLfloat *params);
void ( APIENTRY * qglLighti )(GLenum light, GLenum pname, GLint param);
void ( APIENTRY * qglLightiv )(GLenum light, GLenum pname, const GLint *params);
void ( APIENTRY * qglLineStipple )(GLint factor, GLushort pattern);
void ( APIENTRY * qglLineWidth )(GLfloat width);
void ( APIENTRY * qglListBase )(GLuint base);
void ( APIENTRY * qglLoadIdentity )(void);
void ( APIENTRY * qglLoadMatrixd )(const GLdouble *m);
void ( APIENTRY * qglLoadMatrixf )(const GLfloat *m);
void ( APIENTRY * qglLoadName )(GLuint name);
void ( APIENTRY * qglLogicOp )(GLenum opcode);
void ( APIENTRY * qglMap1d )(GLenum target, GLdouble u1, GLdouble u2, GLint stride, GLint order, const GLdouble *points);
void ( APIENTRY * qglMap1f )(GLenum target, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat *points);
void ( APIENTRY * qglMap2d )(GLenum target, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, const GLdouble *points);
void ( APIENTRY * qglMap2f )(GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, const GLfloat *points);
void ( APIENTRY * qglMapGrid1d )(GLint un, GLdouble u1, GLdouble u2);
void ( APIENTRY * qglMapGrid1f )(GLint un, GLfloat u1, GLfloat u2);
void ( APIENTRY * qglMapGrid2d )(GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2);
void ( APIENTRY * qglMapGrid2f )(GLint un, GLfloat u1, GLfloat u2, GLint vn, GLfloat v1, GLfloat v2);
void ( APIENTRY * qglMaterialf )(GLenum face, GLenum pname, GLfloat param);
void ( APIENTRY * qglMaterialfv )(GLenum face, GLenum pname, const GLfloat *params);
void ( APIENTRY * qglMateriali )(GLenum face, GLenum pname, GLint param);
void ( APIENTRY * qglMaterialiv )(GLenum face, GLenum pname, const GLint *params);
void ( APIENTRY * qglMatrixMode )(GLenum mode);
void ( APIENTRY * qglMultMatrixd )(const GLdouble *m);
void ( APIENTRY * qglMultMatrixf )(const GLfloat *m);
void ( APIENTRY * qglNewList )(GLuint list, GLenum mode);
void ( APIENTRY * qglNormal3b )(GLbyte nx, GLbyte ny, GLbyte nz);
void ( APIENTRY * qglNormal3bv )(const GLbyte *v);
void ( APIENTRY * qglNormal3d )(GLdouble nx, GLdouble ny, GLdouble nz);
void ( APIENTRY * qglNormal3dv )(const GLdouble *v);
void ( APIENTRY * qglNormal3f )(GLfloat nx, GLfloat ny, GLfloat nz);
void ( APIENTRY * qglNormal3fv )(const GLfloat *v);
void ( APIENTRY * qglNormal3i )(GLint nx, GLint ny, GLint nz);
void ( APIENTRY * qglNormal3iv )(const GLint *v);
void ( APIENTRY * qglNormal3s )(GLshort nx, GLshort ny, GLshort nz);
void ( APIENTRY * qglNormal3sv )(const GLshort *v);
void ( APIENTRY * qglNormalPointer )(GLenum type, GLsizei stride, const GLvoid *pointer);
void ( APIENTRY * qglOrtho )(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
void ( APIENTRY * qglPassThrough )(GLfloat token);
void ( APIENTRY * qglPixelMapfv )(GLenum map, GLsizei mapsize, const GLfloat *values);
void ( APIENTRY * qglPixelMapuiv )(GLenum map, GLsizei mapsize, const GLuint *values);
void ( APIENTRY * qglPixelMapusv )(GLenum map, GLsizei mapsize, const GLushort *values);
void ( APIENTRY * qglPixelStoref )(GLenum pname, GLfloat param);
void ( APIENTRY * qglPixelStorei )(GLenum pname, GLint param);
void ( APIENTRY * qglPixelTransferf )(GLenum pname, GLfloat param);
void ( APIENTRY * qglPixelTransferi )(GLenum pname, GLint param);
void ( APIENTRY * qglPixelZoom )(GLfloat xfactor, GLfloat yfactor);
void ( APIENTRY * qglPointSize )(GLfloat size);
void ( APIENTRY * qglPolygonMode )(GLenum face, GLenum mode);
void ( APIENTRY * qglPolygonOffset )(GLfloat factor, GLfloat units);
void ( APIENTRY * qglPolygonStipple )(const GLubyte *mask);
void ( APIENTRY * qglPopAttrib )(void);
void ( APIENTRY * qglPopClientAttrib )(void);
void ( APIENTRY * qglPopMatrix )(void);
void ( APIENTRY * qglPopName )(void);
void ( APIENTRY * qglPrioritizeTextures )(GLsizei n, const GLuint *textures, const GLclampf *priorities);
void ( APIENTRY * qglPushAttrib )(GLbitfield mask);
void ( APIENTRY * qglPushClientAttrib )(GLbitfield mask);
void ( APIENTRY * qglPushMatrix )(void);
void ( APIENTRY * qglPushName )(GLuint name);
void ( APIENTRY * qglRasterPos2d )(GLdouble x, GLdouble y);
void ( APIENTRY * qglRasterPos2dv )(const GLdouble *v);
void ( APIENTRY * qglRasterPos2f )(GLfloat x, GLfloat y);
void ( APIENTRY * qglRasterPos2fv )(const GLfloat *v);
void ( APIENTRY * qglRasterPos2i )(GLint x, GLint y);
void ( APIENTRY * qglRasterPos2iv )(const GLint *v);
void ( APIENTRY * qglRasterPos2s )(GLshort x, GLshort y);
void ( APIENTRY * qglRasterPos2sv )(const GLshort *v);
void ( APIENTRY * qglRasterPos3d )(GLdouble x, GLdouble y, GLdouble z);
void ( APIENTRY * qglRasterPos3dv )(const GLdouble *v);
void ( APIENTRY * qglRasterPos3f )(GLfloat x, GLfloat y, GLfloat z);
void ( APIENTRY * qglRasterPos3fv )(const GLfloat *v);
void ( APIENTRY * qglRasterPos3i )(GLint x, GLint y, GLint z);
void ( APIENTRY * qglRasterPos3iv )(const GLint *v);
void ( APIENTRY * qglRasterPos3s )(GLshort x, GLshort y, GLshort z);
void ( APIENTRY * qglRasterPos3sv )(const GLshort *v);
void ( APIENTRY * qglRasterPos4d )(GLdouble x, GLdouble y, GLdouble z, GLdouble w);
void ( APIENTRY * qglRasterPos4dv )(const GLdouble *v);
void ( APIENTRY * qglRasterPos4f )(GLfloat x, GLfloat y, GLfloat z, GLfloat w);
void ( APIENTRY * qglRasterPos4fv )(const GLfloat *v);
void ( APIENTRY * qglRasterPos4i )(GLint x, GLint y, GLint z, GLint w);
void ( APIENTRY * qglRasterPos4iv )(const GLint *v);
void ( APIENTRY * qglRasterPos4s )(GLshort x, GLshort y, GLshort z, GLshort w);
void ( APIENTRY * qglRasterPos4sv )(const GLshort *v);
void ( APIENTRY * qglReadBuffer )(GLenum mode);
void ( APIENTRY * qglReadPixels )(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels);
void ( APIENTRY * qglRectd )(GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2);
void ( APIENTRY * qglRectdv )(const GLdouble *v1, const GLdouble *v2);
void ( APIENTRY * qglRectf )(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2);
void ( APIENTRY * qglRectfv )(const GLfloat *v1, const GLfloat *v2);
void ( APIENTRY * qglRecti )(GLint x1, GLint y1, GLint x2, GLint y2);
void ( APIENTRY * qglRectiv )(const GLint *v1, const GLint *v2);
void ( APIENTRY * qglRects )(GLshort x1, GLshort y1, GLshort x2, GLshort y2);
void ( APIENTRY * qglRectsv )(const GLshort *v1, const GLshort *v2);
GLint ( APIENTRY * qglRenderMode )(GLenum mode);
void ( APIENTRY * qglRotated )(GLdouble angle, GLdouble x, GLdouble y, GLdouble z);
void ( APIENTRY * qglRotatef )(GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
void ( APIENTRY * qglScaled )(GLdouble x, GLdouble y, GLdouble z);
void ( APIENTRY * qglScalef )(GLfloat x, GLfloat y, GLfloat z);
void ( APIENTRY * qglScissor )(GLint x, GLint y, GLsizei width, GLsizei height);
void ( APIENTRY * qglSelectBuffer )(GLsizei size, GLuint *buffer);
void ( APIENTRY * qglShadeModel )(GLenum mode);
void ( APIENTRY * qglStencilFunc )(GLenum func, GLint ref, GLuint mask);
void ( APIENTRY * qglStencilMask )(GLuint mask);
void ( APIENTRY * qglStencilOp )(GLenum fail, GLenum zfail, GLenum zpass);
void ( APIENTRY * qglTexCoord1d )(GLdouble s);
void ( APIENTRY * qglTexCoord1dv )(const GLdouble *v);
void ( APIENTRY * qglTexCoord1f )(GLfloat s);
void ( APIENTRY * qglTexCoord1fv )(const GLfloat *v);
void ( APIENTRY * qglTexCoord1i )(GLint s);
void ( APIENTRY * qglTexCoord1iv )(const GLint *v);
void ( APIENTRY * qglTexCoord1s )(GLshort s);
void ( APIENTRY * qglTexCoord1sv )(const GLshort *v);
void ( APIENTRY * qglTexCoord2d )(GLdouble s, GLdouble t);
void ( APIENTRY * qglTexCoord2dv )(const GLdouble *v);
void ( APIENTRY * qglTexCoord2f )(GLfloat s, GLfloat t);
void ( APIENTRY * qglTexCoord2fv )(const GLfloat *v);
void ( APIENTRY * qglTexCoord2i )(GLint s, GLint t);
void ( APIENTRY * qglTexCoord2iv )(const GLint *v);
void ( APIENTRY * qglTexCoord2s )(GLshort s, GLshort t);
void ( APIENTRY * qglTexCoord2sv )(const GLshort *v);
void ( APIENTRY * qglTexCoord3d )(GLdouble s, GLdouble t, GLdouble r);
void ( APIENTRY * qglTexCoord3dv )(const GLdouble *v);
void ( APIENTRY * qglTexCoord3f )(GLfloat s, GLfloat t, GLfloat r);
void ( APIENTRY * qglTexCoord3fv )(const GLfloat *v);
void ( APIENTRY * qglTexCoord3i )(GLint s, GLint t, GLint r);
void ( APIENTRY * qglTexCoord3iv )(const GLint *v);
void ( APIENTRY * qglTexCoord3s )(GLshort s, GLshort t, GLshort r);
void ( APIENTRY * qglTexCoord3sv )(const GLshort *v);
void ( APIENTRY * qglTexCoord4d )(GLdouble s, GLdouble t, GLdouble r, GLdouble q);
void ( APIENTRY * qglTexCoord4dv )(const GLdouble *v);
void ( APIENTRY * qglTexCoord4f )(GLfloat s, GLfloat t, GLfloat r, GLfloat q);
void ( APIENTRY * qglTexCoord4fv )(const GLfloat *v);
void ( APIENTRY * qglTexCoord4i )(GLint s, GLint t, GLint r, GLint q);
void ( APIENTRY * qglTexCoord4iv )(const GLint *v);
void ( APIENTRY * qglTexCoord4s )(GLshort s, GLshort t, GLshort r, GLshort q);
void ( APIENTRY * qglTexCoord4sv )(const GLshort *v);
void ( APIENTRY * qglTexCoordPointer )(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
void ( APIENTRY * qglTexEnvf )(GLenum target, GLenum pname, GLfloat param);
void ( APIENTRY * qglTexEnvfv )(GLenum target, GLenum pname, const GLfloat *params);
void ( APIENTRY * qglTexEnvi )(GLenum target, GLenum pname, GLint param);
void ( APIENTRY * qglTexEnviv )(GLenum target, GLenum pname, const GLint *params);
void ( APIENTRY * qglTexGend )(GLenum coord, GLenum pname, GLdouble param);
void ( APIENTRY * qglTexGendv )(GLenum coord, GLenum pname, const GLdouble *params);
void ( APIENTRY * qglTexGenf )(GLenum coord, GLenum pname, GLfloat param);
void ( APIENTRY * qglTexGenfv )(GLenum coord, GLenum pname, const GLfloat *params);
void ( APIENTRY * qglTexGeni )(GLenum coord, GLenum pname, GLint param);
void ( APIENTRY * qglTexGeniv )(GLenum coord, GLenum pname, const GLint *params);
void ( APIENTRY * qglTexImage1D )(GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
void ( APIENTRY * qglTexImage2D )(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
void ( APIENTRY * qglTexParameterf )(GLenum target, GLenum pname, GLfloat param);
void ( APIENTRY * qglTexParameterfv )(GLenum target, GLenum pname, const GLfloat *params);
void ( APIENTRY * qglTexParameteri )(GLenum target, GLenum pname, GLint param);
void ( APIENTRY * qglTexParameteriv )(GLenum target, GLenum pname, const GLint *params);
void ( APIENTRY * qglTexSubImage1D )(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid *pixels);
void ( APIENTRY * qglTexSubImage2D )(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
void ( APIENTRY * qglTranslated )(GLdouble x, GLdouble y, GLdouble z);
void ( APIENTRY * qglTranslatef )(GLfloat x, GLfloat y, GLfloat z);
void ( APIENTRY * qglVertex2d )(GLdouble x, GLdouble y);
void ( APIENTRY * qglVertex2dv )(const GLdouble *v);
void ( APIENTRY * qglVertex2f )(GLfloat x, GLfloat y);
void ( APIENTRY * qglVertex2fv )(const GLfloat *v);
void ( APIENTRY * qglVertex2i )(GLint x, GLint y);
void ( APIENTRY * qglVertex2iv )(const GLint *v);
void ( APIENTRY * qglVertex2s )(GLshort x, GLshort y);
void ( APIENTRY * qglVertex2sv )(const GLshort *v);
void ( APIENTRY * qglVertex3d )(GLdouble x, GLdouble y, GLdouble z);
void ( APIENTRY * qglVertex3dv )(const GLdouble *v);
void ( APIENTRY * qglVertex3f )(GLfloat x, GLfloat y, GLfloat z);
void ( APIENTRY * qglVertex3fv )(const GLfloat *v);
void ( APIENTRY * qglVertex3i )(GLint x, GLint y, GLint z);
void ( APIENTRY * qglVertex3iv )(const GLint *v);
void ( APIENTRY * qglVertex3s )(GLshort x, GLshort y, GLshort z);
void ( APIENTRY * qglVertex3sv )(const GLshort *v);
void ( APIENTRY * qglVertex4d )(GLdouble x, GLdouble y, GLdouble z, GLdouble w);
void ( APIENTRY * qglVertex4dv )(const GLdouble *v);
void ( APIENTRY * qglVertex4f )(GLfloat x, GLfloat y, GLfloat z, GLfloat w);
void ( APIENTRY * qglVertex4fv )(const GLfloat *v);
void ( APIENTRY * qglVertex4i )(GLint x, GLint y, GLint z, GLint w);
void ( APIENTRY * qglVertex4iv )(const GLint *v);
void ( APIENTRY * qglVertex4s )(GLshort x, GLshort y, GLshort z, GLshort w);
void ( APIENTRY * qglVertex4sv )(const GLshort *v);
void ( APIENTRY * qglVertexPointer )(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
void ( APIENTRY * qglViewport )(GLint x, GLint y, GLsizei width, GLsizei height);

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

#ifndef ID_GL_HARDLINK

	//TODO: Only have supported functions
	qglAccum                     = NULL;
	qglAlphaFunc                 = NULL;
	qglAreTexturesResident       = NULL;
	qglArrayElement              = NULL;
	qglBegin                     = NULL;
	qglBindTexture               = NULL;
	qglBitmap                    = NULL;
	qglBlendFunc                 = NULL;
	qglCallList                  = NULL;
	qglCallLists                 = NULL;
	qglClear                     = NULL;
	qglClearAccum                = NULL;
	qglClearColor                = NULL;
	qglClearDepth                = NULL;
	qglClearIndex                = NULL;
	qglClearStencil              = NULL;
	qglClipPlane                 = NULL;
	qglColor3b                   = NULL;
	qglColor3bv                  = NULL;
	qglColor3d                   = NULL;
	qglColor3dv                  = NULL;
	qglColor3f                   = NULL;
	qglColor3fv                  = NULL;
	qglColor3i                   = NULL;
	qglColor3iv                  = NULL;
	qglColor3s                   = NULL;
	qglColor3sv                  = NULL;
	qglColor3ub                  = NULL;
	qglColor3ubv                 = NULL;
	qglColor3ui                  = NULL;
	qglColor3uiv                 = NULL;
	qglColor3us                  = NULL;
	qglColor3usv                 = NULL;
	qglColor4b                   = NULL;
	qglColor4bv                  = NULL;
	qglColor4d                   = NULL;
	qglColor4dv                  = NULL;
	qglColor4f                   = NULL;
	qglColor4fv                  = NULL;
	qglColor4i                   = NULL;
	qglColor4iv                  = NULL;
	qglColor4s                   = NULL;
	qglColor4sv                  = NULL;
	qglColor4ub                  = NULL;
	qglColor4ubv                 = NULL;
	qglColor4ui                  = NULL;
	qglColor4uiv                 = NULL;
	qglColor4us                  = NULL;
	qglColor4usv                 = NULL;
	qglColorMask                 = NULL;
	qglColorMaterial             = NULL;
	qglColorPointer              = NULL;
	qglCopyPixels                = NULL;
	qglCopyTexImage1D            = NULL;
	qglCopyTexImage2D            = NULL;
	qglCopyTexSubImage1D         = NULL;
	qglCopyTexSubImage2D         = NULL;
	qglCullFace                  = NULL;
	qglDeleteLists               = NULL;
	qglDeleteTextures            = NULL;
	qglDepthFunc                 = NULL;
	qglDepthMask                 = NULL;
	qglDepthRange                = NULL;
	qglDisable                   = NULL;
	qglDisableClientState        = NULL;
	qglDrawArrays                = NULL;
	qglDrawBuffer                = NULL;
	qglDrawElements              = NULL;
	qglDrawPixels                = NULL;
	qglEdgeFlag                  = NULL;
	qglEdgeFlagPointer           = NULL;
	qglEdgeFlagv                 = NULL;
	qglEnable                    = NULL;
	qglEnableClientState         = NULL;
	qglEnd                       = NULL;
	qglEndList                   = NULL;
	qglEvalCoord1d               = NULL;
	qglEvalCoord1dv              = NULL;
	qglEvalCoord1f               = NULL;
	qglEvalCoord1fv              = NULL;
	qglEvalCoord2d               = NULL;
	qglEvalCoord2dv              = NULL;
	qglEvalCoord2f               = NULL;
	qglEvalCoord2fv              = NULL;
	qglEvalMesh1                 = NULL;
	qglEvalMesh2                 = NULL;
	qglEvalPoint1                = NULL;
	qglEvalPoint2                = NULL;
	qglFeedbackBuffer            = NULL;
	qglFinish                    = NULL;
	qglFlush                     = NULL;
	qglFogf                      = NULL;
	qglFogfv                     = NULL;
	qglFogi                      = NULL;
	qglFogiv                     = NULL;
	qglFrontFace                 = NULL;
	qglFrustum                   = NULL;
	qglGenLists                  = NULL;
	qglGenTextures               = NULL;
	qglGetBooleanv               = NULL;
	qglGetClipPlane              = NULL;
	qglGetDoublev                = NULL;
	qglGetError                  = NULL;
	qglGetFloatv                 = NULL;
	qglGetIntegerv               = NULL;
	qglGetLightfv                = NULL;
	qglGetLightiv                = NULL;
	qglGetMapdv                  = NULL;
	qglGetMapfv                  = NULL;
	qglGetMapiv                  = NULL;
	qglGetMaterialfv             = NULL;
	qglGetMaterialiv             = NULL;
	qglGetPixelMapfv             = NULL;
	qglGetPixelMapuiv            = NULL;
	qglGetPixelMapusv            = NULL;
	qglGetPointerv               = NULL;
	qglGetPolygonStipple         = NULL;
	qglGetString                 = NULL;
	qglGetTexEnvfv               = NULL;
	qglGetTexEnviv               = NULL;
	qglGetTexGendv               = NULL;
	qglGetTexGenfv               = NULL;
	qglGetTexGeniv               = NULL;
	qglGetTexImage               = NULL;
	qglGetTexLevelParameterfv    = NULL;
	qglGetTexLevelParameteriv    = NULL;
	qglGetTexParameterfv         = NULL;
	qglGetTexParameteriv         = NULL;
	qglHint                      = NULL;
	qglIndexMask                 = NULL;
	qglIndexPointer              = NULL;
	qglIndexd                    = NULL;
	qglIndexdv                   = NULL;
	qglIndexf                    = NULL;
	qglIndexfv                   = NULL;
	qglIndexi                    = NULL;
	qglIndexiv                   = NULL;
	qglIndexs                    = NULL;
	qglIndexsv                   = NULL;
	qglIndexub                   = NULL;
	qglIndexubv                  = NULL;
	qglInitNames                 = NULL;
	qglInterleavedArrays         = NULL;
	qglIsEnabled                 = NULL;
	qglIsList                    = NULL;
	qglIsTexture                 = NULL;
	qglLightModelf               = NULL;
	qglLightModelfv              = NULL;
	qglLightModeli               = NULL;
	qglLightModeliv              = NULL;
	qglLightf                    = NULL;
	qglLightfv                   = NULL;
	qglLighti                    = NULL;
	qglLightiv                   = NULL;
	qglLineStipple               = NULL;
	qglLineWidth                 = NULL;
	qglListBase                  = NULL;
	qglLoadIdentity              = NULL;
	qglLoadMatrixd               = NULL;
	qglLoadMatrixf               = NULL;
	qglLoadName                  = NULL;
	qglLogicOp                   = NULL;
	qglMap1d                     = NULL;
	qglMap1f                     = NULL;
	qglMap2d                     = NULL;
	qglMap2f                     = NULL;
	qglMapGrid1d                 = NULL;
	qglMapGrid1f                 = NULL;
	qglMapGrid2d                 = NULL;
	qglMapGrid2f                 = NULL;
	qglMaterialf                 = NULL;
	qglMaterialfv                = NULL;
	qglMateriali                 = NULL;
	qglMaterialiv                = NULL;
	qglMatrixMode                = NULL;
	qglMultMatrixd               = NULL;
	qglMultMatrixf               = NULL;
	qglNewList                   = NULL;
	qglNormal3b                  = NULL;
	qglNormal3bv                 = NULL;
	qglNormal3d                  = NULL;
	qglNormal3dv                 = NULL;
	qglNormal3f                  = NULL;
	qglNormal3fv                 = NULL;
	qglNormal3i                  = NULL;
	qglNormal3iv                 = NULL;
	qglNormal3s                  = NULL;
	qglNormal3sv                 = NULL;
	qglNormalPointer             = NULL;
	qglOrtho                     = NULL;
	qglPassThrough               = NULL;
	qglPixelMapfv                = NULL;
	qglPixelMapuiv               = NULL;
	qglPixelMapusv               = NULL;
	qglPixelStoref               = NULL;
	qglPixelStorei               = NULL;
	qglPixelTransferf            = NULL;
	qglPixelTransferi            = NULL;
	qglPixelZoom                 = NULL;
	qglPointSize                 = NULL;
	qglPolygonMode               = NULL;
	qglPolygonOffset             = NULL;
	qglPolygonStipple            = NULL;
	qglPopAttrib                 = NULL;
	qglPopClientAttrib           = NULL;
	qglPopMatrix                 = NULL;
	qglPopName                   = NULL;
	qglPrioritizeTextures        = NULL;
	qglPushAttrib                = NULL;
	qglPushClientAttrib          = NULL;
	qglPushMatrix                = NULL;
	qglPushName                  = NULL;
	qglRasterPos2d               = NULL;
	qglRasterPos2dv              = NULL;
	qglRasterPos2f               = NULL;
	qglRasterPos2fv              = NULL;
	qglRasterPos2i               = NULL;
	qglRasterPos2iv              = NULL;
	qglRasterPos2s               = NULL;
	qglRasterPos2sv              = NULL;
	qglRasterPos3d               = NULL;
	qglRasterPos3dv              = NULL;
	qglRasterPos3f               = NULL;
	qglRasterPos3fv              = NULL;
	qglRasterPos3i               = NULL;
	qglRasterPos3iv              = NULL;
	qglRasterPos3s               = NULL;
	qglRasterPos3sv              = NULL;
	qglRasterPos4d               = NULL;
	qglRasterPos4dv              = NULL;
	qglRasterPos4f               = NULL;
	qglRasterPos4fv              = NULL;
	qglRasterPos4i               = NULL;
	qglRasterPos4iv              = NULL;
	qglRasterPos4s               = NULL;
	qglRasterPos4sv              = NULL;
	qglReadBuffer                = NULL;
	qglReadPixels                = NULL;
	qglRectd                     = NULL;
	qglRectdv                    = NULL;
	qglRectf                     = NULL;
	qglRectfv                    = NULL;
	qglRecti                     = NULL;
	qglRectiv                    = NULL;
	qglRects                     = NULL;
	qglRectsv                    = NULL;
	qglRenderMode                = NULL;
	qglRotated                   = NULL;
	qglRotatef                   = NULL;
	qglScaled                    = NULL;
	qglScalef                    = NULL;
	qglScissor                   = NULL;
	qglSelectBuffer              = NULL;
	qglShadeModel                = NULL;
	qglStencilFunc               = NULL;
	qglStencilMask               = NULL;
	qglStencilOp                 = NULL;
	qglTexCoord1d                = NULL;
	qglTexCoord1dv               = NULL;
	qglTexCoord1f                = NULL;
	qglTexCoord1fv               = NULL;
	qglTexCoord1i                = NULL;
	qglTexCoord1iv               = NULL;
	qglTexCoord1s                = NULL;
	qglTexCoord1sv               = NULL;
	qglTexCoord2d                = NULL;
	qglTexCoord2dv               = NULL;
	qglTexCoord2f                = NULL;
	qglTexCoord2fv               = NULL;
	qglTexCoord2i                = NULL;
	qglTexCoord2iv               = NULL;
	qglTexCoord2s                = NULL;
	qglTexCoord2sv               = NULL;
	qglTexCoord3d                = NULL;
	qglTexCoord3dv               = NULL;
	qglTexCoord3f                = NULL;
	qglTexCoord3fv               = NULL;
	qglTexCoord3i                = NULL;
	qglTexCoord3iv               = NULL;
	qglTexCoord3s                = NULL;
	qglTexCoord3sv               = NULL;
	qglTexCoord4d                = NULL;
	qglTexCoord4dv               = NULL;
	qglTexCoord4f                = NULL;
	qglTexCoord4fv               = NULL;
	qglTexCoord4i                = NULL;
	qglTexCoord4iv               = NULL;
	qglTexCoord4s                = NULL;
	qglTexCoord4sv               = NULL;
	qglTexCoordPointer           = NULL;
	qglTexEnvf                   = NULL;
	qglTexEnvfv                  = NULL;
	qglTexEnvi                   = NULL;
	qglTexEnviv                  = NULL;
	qglTexGend                   = NULL;
	qglTexGendv                  = NULL;
	qglTexGenf                   = NULL;
	qglTexGenfv                  = NULL;
	qglTexGeni                   = NULL;
	qglTexGeniv                  = NULL;
	qglTexImage1D                = NULL;
	qglTexImage2D                = NULL;
	qglTexParameterf             = NULL;
	qglTexParameterfv            = NULL;
	qglTexParameteri             = NULL;
	qglTexParameteriv            = NULL;
	qglTexSubImage1D             = NULL;
	qglTexSubImage2D             = NULL;
	qglTranslated                = NULL;
	qglTranslatef                = NULL;
	qglVertex2d                  = NULL;
	qglVertex2dv                 = NULL;
	qglVertex2f                  = NULL;
	qglVertex2fv                 = NULL;
	qglVertex2i                  = NULL;
	qglVertex2iv                 = NULL;
	qglVertex2s                  = NULL;
	qglVertex2sv                 = NULL;
	qglVertex3d                  = NULL;
	qglVertex3dv                 = NULL;
	qglVertex3f                  = NULL;
	qglVertex3fv                 = NULL;
	qglVertex3i                  = NULL;
	qglVertex3iv                 = NULL;
	qglVertex3s                  = NULL;
	qglVertex3sv                 = NULL;
	qglVertex4d                  = NULL;
	qglVertex4dv                 = NULL;
	qglVertex4f                  = NULL;
	qglVertex4fv                 = NULL;
	qglVertex4i                  = NULL;
	qglVertex4iv                 = NULL;
	qglVertex4s                  = NULL;
	qglVertex4sv                 = NULL;
	qglVertexPointer             = NULL;
	qglViewport                  = NULL;

	qeglGetError						= NULL;
	qeglGetDisplay						= NULL;
	qeglInitialize						= NULL;
	qeglTerminate						= NULL;
	qeglQueryString						= NULL;
	qeglGetConfigs						= NULL;
	qeglChooseConfig					= NULL;
	qeglGetConfigAttrib					= NULL;
	qeglCreateWindowSurface				= NULL;
	qeglCreatePbufferSurface			= NULL;
	qeglCreatePixmapSurface				= NULL;
	qeglDestroySurface					= NULL;
	qeglQuerySurface					= NULL;
	qeglBindAPI							= NULL;
	qeglQueryAPI						= NULL;
	qeglWaitClient						= NULL;
	qeglReleaseThread					= NULL;
	qeglCreatePbufferFromClientBuffer	= NULL;
	qeglSurfaceAttrib					= NULL;
	qeglBindTexImage					= NULL;
	qeglReleaseTexImage					= NULL;
	qeglSwapInterval					= NULL;
	qeglCreateContext					= NULL;
	qeglDestroyContext					= NULL;
	qeglMakeCurrent						= NULL;
	qeglGetCurrentContext				= NULL;
	qeglGetCurrentSurface				= NULL;
	qeglGetCurrentDisplay				= NULL;
	qeglQueryContext					= NULL;
	qeglWaitGL							= NULL;
	qeglWaitNative						= NULL;
	qeglSwapBuffers						= NULL;
	qeglCopyBuffers						= NULL;
	qeglGetProcAddress					= NULL;

#endif
}

#ifdef _DEBUG
#define GLSYM( a ) q##a = dlsym( qnx.openGLLib, #a )
#define EGLSYM( a ) q##a = dlsym( qnx.eglLib, #a )
#else
#define GLSYM( a ) q##a = a
#define EGLSYM( a ) GLSYM(a)
#endif

/*
** QGL_Init
**
** This is responsible for binding our qgl function pointers to
** the appropriate GL stuff.
*/
bool QGL_Init( const char *dllname )
{
	assert( qnx.eglLib == NULL );
	assert( qnx.openGLLib == NULL );

	common->Printf( "...initializing QGL\n" );

	// EGL
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

	if ( ( qnx.openGLLib = dlopen( dllName, RTLD_LAZY ) ) == NULL )
	{
		common->Printf( "failed\n" );
		dlclose( qnx.eglLib );
		qnx.eglLib = NULL;
		return false;
	}
	common->Printf( "succeeded\n" );

#ifndef ID_GL_HARDLINK

	//TODO: Remove all unsupported functions (and organize tabs)
	qglAccum                     = dllAccum = glAccum;
	qglAlphaFunc                 = dllAlphaFunc = glAlphaFunc;
	qglAreTexturesResident       = dllAreTexturesResident = glAreTexturesResident;
	qglArrayElement              = dllArrayElement = glArrayElement;
	qglBegin                     = dllBegin = glBegin;
	qglBindTexture               = dllBindTexture = glBindTexture;
	qglBitmap                    = dllBitmap = glBitmap;
	qglBlendFunc                 = dllBlendFunc = glBlendFunc;
	qglCallList                  = dllCallList = glCallList;
	qglCallLists                 = dllCallLists = glCallLists;
	qglClear                     = dllClear = glClear;
	qglClearAccum                = dllClearAccum = glClearAccum;
	qglClearColor                = dllClearColor = glClearColor;
#ifdef glClearDepth
	qglClearDepth                = dllClearDepth = glClearDepth;
#else
	qglClearDepth                = dllClearDepth = glClearDepthf;
#endif
	qglClearIndex                = dllClearIndex = glClearIndex;
	qglClearStencil              = dllClearStencil = glClearStencil;
	qglClipPlane                 = dllClipPlane = glClipPlane;
	qglColor3b                   = dllColor3b = glColor3b;
	qglColor3bv                  = dllColor3bv = glColor3bv;
	qglColor3d                   = dllColor3d = glColor3d;
	qglColor3dv                  = dllColor3dv = glColor3dv;
	qglColor3f                   = dllColor3f = glColor3f;
	qglColor3fv                  = dllColor3fv = glColor3fv;
	qglColor3i                   = dllColor3i = glColor3i;
	qglColor3iv                  = dllColor3iv = glColor3iv;
	qglColor3s                   = dllColor3s = glColor3s;
	qglColor3sv                  = dllColor3sv = glColor3sv;
	qglColor3ub                  = dllColor3ub = glColor3ub;
	qglColor3ubv                 = dllColor3ubv = glColor3ubv;
	qglColor3ui                  = dllColor3ui = glColor3ui;
	qglColor3uiv                 = dllColor3uiv = glColor3uiv;
	qglColor3us                  = dllColor3us = glColor3us;
	qglColor3usv                 = dllColor3usv = glColor3usv;
	qglColor4b                   = dllColor4b = glColor4b;
	qglColor4bv                  = dllColor4bv = glColor4bv;
	qglColor4d                   = dllColor4d = glColor4d;
	qglColor4dv                  = dllColor4dv = glColor4dv;
	qglColor4f                   = dllColor4f = glColor4f;
	qglColor4fv                  = dllColor4fv = glColor4fv;
	qglColor4i                   = dllColor4i = glColor4i;
	qglColor4iv                  = dllColor4iv = glColor4iv;
	qglColor4s                   = dllColor4s = glColor4s;
	qglColor4sv                  = dllColor4sv = glColor4sv;
	qglColor4ub                  = dllColor4ub = glColor4ub;
	qglColor4ubv                 = dllColor4ubv = glColor4ubv;
	qglColor4ui                  = dllColor4ui = glColor4ui;
	qglColor4uiv                 = dllColor4uiv = glColor4uiv;
	qglColor4us                  = dllColor4us = glColor4us;
	qglColor4usv                 = dllColor4usv = glColor4usv;
	qglColorMask                 = dllColorMask = glColorMask;
	qglColorMaterial             = dllColorMaterial = glColorMaterial;
	qglColorPointer              = dllColorPointer = glColorPointer;
	qglCopyPixels                = dllCopyPixels = glCopyPixels;
	qglCopyTexImage1D            = dllCopyTexImage1D = glCopyTexImage1D;
	qglCopyTexImage2D            = dllCopyTexImage2D = glCopyTexImage2D;
	qglCopyTexSubImage1D         = dllCopyTexSubImage1D = glCopyTexSubImage1D;
	qglCopyTexSubImage2D         = dllCopyTexSubImage2D = glCopyTexSubImage2D;
	qglCullFace                  = dllCullFace = glCullFace;
	qglDeleteLists               = dllDeleteLists = glDeleteLists;
	qglDeleteTextures            = dllDeleteTextures = glDeleteTextures;
	qglDepthFunc                 = dllDepthFunc = glDepthFunc;
	qglDepthMask                 = dllDepthMask = glDepthMask;
	qglDepthRange                = dllDepthRange = glDepthRange;
	qglDisable                   = dllDisable = glDisable;
	qglDisableClientState        = dllDisableClientState = glDisableClientState;
	qglDrawArrays                = dllDrawArrays = glDrawArrays;
	qglDrawBuffer                = dllDrawBuffer = glDrawBuffer;
	qglDrawElements              = dllDrawElements = glDrawElements;
	qglDrawPixels                = dllDrawPixels = glDrawPixels;
	qglEdgeFlag                  = dllEdgeFlag = glEdgeFlag;
	qglEdgeFlagPointer           = dllEdgeFlagPointer = glEdgeFlagPointer;
	qglEdgeFlagv                 = dllEdgeFlagv = glEdgeFlagv;
	qglEnable                    = 	dllEnable                    = glEnable;
	qglEnableClientState         = 	dllEnableClientState         = glEnableClientState;
	qglEnd                       = 	dllEnd                       = glEnd;
	qglEndList                   = 	dllEndList                   = glEndList;
	qglEvalCoord1d				 = 	dllEvalCoord1d				 = glEvalCoord1d;
	qglEvalCoord1dv              = 	dllEvalCoord1dv              = glEvalCoord1dv;
	qglEvalCoord1f               = 	dllEvalCoord1f               = glEvalCoord1f;
	qglEvalCoord1fv              = 	dllEvalCoord1fv              = glEvalCoord1fv;
	qglEvalCoord2d               = 	dllEvalCoord2d               = glEvalCoord2d;
	qglEvalCoord2dv              = 	dllEvalCoord2dv              = glEvalCoord2dv;
	qglEvalCoord2f               = 	dllEvalCoord2f               = glEvalCoord2f;
	qglEvalCoord2fv              = 	dllEvalCoord2fv              = glEvalCoord2fv;
	qglEvalMesh1                 = 	dllEvalMesh1                 = glEvalMesh1;
	qglEvalMesh2                 = 	dllEvalMesh2                 = glEvalMesh2;
	qglEvalPoint1                = 	dllEvalPoint1                = glEvalPoint1;
	qglEvalPoint2                = 	dllEvalPoint2                = glEvalPoint2;
	qglFeedbackBuffer            = 	dllFeedbackBuffer            = glFeedbackBuffer;
	qglFinish                    = 	dllFinish                    = glFinish;
	qglFlush                     = 	dllFlush                     = glFlush;
	qglFogf                      = 	dllFogf                      = glFogf;
	qglFogfv                     = 	dllFogfv                     = glFogfv;
	qglFogi                      = 	dllFogi                      = glFogi;
	qglFogiv                     = 	dllFogiv                     = glFogiv;
	qglFrontFace                 = 	dllFrontFace                 = glFrontFace;
	qglFrustum                   = 	dllFrustum                   = glFrustum;
	qglGenLists                  = 	dllGenLists                  = ( GLuint (__stdcall * )(int) ) glGenLists;
	qglGenTextures               = 	dllGenTextures               = glGenTextures;
	qglGetBooleanv               = 	dllGetBooleanv               = glGetBooleanv;
	qglGetClipPlane              = 	dllGetClipPlane              = glGetClipPlane;
	qglGetDoublev                = 	dllGetDoublev                = glGetDoublev;
	qglGetError                  = 	dllGetError                  = ( GLenum (__stdcall * )(void) ) glGetError;
	qglGetFloatv                 = 	dllGetFloatv                 = glGetFloatv;
	qglGetIntegerv               = 	dllGetIntegerv               = glGetIntegerv;
	qglGetLightfv                = 	dllGetLightfv                = glGetLightfv;
	qglGetLightiv                = 	dllGetLightiv                = glGetLightiv;
	qglGetMapdv                  = 	dllGetMapdv                  = glGetMapdv;
	qglGetMapfv                  = 	dllGetMapfv                  = glGetMapfv;
	qglGetMapiv                  = 	dllGetMapiv                  = glGetMapiv;
	qglGetMaterialfv             = 	dllGetMaterialfv             = glGetMaterialfv;
	qglGetMaterialiv             = 	dllGetMaterialiv             = glGetMaterialiv;
	qglGetPixelMapfv             = 	dllGetPixelMapfv             = glGetPixelMapfv;
	qglGetPixelMapuiv            = 	dllGetPixelMapuiv            = glGetPixelMapuiv;
	qglGetPixelMapusv            = 	dllGetPixelMapusv            = glGetPixelMapusv;
	qglGetPointerv               = 	dllGetPointerv               = glGetPointerv;
	qglGetPolygonStipple         = 	dllGetPolygonStipple         = glGetPolygonStipple;
	qglGetString                 = 	dllGetString                 = glGetString;
	qglGetTexEnvfv               = 	dllGetTexEnvfv               = glGetTexEnvfv;
	qglGetTexEnviv               = 	dllGetTexEnviv               = glGetTexEnviv;
	qglGetTexGendv               = 	dllGetTexGendv               = glGetTexGendv;
	qglGetTexGenfv               = 	dllGetTexGenfv               = glGetTexGenfv;
	qglGetTexGeniv               = 	dllGetTexGeniv               = glGetTexGeniv;
	qglGetTexImage               = 	dllGetTexImage               = glGetTexImage;
	qglGetTexLevelParameterfv    = 	dllGetTexLevelParameterfv    = glGetTexLevelParameterfv;
	qglGetTexLevelParameteriv    = 	dllGetTexLevelParameteriv    = glGetTexLevelParameteriv;
	qglGetTexParameterfv         = 	dllGetTexParameterfv         = glGetTexParameterfv;
	qglGetTexParameteriv         = 	dllGetTexParameteriv         = glGetTexParameteriv;
	qglHint                      = 	dllHint                      = glHint;
	qglIndexMask                 = 	dllIndexMask                 = glIndexMask;
	qglIndexPointer              = 	dllIndexPointer              = glIndexPointer;
	qglIndexd                    = 	dllIndexd                    = glIndexd;
	qglIndexdv                   = 	dllIndexdv                   = glIndexdv;
	qglIndexf                    = 	dllIndexf                    = glIndexf;
	qglIndexfv                   = 	dllIndexfv                   = glIndexfv;
	qglIndexi                    = 	dllIndexi                    = glIndexi;
	qglIndexiv                   = 	dllIndexiv                   = glIndexiv;
	qglIndexs                    = 	dllIndexs                    = glIndexs;
	qglIndexsv                   = 	dllIndexsv                   = glIndexsv;
	qglIndexub                   = 	dllIndexub                   = glIndexub;
	qglIndexubv                  = 	dllIndexubv                  = glIndexubv;
	qglInitNames                 = 	dllInitNames                 = glInitNames;
	qglInterleavedArrays         = 	dllInterleavedArrays         = glInterleavedArrays;
	qglIsEnabled                 = 	dllIsEnabled                 = glIsEnabled;
	qglIsList                    = 	dllIsList                    = glIsList;
	qglIsTexture                 = 	dllIsTexture                 = glIsTexture;
	qglLightModelf               = 	dllLightModelf               = glLightModelf;
	qglLightModelfv              = 	dllLightModelfv              = glLightModelfv;
	qglLightModeli               = 	dllLightModeli               = glLightModeli;
	qglLightModeliv              = 	dllLightModeliv              = glLightModeliv;
	qglLightf                    = 	dllLightf                    = glLightf;
	qglLightfv                   = 	dllLightfv                   = glLightfv;
	qglLighti                    = 	dllLighti                    = glLighti;
	qglLightiv                   = 	dllLightiv                   = glLightiv;
	qglLineStipple               = 	dllLineStipple               = glLineStipple;
	qglLineWidth                 = 	dllLineWidth                 = glLineWidth;
	qglListBase                  = 	dllListBase                  = glListBase;
	qglLoadIdentity              = 	dllLoadIdentity              = glLoadIdentity;
	qglLoadMatrixd               = 	dllLoadMatrixd               = glLoadMatrixd;
	qglLoadMatrixf               = 	dllLoadMatrixf               = glLoadMatrixf;
	qglLoadName                  = 	dllLoadName                  = glLoadName;
	qglLogicOp                   = 	dllLogicOp                   = glLogicOp;
	qglMap1d                     = 	dllMap1d                     = glMap1d;
	qglMap1f                     = 	dllMap1f                     = glMap1f;
	qglMap2d                     = 	dllMap2d                     = glMap2d;
	qglMap2f                     = 	dllMap2f                     = glMap2f;
	qglMapGrid1d                 = 	dllMapGrid1d                 = glMapGrid1d;
	qglMapGrid1f                 = 	dllMapGrid1f                 = glMapGrid1f;
	qglMapGrid2d                 = 	dllMapGrid2d                 = glMapGrid2d;
	qglMapGrid2f                 = 	dllMapGrid2f                 = glMapGrid2f;
	qglMaterialf                 = 	dllMaterialf                 = glMaterialf;
	qglMaterialfv                = 	dllMaterialfv                = glMaterialfv;
	qglMateriali                 = 	dllMateriali                 = glMateriali;
	qglMaterialiv                = 	dllMaterialiv                = glMaterialiv;
	qglMatrixMode                = 	dllMatrixMode                = glMatrixMode;
	qglMultMatrixd               = 	dllMultMatrixd               = glMultMatrixd;
	qglMultMatrixf               = 	dllMultMatrixf               = glMultMatrixf;
	qglNewList                   = 	dllNewList                   = glNewList;
	qglNormal3b                  = 	dllNormal3b                  = glNormal3b;
	qglNormal3bv                 = 	dllNormal3bv                 = glNormal3bv;
	qglNormal3d                  = 	dllNormal3d                  = glNormal3d;
	qglNormal3dv                 = 	dllNormal3dv                 = glNormal3dv;
	qglNormal3f                  = 	dllNormal3f                  = glNormal3f;
	qglNormal3fv                 = 	dllNormal3fv                 = glNormal3fv;
	qglNormal3i                  = 	dllNormal3i                  = glNormal3i;
	qglNormal3iv                 = 	dllNormal3iv                 = glNormal3iv;
	qglNormal3s                  = 	dllNormal3s                  = glNormal3s;
	qglNormal3sv                 = 	dllNormal3sv                 = glNormal3sv;
	qglNormalPointer             = 	dllNormalPointer             = glNormalPointer;
	qglOrtho                     = 	dllOrtho                     = glOrtho;
	qglPassThrough               = 	dllPassThrough               = glPassThrough;
	qglPixelMapfv                = 	dllPixelMapfv                = glPixelMapfv;
	qglPixelMapuiv               = 	dllPixelMapuiv               = glPixelMapuiv;
	qglPixelMapusv               = 	dllPixelMapusv               = glPixelMapusv;
	qglPixelStoref               = 	dllPixelStoref               = glPixelStoref;
	qglPixelStorei               = 	dllPixelStorei               = glPixelStorei;
	qglPixelTransferf            = 	dllPixelTransferf            = glPixelTransferf;
	qglPixelTransferi            = 	dllPixelTransferi            = glPixelTransferi;
	qglPixelZoom                 = 	dllPixelZoom                 = glPixelZoom;
	qglPointSize                 = 	dllPointSize                 = glPointSize;
	qglPolygonMode               = 	dllPolygonMode               = glPolygonMode;
	qglPolygonOffset             = 	dllPolygonOffset             = glPolygonOffset;
	qglPolygonStipple            = 	dllPolygonStipple            = glPolygonStipple;
	qglPopAttrib                 = 	dllPopAttrib                 = glPopAttrib;
	qglPopClientAttrib           = 	dllPopClientAttrib           = glPopClientAttrib;
	qglPopMatrix                 = 	dllPopMatrix                 = glPopMatrix;
	qglPopName                   = 	dllPopName                   = glPopName;
	qglPrioritizeTextures        = 	dllPrioritizeTextures        = glPrioritizeTextures;
	qglPushAttrib                = 	dllPushAttrib                = glPushAttrib;
	qglPushClientAttrib          = 	dllPushClientAttrib          = glPushClientAttrib;
	qglPushMatrix                = 	dllPushMatrix                = glPushMatrix;
	qglPushName                  = 	dllPushName                  = glPushName;
	qglRasterPos2d               = 	dllRasterPos2d               = glRasterPos2d;
	qglRasterPos2dv              = 	dllRasterPos2dv              = glRasterPos2dv;
	qglRasterPos2f               = 	dllRasterPos2f               = glRasterPos2f;
	qglRasterPos2fv              = 	dllRasterPos2fv              = glRasterPos2fv;
	qglRasterPos2i               = 	dllRasterPos2i               = glRasterPos2i;
	qglRasterPos2iv              = 	dllRasterPos2iv              = glRasterPos2iv;
	qglRasterPos2s               = 	dllRasterPos2s               = glRasterPos2s;
	qglRasterPos2sv              = 	dllRasterPos2sv              = glRasterPos2sv;
	qglRasterPos3d               = 	dllRasterPos3d               = glRasterPos3d;
	qglRasterPos3dv              = 	dllRasterPos3dv              = glRasterPos3dv;
	qglRasterPos3f               = 	dllRasterPos3f               = glRasterPos3f;
	qglRasterPos3fv              = 	dllRasterPos3fv              = glRasterPos3fv;
	qglRasterPos3i               = 	dllRasterPos3i               = glRasterPos3i;
	qglRasterPos3iv              = 	dllRasterPos3iv              = glRasterPos3iv;
	qglRasterPos3s               = 	dllRasterPos3s               = glRasterPos3s;
	qglRasterPos3sv              = 	dllRasterPos3sv              = glRasterPos3sv;
	qglRasterPos4d               = 	dllRasterPos4d               = glRasterPos4d;
	qglRasterPos4dv              = 	dllRasterPos4dv              = glRasterPos4dv;
	qglRasterPos4f               = 	dllRasterPos4f               = glRasterPos4f;
	qglRasterPos4fv              = 	dllRasterPos4fv              = glRasterPos4fv;
	qglRasterPos4i               = 	dllRasterPos4i               = glRasterPos4i;
	qglRasterPos4iv              = 	dllRasterPos4iv              = glRasterPos4iv;
	qglRasterPos4s               = 	dllRasterPos4s               = glRasterPos4s;
	qglRasterPos4sv              = 	dllRasterPos4sv              = glRasterPos4sv;
	qglReadBuffer                = 	dllReadBuffer                = glReadBuffer;
	qglReadPixels                = 	dllReadPixels                = glReadPixels;
	qglRectd                     = 	dllRectd                     = glRectd;
	qglRectdv                    = 	dllRectdv                    = glRectdv;
	qglRectf                     = 	dllRectf                     = glRectf;
	qglRectfv                    = 	dllRectfv                    = glRectfv;
	qglRecti                     = 	dllRecti                     = glRecti;
	qglRectiv                    = 	dllRectiv                    = glRectiv;
	qglRects                     = 	dllRects                     = glRects;
	qglRectsv                    = 	dllRectsv                    = glRectsv;
	qglRenderMode                = 	dllRenderMode                = glRenderMode;
	qglRotated                   = 	dllRotated                   = glRotated;
	qglRotatef                   = 	dllRotatef                   = glRotatef;
	qglScaled                    = 	dllScaled                    = glScaled;
	qglScalef                    = 	dllScalef                    = glScalef;
	qglScissor                   = 	dllScissor                   = glScissor;
	qglSelectBuffer              = 	dllSelectBuffer              = glSelectBuffer;
	qglShadeModel                = 	dllShadeModel                = glShadeModel;
	qglStencilFunc               = 	dllStencilFunc               = glStencilFunc;
	qglStencilMask               = 	dllStencilMask               = glStencilMask;
	qglStencilOp                 = 	dllStencilOp                 = glStencilOp;
	qglTexCoord1d                = 	dllTexCoord1d                = glTexCoord1d;
	qglTexCoord1dv               = 	dllTexCoord1dv               = glTexCoord1dv;
	qglTexCoord1f                = 	dllTexCoord1f                = glTexCoord1f;
	qglTexCoord1fv               = 	dllTexCoord1fv               = glTexCoord1fv;
	qglTexCoord1i                = 	dllTexCoord1i                = glTexCoord1i;
	qglTexCoord1iv               = 	dllTexCoord1iv               = glTexCoord1iv;
	qglTexCoord1s                = 	dllTexCoord1s                = glTexCoord1s;
	qglTexCoord1sv               = 	dllTexCoord1sv               = glTexCoord1sv;
	qglTexCoord2d                = 	dllTexCoord2d                = glTexCoord2d;
	qglTexCoord2dv               = 	dllTexCoord2dv               = glTexCoord2dv;
	qglTexCoord2f                = 	dllTexCoord2f                = glTexCoord2f;
	qglTexCoord2fv               = 	dllTexCoord2fv               = glTexCoord2fv;
	qglTexCoord2i                = 	dllTexCoord2i                = glTexCoord2i;
	qglTexCoord2iv               = 	dllTexCoord2iv               = glTexCoord2iv;
	qglTexCoord2s                = 	dllTexCoord2s                = glTexCoord2s;
	qglTexCoord2sv               = 	dllTexCoord2sv               = glTexCoord2sv;
	qglTexCoord3d                = 	dllTexCoord3d                = glTexCoord3d;
	qglTexCoord3dv               = 	dllTexCoord3dv               = glTexCoord3dv;
	qglTexCoord3f                = 	dllTexCoord3f                = glTexCoord3f;
	qglTexCoord3fv               = 	dllTexCoord3fv               = glTexCoord3fv;
	qglTexCoord3i                = 	dllTexCoord3i                = glTexCoord3i;
	qglTexCoord3iv               = 	dllTexCoord3iv               = glTexCoord3iv;
	qglTexCoord3s                = 	dllTexCoord3s                = glTexCoord3s;
	qglTexCoord3sv               = 	dllTexCoord3sv               = glTexCoord3sv;
	qglTexCoord4d                = 	dllTexCoord4d                = glTexCoord4d;
	qglTexCoord4dv               = 	dllTexCoord4dv               = glTexCoord4dv;
	qglTexCoord4f                = 	dllTexCoord4f                = glTexCoord4f;
	qglTexCoord4fv               = 	dllTexCoord4fv               = glTexCoord4fv;
	qglTexCoord4i                = 	dllTexCoord4i                = glTexCoord4i;
	qglTexCoord4iv               = 	dllTexCoord4iv               = glTexCoord4iv;
	qglTexCoord4s                = 	dllTexCoord4s                = glTexCoord4s;
	qglTexCoord4sv               = 	dllTexCoord4sv               = glTexCoord4sv;
	qglTexCoordPointer           = 	dllTexCoordPointer           = glTexCoordPointer;
	qglTexEnvf                   = 	dllTexEnvf                   = glTexEnvf;
	qglTexEnvfv                  = 	dllTexEnvfv                  = glTexEnvfv;
	qglTexEnvi                   = 	dllTexEnvi                   = glTexEnvi;
	qglTexEnviv                  = 	dllTexEnviv                  = glTexEnviv;
	qglTexGend                   = 	dllTexGend                   = glTexGend;
	qglTexGendv                  = 	dllTexGendv                  = glTexGendv;
	qglTexGenf                   = 	dllTexGenf                   = glTexGenf;
	qglTexGenfv                  = 	dllTexGenfv                  = glTexGenfv;
	qglTexGeni                   = 	dllTexGeni                   = glTexGeni;
	qglTexGeniv                  = 	dllTexGeniv                  = glTexGeniv;
	qglTexImage1D                = 	dllTexImage1D                = glTexImage1D;
	qglTexImage2D                = 	dllTexImage2D                = glTexImage2D;
	qglTexParameterf             = 	dllTexParameterf             = glTexParameterf;
	qglTexParameterfv            = 	dllTexParameterfv            = glTexParameterfv;
	qglTexParameteri             = 	dllTexParameteri             = glTexParameteri;
	qglTexParameteriv            = 	dllTexParameteriv            = glTexParameteriv;
	qglTexSubImage1D             = 	dllTexSubImage1D             = glTexSubImage1D;
	qglTexSubImage2D             = 	dllTexSubImage2D             = glTexSubImage2D;
	qglTranslated                = 	dllTranslated                = glTranslated;
	qglTranslatef                = 	dllTranslatef                = glTranslatef;
	qglVertex2d                  = 	dllVertex2d                  = glVertex2d;
	qglVertex2dv                 = 	dllVertex2dv                 = glVertex2dv;
	qglVertex2f                  = 	dllVertex2f                  = glVertex2f;
	qglVertex2fv                 = 	dllVertex2fv                 = glVertex2fv;
	qglVertex2i                  = 	dllVertex2i                  = glVertex2i;
	qglVertex2iv                 = 	dllVertex2iv                 = glVertex2iv;
	qglVertex2s                  = 	dllVertex2s                  = glVertex2s;
	qglVertex2sv                 = 	dllVertex2sv                 = glVertex2sv;
	qglVertex3d                  = 	dllVertex3d                  = glVertex3d;
	qglVertex3dv                 = 	dllVertex3dv                 = glVertex3dv;
	qglVertex3f                  = 	dllVertex3f                  = glVertex3f;
	qglVertex3fv                 = 	dllVertex3fv                 = glVertex3fv;
	qglVertex3i                  = 	dllVertex3i                  = glVertex3i;
	qglVertex3iv                 = 	dllVertex3iv                 = glVertex3iv;
	qglVertex3s                  = 	dllVertex3s                  = glVertex3s;
	qglVertex3sv                 = 	dllVertex3sv                 = glVertex3sv;
	qglVertex4d                  = 	dllVertex4d                  = glVertex4d;
	qglVertex4dv                 = 	dllVertex4dv                 = glVertex4dv;
	qglVertex4f                  = 	dllVertex4f                  = glVertex4f;
	qglVertex4fv                 = 	dllVertex4fv                 = glVertex4fv;
	qglVertex4i                  = 	dllVertex4i                  = glVertex4i;
	qglVertex4iv                 = 	dllVertex4iv                 = glVertex4iv;
	qglVertex4s                  = 	dllVertex4s                  = glVertex4s;
	qglVertex4sv                 = 	dllVertex4sv                 = glVertex4sv;
	qglVertexPointer             = 	dllVertexPointer             = glVertexPointer;
	qglViewport                  = 	dllViewport                  = glViewport;

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

#endif

	return true;
}

/*
==================
GLimp_EnableLogging
==================
*/
void GLimp_EnableLogging( bool enable ) {

}
