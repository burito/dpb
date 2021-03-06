/*
Copyright (C) 2018 Daniel Burke

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
#ifndef _WIN32	// this causes msvcrt.dll to be linked (don't know why)
//#define _XOPEN_SOURCE 700
#endif
#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <inttypes.h>
#include <math.h>
#include <time.h>

#include "global.h"
#include "log.h"

#ifdef _WIN32
#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#endif


void log_init(void)
{

#ifdef _WIN32
//	AllocConsole();
//	AttachConsole(GetCurrentProcessId());
//	freopen("CON", "w", stdout);
//	freopen("CON", "w", stderr);

	// enable ANSI codes in windows console (conhost.exe)
	// http://www.nivot.org/blog/post/2016/02/04/Windows-10-TH2-(v1511)-Console-Host-Enhancements
	DWORD mode;
	HANDLE console = GetStdHandle(-11); // STD_OUTPUT_HANDLE
	GetConsoleMode(console, &mode);
	mode = mode | 4; // ENABLE_VIRTUAL_TERMINAL_PROCESSING
	SetConsoleMode(console, mode);
#endif
	sys_time_init();
	log_info("Version     : %s", git_version);
}

static const char * log_label(enum LOG_LEVEL level)
{
	switch(level) {
	case LOG_TRACE:
		return "\x1b[37m[TRACE]\x1b[0m";	// bright white
	case LOG_DEBUG:
		return "\x1b[36m[DEBUG]\x1b[0m";	// cyan
	case LOG_VERBOSE:
		return "\x1b[34m[VERBOSE]\x1b[0m";	// blue
	case LOG_INFO:
		return "\x1b[32m[INFO]\x1b[0m";		// green
	case LOG_WARNING:
		return "\x1b[33m[WARN]\x1b[0m";		// yellow
	case LOG_ERROR:
		return "\x1b[35m[ERROR]\x1b[0m";	// magenta
	case LOG_FATAL:
		return "\x1b[31m[FATAL]\x1b[0m";	// red
	default:
		return "\x1b[0m";		// reset
	}
}


// https://docs.microsoft.com/en-us/windows/console/console-virtual-terminal-sequences
void log_out(char* file, int line, enum LOG_LEVEL level, char *fmt, ...)
{
	struct timespec tv;

	double now = (double)sys_time() / (double)sys_ticksecond;
	tv.tv_nsec = fmod( now, 1.0) * 1000000000.0f;
	tv.tv_sec = (uint64_t)now;

	printf( "%" PRIu64 ".%09ld %s:%d %s ",
		(uint64_t)tv.tv_sec, tv.tv_nsec,
		file, line,
		log_label(level) );

	va_list args;
	va_start(args, fmt);
	vprintf(fmt, args);
	va_end(args);

	printf("\x1b[0m\n");

}
