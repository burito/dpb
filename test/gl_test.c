#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl.h>
#else
#include <GL/glew.h>
#endif

#include "global.h"


void gfx_init(void);
void gfx_end(void);
void gfx_swap(void);

int main_init(int argc, char *argv[])
{
	gfx_init();
	return 0;
}

void main_loop(void)
{
	glClearColor(0,1,0,1);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glColor4f(0,0,1,1);
	glRectf( -0.5, -0.5, 0.5, 0.5);
	gfx_swap();
}

void main_end(void)
{
	gfx_end();
}

