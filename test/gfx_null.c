#include "global.h"

int main_init(int argc, char *argv[])
{

	return 0;
}

void main_loop(void)
{

}

void main_end(void)
{

}

void gfx_resize(void)
{

}


#ifdef __APPLE__

//#include <Cocoa/Cocoa.h>


void* _displayLink;

void osx_view_init(void)
{

}

#include <objc/objc.h>
#include <objc/message.h>
extern id NSApp;
void DisplayLinkCallback(void)
{
	if(killme != 0)
	{
//		[NSApp terminate:nil];
		SEL terminate = sel_registerName("terminate:");
		objc_msgSend(NSApp, terminate, 0);
	}

}

#endif
