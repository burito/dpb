#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl.h>
#else
#include <GL/glew.h>
#endif

#include "global.h"
#include "log.h"

#include <math.h>

void gfx_init(void);
void gfx_end(void);
void gfx_swap(void);


double time_start;
int frames = 0;

int main_init(int argc, char *argv[])
{
	gfx_init();

	time_start = (double)sys_time() / (double)sys_ticksecond;
	return 0;
}

void main_loop(void)
{
	glViewport( 0, 0, vid_width, vid_height);
	// test that the keyboard works
	if(keys[KEY_ESCAPE])
	{
		log_info("Shutdown on : Button press (ESC)");
		killme=1;
	}

	// clear the window to green	
	glClearColor(0,1,0,1);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	// draw a blue box in the center of the window
	glColor4f(0,0,1,1);
	glRectf( -0.5, -0.5, 0.5, 0.5);

	// draw a 25 pixel tall box, that scales with the system UI elements
	float y = ( 25.0 / ((float)vid_height / 2.0) ) * sys_dpi;
	glRectf( -0.8, -0.8, 0.8, -0.8+y);

	// Figure out the OpenGL screen coordinates of the mouse
	float mx = (float)mouse_x / (float)vid_width;
	float my = (float)mouse_y / (float)vid_height;
	mx = mx * 2 - 1;
	my = my * 2 - 1;
	my = -my;
	float md = 0.01;
	// draw a red box around the mouse
	glColor4f(1,0,0,1);
	glRectf( mx-md, my-md, mx+md, my+md);

	double now = (double)sys_time() / (double)sys_ticksecond;

	double pos = sin(fmod(now, 2.0*M_PI));
	glRectf(-0.95, pos-md, -0.91, pos+md);

	frames ++;
	if( (now - time_start) > 1.0 )
	{
		time_start += 1.0;
		log_info("fps = %d", frames);
		frames = 0;
	}

	// swap the buffer onto the screen
	gfx_swap();
}

void main_end(void)
{
	gfx_end();
}

