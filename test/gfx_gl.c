#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl.h>
#else
#include <GL/glew.h>
#endif

#include "global.h"
#include "log.h"


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
	if(keys[KEY_ESCAPE])
	{
		log_info("Shutdown on : Button press (ESC)");
		killme=1;
	}
	
	glClearColor(0,1,0,1);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glColor4f(0,0,1,1);
	glRectf( -0.5, -0.5, 0.5, 0.5);

	float mx = (float)mouse_x / (float)vid_width;
	float my = (float)mouse_y / (float)vid_height;
	mx = mx * 2 - 1;
	my = my * 2 - 1;
	my = -my;
	float md = 0.01;

	glColor4f(1,0,0,1);
	glRectf( mx-md, my-md, mx+md, my+md);

//	log_info("win = (%d, %d) mouse = (%d, %d)", vid_width, vid_height, mouse_x, mouse_y);
//	log_info("m = (%f, %f)", mx, my);

	gfx_swap();
}

void main_end(void)
{
	gfx_end();
}

