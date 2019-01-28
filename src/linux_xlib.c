/*
Copyright (c) 2012 Daniel Burke

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

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/extensions/XInput2.h>


#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <dlfcn.h>
#include <fcntl.h>
#include <unistd.h>

#include "log.h"
#include "global.h"

///////////////////////////////////////////////////////////////////////////////
//////// Public Interface to the rest of the program
///////////////////////////////////////////////////////////////////////////////

int win_width  = 0;	/* used for switching from fullscreen back to window */
int win_height = 0;
int win_x = 0;
int win_y = 0;

int fullscreen=0;
int fullscreen_toggle=0;

///////////////////////////////////////////////////////////////////////////////
//////// X11 OpenGL window setup
///////////////////////////////////////////////////////////////////////////////

extern unsigned char _binary_lin_icon_head_start[];

Display *display;
Window window;
static int screen;

XSetWindowAttributes xwin_attr;
Cursor cursor_none;
int opcode;		// for XInput


int oldx=20, oldy=20;

void gfx_resize(void);


//char* x11_error(unsigned long result);
//char* x11_opcode(unsigned long opcode);
#include "x11_helper.c"

int x11_error_handler(Display * dpy, XErrorEvent * error)
{
	log_error("X11: %s() = %s",
		x11_opcode(error->request_code),
		x11_error(error->error_code)
	);
	return 1;
}


Atom atom_decorations;
//Atom atom_fullscreen;
unsigned long hints_decorations[5] = { 0, 0, 0, 0, 0};
int hints_size = 1;

void x11_fullscreen_init(void)
{
/*	atom_decorations = XInternAtom(display, "_NET_WM_STATE", True);
	if(atom_decorations != None)
	{
		log_debug("NET_WM_STATE method");
		atom_fullscreen = XInternAtom(display, "_NET_WM_STATE_FULLSCREEN", True);
		if(atom_decorations == None)
			log_warning("problem");

		return;
	}
*/	atom_decorations = XInternAtom(display, "_MOTIF_WM_HINTS", True);
	if(atom_decorations != None)
	{
		log_debug("Motif method");
		hints_decorations[0] = 2;
		hints_size = 5;
		return;
	}
	atom_decorations = XInternAtom(display, "KWM_WIN_DECORATION", True);
	if(atom_decorations != None)
	{
		log_debug("KWM method");
		hints_decorations[0] = 2; // KDE_tinyDecoration
		return;
	}
	atom_decorations = XInternAtom(display, "_WIN_HINTS", True);
	if(atom_decorations != None)
	{
		log_debug("GNOME method");
		return;
	}

}


static void x11_window(void)
{
/*	if(fullscreen)
	{
	XWindowAttributes attr;
	Window root = DefaultRootWindow(display);
	XGetWindowAttributes(display, root, &attr);

	vid_width = attr.width;
	vid_height = attr.height;

	xwin_attr.override_redirect = True;
	window = XCreateWindow(display, RootWindow(display, xvis->screen),
		0, 0, vid_width, vid_height, 0, xvis->depth, InputOutput,
		xvis->visual, CWBorderPixel | CWColormap | CWEventMask
		| CWOverrideRedirect
		, &xwin_attr);


	XMapRaised(display, window);
//		window = DefaultRootWindow(display);
//		XF86VidModeSetViewPort(display, screen, 0, 0);
//		XWarpPointer(display, None, window, 0, 0, 0, 0,
//				vid_width/2, vid_height/2);
	XGrabKeyboard(display, window, True,
		GrabModeAsync, GrabModeAsync, CurrentTime);
//	XGrabPointer(display, window, True,
//			PointerMotionMask | ButtonPressMask | ButtonReleaseMask,
//			GrabModeAsync, GrabModeAsync, window, None, CurrentTime);
//	XDefineCursor(display, window, cursor_none);

	}
	else
*/
	XSetErrorHandler(x11_error_handler);
	int result;
	xwin_attr.override_redirect = False;
	int screen = DefaultScreen(display);

	window = XCreateWindow(display, RootWindow(display, screen),
		0, 0, vid_width, vid_height, 0,
		DefaultDepth(display, screen), InputOutput, DefaultVisual(display, screen),
		CWBorderPixel | CWColormap | CWEventMask | CWOverrideRedirect,
		&xwin_attr);
	// Set the icon
	Atom net_wm_icon = XInternAtom(display, "_NET_WM_ICON", False);
	Atom cardinal = XInternAtom(display, "CARDINAL", False);
	int icon_length = 2 + 256 * 256;
	result = XChangeProperty(display, window, net_wm_icon, cardinal, 32,
		PropModeReplace, _binary_lin_icon_head_start, icon_length);
	if(result != 1)log_error("XChangeProperty() = %s", x11_error(result));


	// I will handle quit messages
	Atom delwm = XInternAtom(display, "WM_DELETE_WINDOW", False);
	result = XSetWMProtocols(display, window, &delwm, 1);
	if(result != 1)log_error("XSetWMProtocols() = %s", x11_error(result));

	// My window shall be named...
	XSetStandardProperties(display, window, binary_name, binary_name, None, NULL, 0, NULL);
//	XSetWMProperties(display, window, binary_name, binary_name, NULL, 0, NULL, NULL, NULL);
	XMapRaised(display, window);

	gfx_resize();
}


int ignore_mouse = 0;
static void x11_init(void)
{
	memset(keys, 0, KEYMAX);
	memset(mouse, 0, 3);

	display = XOpenDisplay(0);
	Screen *screen_ptr = DefaultScreenOfDisplay(display);
	sys_width = XWidthOfScreen(screen_ptr);
	sys_height = XHeightOfScreen(screen_ptr);

	memset(&xwin_attr, 0, sizeof(XSetWindowAttributes));
	screen = DefaultScreen(display);
	xwin_attr.colormap = XCreateColormap(display,
	RootWindow(display, screen), DefaultVisual(display, screen), AllocNone);

	xwin_attr.event_mask = ExposureMask | StructureNotifyMask |
	ButtonPressMask | ButtonReleaseMask | PointerMotionMask |
	KeyPressMask | KeyReleaseMask;

	x11_window();

	static char buf[] = { 0,0,0,0,0,0,0,0 };
	XColor black = { .red=0, .green=0, .blue=0 };
	Pixmap bitmap = XCreateBitmapFromData(display, window, buf, 2, 2);
	cursor_none = XCreatePixmapCursor(display, bitmap, bitmap,
		&black, &black, 0, 0);

	int event, error;
	if(!XQueryExtension(display,"XInputExtension", &opcode, &event, &error))
	{
		log_warning("X Input extension not available");
		return;
	}

	x11_fullscreen_init();
}





static void handle_events(void)
{
	int deltax;
	int deltay;
	XEvent event;
	XIDeviceEvent *e;
	double value;

	mickey_x = mickey_y = 0;

	float x=0, y=0;
	int pos=0;
	while(XPending(display) > 0)
	{
		XNextEvent(display, &event);
		if(XGetEventData(display, &event.xcookie))
		{
			XGenericEventCookie *c = &event.xcookie;
			e = c->data;
			if(c->extension == opcode && c->type == GenericEvent)
			switch(c->evtype) {
//			case XI_RawMotion:
//				print_rawmotion(e);
//				break;
		//	case XI_ButtonPress:
			case XI_Motion:
		//	case XI_KeyPress:
				pos = 0;
				for(int i=0; i < e->valuators.mask_len * 8; i++)
				if(XIMaskIsSet(e->valuators.mask, i))
				{
					value = e->valuators.values[pos];
					log_debug("%d -- %f --",pos, value);
					switch(i){
					case 0:
						if(value > 1.0)
						{
							x=value;
							pos++;
							log_debug("%#+f",x);
						}
						break;
					case 1:
						if(value > 1.0)
						{
							y=value;
							pos++;
							log_debug("\t\t%f",y);
						}
						break;
					default:
						log_debug("%d -- %f --",pos, value);
						break;
					}
				}

				log_debug("%f\t%f\t--",x, y);
				break;
			default:
				break;
			}
			XFreeEventData(display, &event.xcookie);
		}
		else
		switch(event.type) {

		case ConfigureNotify:
			vid_width = event.xconfigure.width;
			vid_height = event.xconfigure.height;
			gfx_resize();
			break;

		case ClientMessage:
			killme=1;
			break;

/* Mouse */
		case ButtonPress:
			switch(event.xbutton.button) {
			case 1:	mouse[0]=1; break;
			case 2:	mouse[1]=1; break;
			case 3:	mouse[2]=1; break;
			}
//			log_debug("mouse[] %d, @ %d", foo, (int)event.xbutton.time);
			break;
		case ButtonRelease:
			switch(event.xbutton.button) {
			case 1:	mouse[0]=0; break;
			case 2:	mouse[1]=0; break;
			case 3:	mouse[2]=0; break;
			}
			break;

		case MotionNotify:
//			log_debug("x=%d, y=%d", event.xmotion.x_root,
//				 event.xmotion.y_root);
			if(ignore_mouse)
			if(event.xmotion.x == vid_width/2)
			{
				ignore_mouse = 0;
				break;
			}
			deltax = mouse_x - event.xmotion.x;
			mouse_x = event.xmotion.x;
			mickey_x += deltax;
			deltay = mouse_y - event.xmotion.y;
			mouse_y = event.xmotion.y;
			mickey_y += deltay;
			break;

/* keyboard */
		case KeyPress:
//			log_debug("keyd[%d] @ %d", event.xkey.keycode, (int)event.xkey.time);
			if(event.xkey.keycode < KEYMAX)
				keys[event.xkey.keycode] = 1;
			break;
		case KeyRelease:
			if(event.xkey.keycode < KEYMAX)
				keys[event.xkey.keycode] = 0;
			if(event.xkey.keycode == KEY_F11)
				fullscreen_toggle=1;
			break;
		}
	}

	if(fullscreen_toggle)
	{
		fullscreen_toggle = 0;
		fullscreen = !fullscreen;

		if( fullscreen )
		{
			log_debug("enter fullscreen");
			// https://stackoverflow.com/a/23940869


//			XUnmapWindow(display, window);
//			XMapWindow(display, window);
			int x, y;
			Window child;
			XWindowAttributes xwa;
			XTranslateCoordinates( display, RootWindow(display, screen), window, 0, 0, &x, &y, &child );
			XGetWindowAttributes( display, window, &xwa );

			log_debug("pos = (%d, %d) size = (%d, %d)", xwa.x, xwa.y, xwa.width, xwa.height);
			log_debug("xy = (%d, %d)", x, y);


			win_x =  - x;
			win_y = - y;
			XGetWindowAttributes( display, window, &xwa );
			win_width = xwa.width;
			win_height = xwa.height;
			log_debug("pos = (%d, %d) size = (%d, %d)", xwa.x, xwa.y, xwa.width, xwa.height);
			log_debug("xy = (%d, %d)", x, y);

			log_debug("win_xy = (%d, %d)", win_x, win_y);

			XFlush(display);
			XChangeProperty( display, window, atom_decorations, atom_decorations, 32, PropModeReplace, (unsigned char*)&hints_decorations, hints_size);
//			Atom atom_list[] = { atom_fullscreen };
//			XChangeProperty( display, window, atom_decorations, XA_ATOM, 32, PropModeReplace, (unsigned char*)atom_list, 1);
//			XSetTransientForHint(display, window, RootWindow(display, screen));
			XFlush(display);

			XMoveResizeWindow(display, window, 0, 0, sys_width, sys_height);

//			XMoveWindow(display, window, 0, 0);
//			XResizeWindow(display, window, sys_width, sys_height);

//			XRaiseWindow(display, window);

//			XSetInputFocus(display, window, RevertToParent, CurrentTime);

		}
		else
		{
			log_debug("exit fullscreen");

//			XWindowAttributes xwa;
//			XGetWindowAttributes(display, window, &xwa);
//			log_debug("pos = (%d, %d) size = (%d, %d)", xwa.x, xwa.y, xwa.width, xwa.height);

//			XFlush(display);

			XDeleteProperty(display, window, atom_decorations);
//			XFlush(display);

			XMoveResizeWindow(display, window, win_x, win_y, win_width, win_height);

//			XResizeWindow(display, window, win_width, win_height);
//			XMoveWindow(display, window, win_x, win_y);


		}


	}
}


static void x11_end(void)
{
	XDestroyWindow(display, window);
	XCloseDisplay(display);
}


int main(int argc, char* argv[])
{
	log_init();
	log_info("Platform    : Xlib");

	x11_init();
	int ret = main_init(argc, argv);
	if(ret)
	{
		x11_end();
		return ret;
	}

	while(!killme)
	{
		handle_events();
		main_loop();
	}

	main_end();
	x11_end();
	return 0;
}


