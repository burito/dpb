/*
Copyright (c) 2012-2019 Daniel Burke

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

   1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would be
   appreciated but is not required.

   2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.

   3. This notice may not be removed or altered from any source
   distribution.
*/

#include <stdint.h>

#ifdef _WIN32
#define KEYMAX 512
#elif defined __APPLE__
#define KEYMAX 512
#else
#define KEYMAX 128
#endif


extern int killme;	/* global killswitch */
extern int sys_width;	/* dimensions of default screen */
extern int sys_height;
extern float sys_dpi;
extern int vid_width;	/* dimensions of our part of the screen */
extern int vid_height;
extern int mouse_x;	/* position */
extern int mouse_y;
extern int mickey_x;	/* velocity */
extern int mickey_y;
extern char keys[KEYMAX];
extern char mouse[8];	/* button status 0=up 1=down */
extern int mouse_wheel;
// On mac...
// the Logitech drivers are happy to send up to 8 numbered "mouse" buttons
// http://www.logitech.com/pub/techsupport/mouse/mac/lcc3.9.1.b20.zip
extern int fullscreen;
extern int fullscreen_toggle;

int main_init(int argc, char *argv[]);
void main_loop(void);
void main_end(void);

extern uint64_t sys_ticksecond;	/* ticks in a second */
uint64_t sys_time(void);
void sys_time_init(void);

void sys_browser(char *url);


extern const char git_version[];
extern const char binary_name[];

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif


#include "keyboard.h"


#define KEY_MOD_CHAR	(1<<1)
#define KEY_MOD_SHIFT	(1<<2)
#define KEY_MOD_LSHIFT	(1<<3)
#define KEY_MOD_RSHIFT	(1<<4)
#define KEY_MOD_ALT	(1<<5)
#define KEY_MOD_LALT	(1<<6)
#define KEY_MOD_RALT	(1<<7)
#define KEY_MOD_CTRL	(1<<8)
#define KEY_MOD_LCTRL	(1<<9)
#define KEY_MOD_RCTRL	(1<<10)
#define KEY_MOD_LOGO	(1<<11)
#define KEY_MOD_LLOGO	(1<<12)
#define KEY_MOD_RLOGO	(1<<13)
#define KEY_MOD_MENU	(1<<14)


struct sys_event {
	enum type {
		EVENT_KEY_DOWN,
		EVENT_KEY_UP
	} type;
	uint32_t charcode;
	uint16_t keycode;
	uint16_t modifiers;
};
void sys_event_init(void);
int sys_event_read(struct sys_event *event);
int sys_event_write(struct sys_event event);
uint16_t sys_key_modifiers(void);

// For the graphics library, whatever is used
void gfx_init(void);
void gfx_end(void);
void gfx_swap(void);
