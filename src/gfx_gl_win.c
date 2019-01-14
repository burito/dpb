
#include <windows.h>
#include <GL/glew.h>
#include <GL/wglew.h>

#include "global.h"
#include "log.h"

HDC hDC;
HGLRC hGLRC;
extern HWND hWnd;

char* win_errormsg(void);


void gfx_resize(void)
{
	glViewport(0, 0, vid_width, vid_height);
}

void gfx_swap(void)
{
#ifdef _WIN32
	SwapBuffers(hDC);
#else
	glXSwapBuffers(display, window);
#endif
}

void gfx_init(void)
{
	HGLRC tmpGLRC;
	hDC = GetDC(hWnd);
	unsigned char bpp = 32;
	PIXELFORMATDESCRIPTOR pfd = {
		sizeof(PIXELFORMATDESCRIPTOR),
		1,				// version number
		PFD_DRAW_TO_WINDOW |
		PFD_SUPPORT_OPENGL |
		PFD_DOUBLEBUFFER |
		PFD_STEREO_DONTCARE,
		PFD_TYPE_RGBA,
		bpp,
		0, 0, 0, 0, 0, 0,		// color bits ignored
		0,				// no alpha buffer
		0,				// shift bit ignored
		0,				// no accumulation buffer
		0, 0, 0, 0,			// accum bits ignored
		bpp,				// 32-bit z-buffer
		0,				// no stencil buffer
		0,				// no auxiliary buffer
		PFD_MAIN_PLANE,			// main layer
		0,				// reserved
		0, 0, 0				// layer masks ignored
	};

	int pf = ChoosePixelFormat(hDC, &pfd);
	if(!pf)
	{
		log_fatal("ChoosePixelFormat() = %s", win_errormsg());
		return;
	}

	if(!SetPixelFormat(hDC, pf, &pfd))
	{
		log_fatal("SetPixelFormat() = %s", win_errormsg());
		return;
	}

	if(!DescribePixelFormat(hDC, pf, sizeof(PIXELFORMATDESCRIPTOR), &pfd))
	{
		log_fatal("DescribePixelFormat() = %s", win_errormsg());
		return;
	}


	tmpGLRC = wglCreateContext(hDC);
	if(!tmpGLRC)
	{
		log_fatal("wglCreateContext() = %s", win_errormsg());
		return;
	}
	if(!wglMakeCurrent(hDC, tmpGLRC))
	{
		log_fatal("wglMakeCurrent() = %s", win_errormsg());
		return;
	}
	glewInit();
	GLint glattrib[] = {
		WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
		WGL_CONTEXT_MINOR_VERSION_ARB, 1,
//		WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
		WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
		WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
//		WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_DEBUG_BIT_ARB,
		0 };

	if(1 == wglewIsSupported("WGL_ARB_create_context"))
	{
		hGLRC = wglCreateContextAttribsARB(hDC, 0, glattrib);
		wglMakeCurrent(NULL, NULL);
		wglDeleteContext(tmpGLRC);
		wglMakeCurrent(hDC, hGLRC);
	}
	else hGLRC = tmpGLRC;

//	gfx_resize();
}

void gfx_end(void)
{
	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(hGLRC);
//	ReleaseDC(hDC);
}

