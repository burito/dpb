

#include <GL/glew.h>
#include <GL/glx.h>

#include "global.h"
#include "log.h"


extern Display *display;
extern Window window;
XVisualInfo *xvis;

GLXContext glx_context;

int xAttrList[] = {
	GLX_RGBA,
	GLX_DOUBLEBUFFER,
	GLX_RED_SIZE, 8,
	GLX_GREEN_SIZE, 8,
	GLX_BLUE_SIZE, 8,
	GLX_DEPTH_SIZE, 24,
	None
};

void gfx_init(void)
{
	int screen_id = DefaultScreen(display);
	xvis = glXChooseVisual(display, screen_id, xAttrList);
	if(!xvis)
	{
		log_fatal("glXChooseVisual() failed");
	}
	glx_context = glXCreateContext(display, xvis, 0, GL_TRUE);
	if(glx_context == NULL)
	{
		log_fatal("glXCreateContext() failed");
	}
//	gfx_resize();
	glXMakeCurrent(display, window, glx_context);

	glewInit();	// belongs after GL context creation
}


void gfx_end(void)
{
	glXMakeCurrent(display, None, NULL);
	glXDestroyContext(display, glx_context);
}


void gfx_resize(void)
{
	// let OpenGL know what's going on
//	glXMakeCurrent(display, window, glx_context);
	glViewport(0, 0, vid_width, vid_height);
//	if(!glXIsDirect(display, glx_context))
//		log_fatal("DRI did not respond to hails.");

}


void gfx_swap(void)
{
	glXSwapBuffers(display, window);
}
