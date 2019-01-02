/*
Copyright (C) 2019 Daniel Burke

This software is provided 'as-is', without any express or implied
warranty.  In no event will the authors be held liable for any damages
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
3. This notice may not be removed or altered from any source distribution.
*/
#define _XOPEN_SOURCE 700

#include <stdint.h>

#include "global.h"

int killme=0;
int sys_width  = 1980;	/* dimensions of default screen */
int sys_height = 1200;
float sys_dpi = 1.0;
int vid_width  = 1280;	/* dimensions of our part of the screen */
int vid_height = 720;
int mouse_x = 0;
int mouse_y = 0;
int mickey_x = 0;
int mickey_y = 0;
char mouse[8] = {0,0,0,0,0,0,0,0};
char keys[KEYMAX];


///////////////////////////////////////////////////////////////////////////////
//////// sys_time = ticks since app start, where 1s = sys_ticksecond of ticks
///////////////////////////////////////////////////////////////////////////////
#ifdef _WIN32
#include <windows.h>
uint64_t sys_ticksecond = 1000;
static uint64_t sys_time_start = 0;
uint64_t sys_time(void)
{
	uint64_t now;
	QueryPerformanceCounter((LARGE_INTEGER*)&now);
	return now - sys_time_start;
}

void sys_time_init(void)
{
	QueryPerformanceFrequency((LARGE_INTEGER*)&sys_ticksecond);
	QueryPerformanceCounter((LARGE_INTEGER*)&sys_time_start);
}

#else	// Mac & Linux versions are identical


#include <time.h>

uint64_t sys_ticksecond = 1000000000;
static uint64_t sys_time_start = 0;
uint64_t sys_time(void)
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
	return (ts.tv_sec * 1000000000 + ts.tv_nsec) - sys_time_start;
}

void sys_time_init(void)
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
	sys_time_start = ts.tv_sec * 1000000000 + ts.tv_nsec;
}
#endif

///////////////////////////////////////////////////////////////////////////////
//////// sys_browser(url) - opens a webbrowser at the given url
///////////////////////////////////////////////////////////////////////////////

#ifdef _WIN32
void sys_browser(char *url)
{
	ShellExecute(NULL, "open", url, NULL, NULL, SW_SHOWNORMAL);
}

#elif defined __APPLE__
#include <objc/objc.h>
#include <objc/runtime.h>
#include <objc/message.h>
void sys_browser(char *url)
{
//	NSURL *MyNSURL = [NSURL URLWithString:[NSString stringWithUTF8String:url]];
//	[[NSWorkspace sharedWorkspace] openURL:MyNSURL];
	SEL URLWithString = sel_registerName("URLWithString:");
	SEL stringWithUTF8String = sel_registerName("stringWithUTF8String:");
	SEL sharedWorkspace = sel_registerName("sharedWorkspace");
	SEL openURL = sel_registerName("openURL:");
	Class NSURL = objc_getClass("NSURL");
	Class NSString = objc_getClass("NSString");
	Class NSWorkspace = objc_getClass("NSWorkspace");
	id url_nsstring = ((id(*)(Class,SEL, char*))objc_msgSend)(NSString, stringWithUTF8String, url);
	id ns_url = ((id(*)(Class,SEL,id))objc_msgSend)(NSURL, URLWithString, url_nsstring);
	id workspace = ((id(*)(Class,SEL))objc_msgSend)(NSWorkspace, sharedWorkspace);
	((id(*)(id,SEL,id))objc_msgSend)(workspace, openURL, ns_url);
}

#else	// linux version
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
void sys_browser(char *url)
{
	int c=1000;
	char buf[c];
	memset(buf, 0, sizeof(char)*c);
	snprintf(buf, c, "sensible-browser %s &", url);
	system(buf);
}
#endif



