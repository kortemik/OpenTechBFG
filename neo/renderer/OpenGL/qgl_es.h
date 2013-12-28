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
#include <GLES2/gl2ext.h>
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

//=============
//General defines
//=============
#define GL_GLEXT_VERSION 1 //So that glext.h isn't included in qgl.h

//=============
//Types
//=============
#ifndef GL_VERSION_1_1 // Desktop OpenGL. Should never show up on mobile
	typedef khronos_float_t GLdouble;
	typedef khronos_float_t GLclampd;
#endif
#ifndef GL_ARB_vertex_buffer_object // Desktop OpenGL. Should never show up on mobile
	typedef GLsizeiptr GLsizeiptrARB;
#endif

//=============
//Helper macros
//=============
#define ID_GLES_VAR_DEF( x ) glesSpecific_##x
#define ID_GLES_VAR_REPLACE_DEF( x ) ( glConfig.ID_GLES_VAR_DEF( x ) )

//=============
//Defines for functions and their enums that are defined but may not actually exist
//=============
#ifndef GL_VERSION_1_3
	typedef void (GL_APIENTRYP PFNGLACTIVETEXTUREPROC) (GLenum texture);
#endif
#ifndef GL_VERSION_1_3_DEPRECATED
	typedef void (GL_APIENTRYP PFNGLCLIENTACTIVETEXTUREPROC) (GLenum texture);
#endif

#ifndef GL_TEXTURE0_ARB
	#define GL_TEXTURE0_ARB GL_TEXTURE0
#endif
#ifndef GL_EXT_direct_state_access
	typedef void (GL_APIENTRYP PFNGLBINDMULTITEXTUREEXTPROC) (GLenum texunit, GLenum target, GLuint texture);
#endif

#ifndef GL_ARB_texture_compression
	typedef void (GL_APIENTRYP PFNGLCOMPRESSEDTEXIMAGE2DARBPROC) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid *data);
	typedef void (GL_APIENTRYP PFNGLCOMPRESSEDTEXSUBIMAGE2DARBPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid *data);
	typedef void (GL_APIENTRYP PFNGLGETCOMPRESSEDTEXIMAGEARBPROC) (GLenum target, GLint level, GLvoid *img);
#endif

#ifndef GL_ELEMENT_ARRAY_BUFFER_ARB
	#define GL_ELEMENT_ARRAY_BUFFER_ARB GL_ELEMENT_ARRAY_BUFFER
#endif
#ifndef GL_ARRAY_BUFFER_ARB
	#define GL_ARRAY_BUFFER_ARB GL_ARRAY_BUFFER
#endif
#ifndef GL_DYNAMIC_DRAW_ARB
	#define GL_DYNAMIC_DRAW_ARB GL_DYNAMIC_DRAW
#endif
#ifndef GL_STREAM_DRAW_ARB
	#define GL_STREAM_DRAW_ARB GL_STREAM_DRAW
#endif
#ifndef GL_ARB_vertex_buffer_object
	typedef void (GL_APIENTRYP PFNGLBINDBUFFERARBPROC) (GLenum target, GLuint buffer);
	typedef void (GL_APIENTRYP PFNGLDELETEBUFFERSARBPROC) (GLsizei n, const GLuint *buffers);
	typedef void (GL_APIENTRYP PFNGLGENBUFFERSARBPROC) (GLsizei n, GLuint *buffers);
	typedef GLboolean (GL_APIENTRYP PFNGLISBUFFERARBPROC) (GLuint buffer);
	typedef void (GL_APIENTRYP PFNGLBUFFERDATAARBPROC) (GLenum target, GLsizeiptr size, const GLvoid *data, GLenum usage);
	typedef void (GL_APIENTRYP PFNGLBUFFERSUBDATAARBPROC) (GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid *data);
	typedef void (GL_APIENTRYP PFNGLGETBUFFERSUBDATAARBPROC) (GLenum target, GLintptr offset, GLsizeiptr size, GLvoid *data);
	typedef GLvoid* (GL_APIENTRYP PFNGLMAPBUFFERARBPROC) (GLenum target, GLenum access);
	#ifdef GL_OES_mapbuffer
		typedef PFNGLUNMAPBUFFEROESPROC PFNGLUNMAPBUFFERARBPROC;
	#else
		typedef GLboolean (GL_APIENTRYP PFNGLUNMAPBUFFERARBPROC) (GLenum target);
	#endif
	typedef void (GL_APIENTRYP PFNGLGETBUFFERPARAMETERIVARBPROC) (GLenum target, GLenum pname, GLint *params);
	#ifdef GL_OES_mapbuffer
		typedef PFNGLGETBUFFERPOINTERVOESPROC PFNGLGETBUFFERPOINTERVARBPROC;
	#else
		typedef void (GL_APIENTRYP PFNGLGETBUFFERPOINTERVARBPROC) (GLenum target, GLenum pname, GLvoid* *params);
	#endif
#endif
#ifndef GL_VERSION_3_0
	typedef void (GL_APIENTRYP PFNGLBINDBUFFERRANGEPROC) (GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size);
	typedef const GLubyte * (GL_APIENTRYP PFNGLGETSTRINGIPROC) (GLenum name, GLuint index);
#endif

#ifndef GL_ARB_map_buffer_range
	#ifdef GL_EXT_map_buffer_range
		typedef PFNGLMAPBUFFERRANGEEXTPROC PFNGLMAPBUFFERRANGEPROC;
	#else
		typedef GLvoid* (GL_APIENTRYP PFNGLMAPBUFFERRANGEPROC) (GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access);
	#endif
#endif

#ifndef GL_ARB_draw_elements_base_vertex
	typedef void (GL_APIENTRYP PFNGLDRAWELEMENTSBASEVERTEXPROC) (GLenum mode, GLsizei count, GLenum type, const GLvoid *indices, GLint basevertex);
#endif

#ifndef GL_ARB_vertex_array_object
	#ifdef GL_OES_vertex_array_object
		typedef PFNGLBINDVERTEXARRAYOESPROC PFNGLBINDVERTEXARRAYPROC;
		typedef PFNGLDELETEVERTEXARRAYSOESPROC PFNGLDELETEVERTEXARRAYSPROC;
		typedef PFNGLGENVERTEXARRAYSOESPROC PFNGLGENVERTEXARRAYSPROC;
	#else
		typedef void (GL_APIENTRYP PFNGLBINDVERTEXARRAYPROC) (GLuint array);
		typedef void (GL_APIENTRYP PFNGLDELETEVERTEXARRAYSPROC) (GLsizei n, const GLuint *arrays);
		typedef void (GL_APIENTRYP PFNGLGENVERTEXARRAYSPROC) (GLsizei n, GLuint *arrays);
	#endif
#endif

#ifndef GL_ARB_vertex_program
	typedef void (GL_APIENTRYP PFNGLVERTEXATTRIBPOINTERARBPROC) (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid *pointer);
	typedef void (GL_APIENTRYP PFNGLENABLEVERTEXATTRIBARRAYARBPROC) (GLuint index);
	typedef void (GL_APIENTRYP PFNGLDISABLEVERTEXATTRIBARRAYARBPROC) (GLuint index);
	typedef void (GL_APIENTRYP PFNGLPROGRAMSTRINGARBPROC) (GLenum target, GLenum format, GLsizei len, const GLvoid *string);
	typedef void (GL_APIENTRYP PFNGLBINDPROGRAMARBPROC) (GLenum target, GLuint program);
	typedef void (GL_APIENTRYP PFNGLGENPROGRAMSARBPROC) (GLsizei n, GLuint *programs);
	typedef void (GL_APIENTRYP PFNGLDELETEPROGRAMSARBPROC) (GLsizei n, const GLuint *programs);
	typedef void (GL_APIENTRYP PFNGLPROGRAMENVPARAMETER4FVARBPROC) (GLenum target, GLuint index, const GLfloat *params);
	typedef void (GL_APIENTRYP PFNGLPROGRAMLOCALPARAMETER4FVARBPROC) (GLenum target, GLuint index, const GLfloat *params);
#endif

#ifndef GL_VERSION_2_0
	typedef GLuint (GL_APIENTRYP PFNGLCREATESHADERPROC) (GLenum type);
	typedef void (GL_APIENTRYP PFNGLDELETESHADERPROC) (GLuint shader);
	typedef void (GL_APIENTRYP PFNGLSHADERSOURCEPROC) (GLuint shader, GLsizei count, const GLchar* *string, const GLint *length);
	typedef void (GL_APIENTRYP PFNGLCOMPILESHADERPROC) (GLuint shader);
	typedef void (GL_APIENTRYP PFNGLGETSHADERIVPROC) (GLuint shader, GLenum pname, GLint *params);
	typedef void (GL_APIENTRYP PFNGLGETSHADERINFOLOGPROC) (GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
	typedef GLuint (GL_APIENTRYP PFNGLCREATEPROGRAMPROC) (void);
	typedef void (GL_APIENTRYP PFNGLDELETEPROGRAMPROC) (GLuint program);
	typedef void (GL_APIENTRYP PFNGLATTACHSHADERPROC) (GLuint program, GLuint shader);
	typedef void (GL_APIENTRYP PFNGLDETACHSHADERPROC) (GLuint program, GLuint shader);
	typedef void (GL_APIENTRYP PFNGLLINKPROGRAMPROC) (GLuint program);
	typedef void (GL_APIENTRYP PFNGLUSEPROGRAMPROC) (GLuint program);
	typedef void (GL_APIENTRYP PFNGLGETPROGRAMIVPROC) (GLuint program, GLenum pname, GLint *params);
	typedef void (GL_APIENTRYP PFNGLGETPROGRAMINFOLOGPROC) (GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
	typedef void (GL_APIENTRYP PFNGLBINDATTRIBLOCATIONPROC) (GLuint program, GLuint index, const GLchar *name);
	typedef GLint (GL_APIENTRYP PFNGLGETUNIFORMLOCATIONPROC) (GLuint program, const GLchar *name);
	typedef void (GL_APIENTRYP PFNGLUNIFORM1IPROC) (GLint location, GLint v0);
	typedef void (GL_APIENTRYP PFNGLUNIFORM4FVPROC) (GLint location, GLsizei count, const GLfloat *value);
#endif

#ifndef GL_ARB_get_program_binary
	// Related extensions, different purposes
	#ifdef GL_EXT_separate_shader_objects
		typedef PFNGLPROGRAMPARAMETERIEXTPROC PFNGLPROGRAMPARAMETERIPROC;
	#else
		typedef void (GL_APIENTRYP PFNGLPROGRAMPARAMETERIPROC) (GLuint program, GLenum pname, GLint value);
	#endif
#endif

#ifndef GL_ARB_uniform_buffer_object
	typedef GLuint (GL_APIENTRYP PFNGLGETUNIFORMBLOCKINDEXPROC) (GLuint program, const GLchar *uniformBlockName);
	typedef void (GL_APIENTRYP PFNGLUNIFORMBLOCKBINDINGPROC) (GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding);
#endif

#ifndef GL_ATI_separate_stencil
	typedef void (GL_APIENTRYP PFNGLSTENCILOPSEPARATEATIPROC) (GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass);
	typedef void (GL_APIENTRYP PFNGLSTENCILFUNCSEPARATEATIPROC) (GLenum frontfunc, GLenum backfunc, GLint ref, GLuint mask);
#endif

#ifndef GL_DEPTH_BOUNDS_TEST_EXT
	#define GL_DEPTH_BOUNDS_TEST_EXT 0x8890
#endif
#ifndef GL_EXT_depth_bounds_test
	typedef void (GL_APIENTRYP PFNGLDEPTHBOUNDSEXTPROC) (GLclampd zmin, GLclampd zmax);
#endif

#ifndef GL_SYNC_GPU_COMMANDS_COMPLETE
	#ifdef GL_APPLE_sync
		#define GL_SYNC_GPU_COMMANDS_COMPLETE GL_SYNC_GPU_COMMANDS_COMPLETE_APPLE
	#else
		#define GL_SYNC_GPU_COMMANDS_COMPLETE 0x9117
	#endif
#endif
#ifndef GL_SYNC_FLUSH_COMMANDS_BIT
	#ifdef GL_APPLE_sync
		#define GL_SYNC_FLUSH_COMMANDS_BIT GL_SYNC_FLUSH_COMMANDS_BIT_APPLE
	#else
		#define GL_SYNC_FLUSH_COMMANDS_BIT 0x00000001
	#endif
#endif
#ifndef GL_ARB_sync
	#ifndef GL_APPLE_sync
		typedef khronos_uint64_t GLuint64;
		typedef struct __GLsync *GLsync;
	#endif

	typedef GLsync (GL_APIENTRYP PFNGLFENCESYNCPROC) (GLenum condition, GLbitfield flags);
	typedef GLboolean (GL_APIENTRYP PFNGLISSYNCPROC) (GLsync sync);
	typedef void (GL_APIENTRYP PFNGLDELETESYNCPROC) (GLsync sync);
	typedef GLenum (GL_APIENTRYP PFNGLCLIENTWAITSYNCPROC) (GLsync sync, GLbitfield flags, GLuint64 timeout);
#endif

#ifndef GL_ARB_occlusion_query
	#ifdef GL_EXT_occlusion_query_boolean
		typedef PFNGLGENQUERIESEXTPROC PFNGLGENQUERIESARBPROC;
		typedef PFNGLDELETEQUERIESEXTPROC PFNGLDELETEQUERIESARBPROC;
		typedef PFNGLISQUERYEXTPROC PFNGLISQUERYARBPROC;
		typedef PFNGLBEGINQUERYEXTPROC PFNGLBEGINQUERYARBPROC;
		typedef PFNGLENDQUERYEXTPROC PFNGLENDQUERYARBPROC;
		typedef PFNGLGETQUERYIVEXTPROC PFNGLGETQUERYIVARBPROC;
		typedef PFNGLGETQUERYOBJECTUIVEXTPROC PFNGLGETQUERYOBJECTUIVARBPROC;
	#else
		typedef void (GL_APIENTRYP PFNGLGENQUERIESARBPROC) (GLsizei n, GLuint *ids);
		typedef void (GL_APIENTRYP PFNGLDELETEQUERIESARBPROC) (GLsizei n, const GLuint *ids);
		typedef GLboolean (GL_APIENTRYP PFNGLISQUERYARBPROC) (GLuint id);
		typedef void (GL_APIENTRYP PFNGLBEGINQUERYARBPROC) (GLenum target, GLuint id);
		typedef void (GL_APIENTRYP PFNGLENDQUERYARBPROC) (GLenum target);
		typedef void (GL_APIENTRYP PFNGLGETQUERYIVARBPROC) (GLenum target, GLenum pname, GLint *params);
		typedef void (GL_APIENTRYP PFNGLGETQUERYOBJECTUIVARBPROC) (GLuint id, GLenum pname, GLuint *params);
	#endif
	typedef void (GL_APIENTRYP PFNGLGETQUERYOBJECTIVARBPROC) (GLuint id, GLenum pname, GLint *params);
#endif

#ifndef GL_TIME_ELAPSED_EXT
	#ifdef GL_TIME_ELAPSED
		#define GL_TIME_ELAPSED_EXT GL_TIME_ELAPSED
	#else
		#define GL_TIME_ELAPSED_EXT 0x88BF
	#endif
#endif
#ifndef GL_QUERY_RESULT
	#ifdef GL_QUERY_RESULT_EXT
		#define GL_QUERY_RESULT GL_QUERY_RESULT_EXT
	#else
		#define GL_QUERY_RESULT 0x8866
	#endif
#endif
#ifndef GL_EXT_disjoint_timer_query
	typedef void (GL_APIENTRYP PFNGLGETQUERYOBJECTUI64VEXTPROC) (GLuint id, GLenum pname, GLuint64 *params);
#endif

#ifndef GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB
	#ifdef GL_DEBUG_OUTPUT_SYNCHRONOUS
		#define GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB GL_DEBUG_OUTPUT_SYNCHRONOUS
	#else
		#define GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB 0x8242
	#endif
#endif
#ifndef GL_DEBUG_SEVERITY_LOW_ARB
	#ifdef GL_DEBUG_SEVERITY_LOW
		#define GL_DEBUG_SEVERITY_LOW_ARB GL_DEBUG_SEVERITY_LOW
	#else
		#define GL_DEBUG_SEVERITY_LOW_ARB 0x9148
	#endif
#endif
#ifndef GL_ARB_debug_output
	#ifdef GL_KHR_debug
		typedef PFNGLDEBUGMESSAGECONTROLPROC PFNGLDEBUGMESSAGECONTROLARBPROC;
		typedef PFNGLDEBUGMESSAGEINSERTPROC PFNGLDEBUGMESSAGEINSERTARBPROC;
		typedef PFNGLDEBUGMESSAGECALLBACKPROC PFNGLDEBUGMESSAGECALLBACKARBPROC;
		typedef PFNGLGETDEBUGMESSAGELOGPROC PFNGLGETDEBUGMESSAGELOGARBPROC;
	#else
		typedef void (GL_APIENTRYP *GLDEBUGPROCARB)(GLenum source,GLenum type,GLuint id,GLenum severity,GLsizei length,const GLchar *message,GLvoid *userParam);

		typedef void (GL_APIENTRYP PFNGLDEBUGMESSAGECONTROLARBPROC) (GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint *ids, GLboolean enabled);
		typedef void (GL_APIENTRYP PFNGLDEBUGMESSAGEINSERTARBPROC) (GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *buf);
		typedef void (GL_APIENTRYP PFNGLDEBUGMESSAGECALLBACKARBPROC) (GLDEBUGPROCARB callback, const GLvoid *userParam);
		typedef GLuint (GL_APIENTRYP PFNGLGETDEBUGMESSAGELOGARBPROC) (GLuint count, GLsizei bufsize, GLenum *sources, GLenum *types, GLuint *ids, GLenum *severities, GLsizei *lengths, GLchar *messageLog);
	#endif
#endif

#ifndef GL_READ_FRAMEBUFFER_ANGLE
	#ifdef GL_READ_FRAMEBUFFER_NV
		#define GL_READ_FRAMEBUFFER_ANGLE GL_READ_FRAMEBUFFER_NV
	#elif defined(GL_ES_VERSION_3_0)
		#define GL_READ_FRAMEBUFFER_ANGLE GL_READ_FRAMEBUFFER
	#else
		#define GL_READ_FRAMEBUFFER_ANGLE 0x8CA8
	#endif
#endif
#ifndef GL_DRAW_FRAMEBUFFER_ANGLE
	#ifdef GL_DRAW_FRAMEBUFFER_NV
		#define GL_DRAW_FRAMEBUFFER_ANGLE GL_DRAW_FRAMEBUFFER_NV
	#elif defined(GL_ES_VERSION_3_0)
		#define GL_DRAW_FRAMEBUFFER_ANGLE GL_DRAW_FRAMEBUFFER
	#else
		#define GL_DRAW_FRAMEBUFFER_ANGLE 0x8CA9
	#endif
#endif
#ifndef GL_ANGLE_framebuffer_blit
	typedef void (GL_APIENTRYP PFNGLBLITFRAMEBUFFERANGLEPROC) (GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);
#endif

#ifndef GL_RENDERBUFFER_SAMPLES_ANGLE
	#ifdef GL_RENDERBUFFER_SAMPLES_NV
		#define GL_RENDERBUFFER_SAMPLES_ANGLE GL_RENDERBUFFER_SAMPLES_NV
	#elif defined(GL_ES_VERSION_3_0)
		#define GL_RENDERBUFFER_SAMPLES_ANGLE GL_RENDERBUFFER_SAMPLES
	#else
		#define GL_RENDERBUFFER_SAMPLES_ANGLE 0x8CAB
	#endif
#endif
#ifndef GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE_ANGLE
	#ifdef GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE_NV
		#define GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE_ANGLE GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE_NV
	#elif defined(GL_ES_VERSION_3_0)
		#define GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE_ANGLE GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE
	#else
		#define GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE_ANGLE 0x8D56
	#endif
#endif
#ifndef GL_MAX_SAMPLES_ANGLE
	#ifdef GL_MAX_SAMPLES_NV
		#define GL_MAX_SAMPLES_ANGLE GL_MAX_SAMPLES_NV
	#elif defined(GL_ES_VERSION_3_0)
		#define GL_MAX_SAMPLES_ANGLE GL_MAX_SAMPLES
	#else
		#define GL_MAX_SAMPLES_ANGLE 0x8D57
	#endif
#endif
#ifndef GL_ANGLE_framebuffer_multisample
	typedef void (GL_APIENTRYP PFNGLRENDERBUFFERSTORAGEMULTISAMPLEANGLEPROC) (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);
#endif

#ifndef GL_RENDERBUFFER_SAMPLES_IMG
	#define GL_RENDERBUFFER_SAMPLES_IMG 0x9133
#endif
#ifndef GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE_IMG
	#define GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE_IMG 0x9134
#endif
#ifndef GL_MAX_SAMPLES_IMG
	#define GL_MAX_SAMPLES_IMG 0x9135
#endif
#ifndef GL_TEXTURE_SAMPLES_IMG
	#define GL_TEXTURE_SAMPLES_IMG 0x9136
#endif
#ifndef GL_IMG_multisampled_render_to_texture
	typedef void (GL_APIENTRYP PFNGLFRAMEBUFFERTEXTURE2DMULTISAMPLEIMGPROC) (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLsizei samples);
#endif

//=============
//Defines for enums that are defined but may not actually exist
//=============
#ifndef GL_FRONT_LEFT
	#define GL_FRONT_LEFT 0x0400
#endif
#ifndef GL_FRONT_RIGHT
	#define GL_FRONT_RIGHT 0x0401
#endif
#ifndef GL_BACK_LEFT
	#define GL_BACK_LEFT 0x0402
#endif
#ifndef GL_BACK_RIGHT
	#define GL_BACK_RIGHT 0x0403
#endif

#ifndef GL_TEXTURE_CUBE_MAP_SEAMLESS
	#define GL_TEXTURE_CUBE_MAP_SEAMLESS 0x884F
#endif

#ifndef GL_TEXTURE_CUBE_MAP_EXT
	#define GL_TEXTURE_CUBE_MAP_EXT GL_TEXTURE_CUBE_MAP
#endif

#ifndef GL_TEXTURE_CUBE_MAP_POSITIVE_X_EXT
	#define GL_TEXTURE_CUBE_MAP_POSITIVE_X_EXT GL_TEXTURE_CUBE_MAP_POSITIVE_X
#endif

#ifndef GL_FRAMEBUFFER_SRGB
	#define GL_FRAMEBUFFER_SRGB 0x8DB9
#endif

#ifndef GL_UNPACK_ROW_LENGTH
	#ifdef GL_UNPACK_ROW_LENGTH_EXT
		#define GL_UNPACK_ROW_LENGTH GL_UNPACK_ROW_LENGTH_EXT
	#else
		#define GL_UNPACK_ROW_LENGTH 0x0CF2
	#endif
#endif

#ifndef GL_COMPRESSED_RGBA_S3TC_DXT1_EXT
	#ifdef GL_COMPRESSED_RGBA_S3TC_DXT1_NV
		#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT GL_COMPRESSED_RGBA_S3TC_DXT1_NV
	#else
		#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT 0x83F1
	#endif
#endif
#ifndef GL_COMPRESSED_RGBA_S3TC_DXT5_EXT
	#ifdef GL_COMPRESSED_RGBA_S3TC_DXT5_ANGLE
		#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT GL_COMPRESSED_RGBA_S3TC_DXT5_ANGLE
	#elif defined( GL_COMPRESSED_RGBA_S3TC_DXT5_NV )
		#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT GL_COMPRESSED_RGBA_S3TC_DXT5_NV
	#else
		#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT 0x83F3
	#endif
#endif

#ifndef GL_TEXTURE_LOD_BIAS_EXT
	#define GL_TEXTURE_LOD_BIAS_EXT 0x8501
#endif
#ifndef GL_MAX_TEXTURE_LOD_BIAS
	#define GL_MAX_TEXTURE_LOD_BIAS 0x84FD
#endif
#ifndef GL_MAX_TEXTURE_LOD_BIAS_EXT
	#define GL_MAX_TEXTURE_LOD_BIAS_EXT GL_MAX_TEXTURE_LOD_BIAS
#endif

#ifndef GL_MULTISAMPLE_ARB
	#define GL_MULTISAMPLE_ARB 0x809D
#endif

#ifndef GL_TEXTURE_BORDER_COLOR
	#ifdef GL_TEXTURE_BORDER_COLOR_NV
		#define GL_TEXTURE_BORDER_COLOR GL_TEXTURE_BORDER_COLOR_NV
	#else
		#define GL_TEXTURE_BORDER_COLOR 0x1004
	#endif
#endif
#ifdef GL_CLAMP_TO_BORDER
	#define ID_GLES_REAL_GL_CLAMP_TO_BORDER 0x812D
	#undef GL_CLAMP_TO_BORDER
#else
	#ifdef GL_CLAMP_TO_BORDER_NV
		#define ID_GLES_REAL_GL_CLAMP_TO_BORDER GL_CLAMP_TO_BORDER_NV
	#else
		#define ID_GLES_REAL_GL_CLAMP_TO_BORDER GL_CLAMP_TO_EDGE
	#endif
#endif
#define GL_CLAMP_TO_BORDER ID_GLES_VAR_REPLACE_DEF( GL_CLAMP_TO_BORDER )

#ifdef GL_RGBA8
	#define ID_GLES_REAL_GL_RGBA8 0x8058
	#undef GL_RGBA8
#else
	#ifdef GL_RGBA8_OES
		#define ID_GLES_REAL_GL_RGBA8 GL_RGBA8_OES
	#else
		#define ID_GLES_REAL_GL_RGBA8 GL_RGBA
	#endif
#endif
#define GL_RGBA8 ID_GLES_VAR_REPLACE_DEF( GL_RGBA8 )
// This is a workaround for many renderbuffers only supporting RGBA4 for color-buffers, and
// GL_OES_rgb8_rgba8 not supporting RGBA8 for textures, which is where this is needed
#define GL_RGBA8_FB ID_GLES_VAR_REPLACE_DEF( GL_RGBA8_FB )

// Do this after the above just so that it doesn't effect it
#ifndef GL_RGBA8_OES
	#define GL_RGBA8_OES 0x8058
#endif

#ifndef GL_STENCIL_INDEX
	#define GL_STENCIL_INDEX GL_STENCIL_INDEX8
#endif

#ifndef GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT
	#define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT 0x84FF
#endif

#ifndef GL_TEXTURE_MAX_LEVEL
	#ifdef GL_TEXTURE_MAX_LEVEL_APPLE
		#define GL_TEXTURE_MAX_LEVEL GL_TEXTURE_MAX_LEVEL_APPLE
	#else
		#define GL_TEXTURE_MAX_LEVEL 0x813D
	#endif
#endif

#ifndef GL_PROGRAM_FORMAT_ASCII_ARB
	#define GL_PROGRAM_FORMAT_ASCII_ARB 0x8875
#endif

#ifndef GL_PROGRAM_ERROR_POSITION_ARB
	#define GL_PROGRAM_ERROR_POSITION_ARB 0x864B
#endif

#ifndef GL_PROGRAM_ERROR_STRING_ARB
	#define GL_PROGRAM_ERROR_STRING_ARB 0x8874
#endif

#ifndef GL_SRGB8_ALPHA8_EXT
	#ifdef GL_ES_VERSION_3_0
		#define GL_SRGB8_ALPHA8_EXT GL_SRGB8_ALPHA8
	#else
		#define GL_SRGB8_ALPHA8_EXT 0x8C43
	#endif
#endif

#ifndef GL_DEPTH24_STENCIL8_OES
	#ifdef GL_ES_VERSION_3_0
		#define GL_DEPTH24_STENCIL8_OES GL_DEPTH24_STENCIL8
	#else
		#define GL_DEPTH24_STENCIL8_OES 0x88F0
	#endif
#endif

#ifndef GL_DEPTH_COMPONENT24_OES
	#ifdef GL_ES_VERSION_3_0
		#define GL_DEPTH_COMPONENT24_OES GL_DEPTH_COMPONENT24
	#else
		#define GL_DEPTH_COMPONENT24_OES 0x81A6
	#endif
#endif

#ifdef __cplusplus
}
#endif

#endif
