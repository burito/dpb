/* dpb_path - v1.00 - gets the correct path for saved games

   Do this:
      #define DPB_PATH_IMPLEMENTATION
   before you include this file in *one* C or C++ file to create the implementation.

   // i.e. it should look like this:
   #include ...
   #include ...
   #include ...
   #define DPB_PATH_IMPLEMENTATION
   #include "dpb_path.h"

This does one thing, and one thing only. It gets the correct path for a game
to place it's saved games in. I encourage using it something like this...

printf( "%s/id Software/Rage", dpb_path_savedgames() );

On the worst case scenario, someone is running Windows with a version of
Internet Explorer that is *less* than version 4.0 (Win95 with no updates),
this function will return ".", so the result will be...
    ./id Software/Rage

If you think that forward slashes will not work on windows, you are mistaken.
They have been supported on Microsoft OS's since MS-DOS 2.

On Windows, this library depends on kernel32.lib and windows.h. It's almost
impossible to compile a windows binary without them, so for all intents and
purposes, this library has no depenencies.

On Linux & Mac, there are no dependencies.

Of important note, this library deals exclusively in UTF8 strings. On MacOS
you will probably need to use...

    [NSString stringWithUTF8String:dpb_path_savedgames()];

On Windows, if you are using WCHAR_T, you should use...

    char *in = dpb_path_savedgames();
    wchar_t out[MAX_PATH];
    MultiByteToWideChar(CP_UTF8, 0, in, strlen(in), out, MAX_PATH);

If you are using char for your filenames, you don't have to do anything.
If you are using Linux, UTF8 is the default, you don't have to do anything.

The string that dpb_path_savedgames() returns is declared statically, you
should not attempt to free() it.

Why did I write this library? I have 500 games in my Steam Library. Do you
know how many games are storing their saved games in the "Saved Games" folder
that has been the correct place to store them since Windows Vista? 8 games.
And two of them are made by id Software (yes, Rage is one of them).

If you don't like this library, feel free to just cut & paste code out of it
into your project. I made this to solve a problem, and I don't care how the
problem gets solved, it just needs to be solved. Give me credit, or don't give
me credit, I don't care, just use the correct directory.

An example program of this in action is...
//////////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#define DPB_PATH_IMPLEMENTATION
#include "dpb_path.h"

int main(int argc, char* argv[])
{
	printf( "Saved Games: %s\n", dpb_path_savedgames() );
	return 0;
}
//////////////////////////////////////////////////////////////////////////////


Special Thanks to Sean Barrett, for inspiration, and generous cut & pasting
of his license and layout. Check out his stuff, it's much better than mine.
https://github.com/nothings/stb

*/

#ifndef DPBP_INCLUDE_DPB_PATH_H
#define DPBP_INCLUDE_DPB_PATH_H
char* dpb_path_savedgames(void);
#endif // DPBP_INCLUDE_DPB_PATH_H

#ifdef DPB_PATH_IMPLEMENTATION

// SHGetFolderPathA
// https://docs.microsoft.com/en-us/windows/desktop/api/shlobj_core/nf-shlobj_core-shgetfolderpatha
// CSIDL_APPDATA
// https://msdn.microsoft.com/en-au/33d92271-2865-4ebd-b96c-bf293deb4310


// SHGetKnownFolderPath
// https://docs.microsoft.com/en-au/windows/desktop/api/shlobj_core/nf-shlobj_core-shgetknownfolderpath
// FOLDERID_SavedGames
// https://docs.microsoft.com/en-au/windows/desktop/shell/knownfolderid
// {4C5C32FF-BB9D-43b0-B5B4-2D72E54EAAA4}


#ifdef _WIN32
// windows
#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
//#include <shlobj.h>

#ifndef _SHLOBJ_H_
// https://docs.microsoft.com/en-au/windows/desktop/api/shlwapi/ns-shlwapi-_dllversioninfo
typedef struct _DLLVERSIONINFO {
  DWORD cbSize;
  DWORD dwMajorVersion;
  DWORD dwMinorVersion;
  DWORD dwBuildNumber;
  DWORD dwPlatformID;
} DLLVERSIONINFO;
// https://docs.microsoft.com/en-au/windows/desktop/api/shlwapi/ns-shlwapi-_dllversioninfo2
typedef struct _DLLVERSIONINFO2 {
  DLLVERSIONINFO info1;
  DWORD          dwFlags;
  ULONGLONG      ullVersion;
} DLLVERSIONINFO2;

static const GUID FOLDERID_SavedGames = { 0x4c5c32ff, 0xbb9d, 0x43b0, 0xb5, 0xb4, 0x2d, 0x72, 0xe5, 0x4e, 0xaa, 0xa4 };
#endif

// It's supposed to be MAX_PATH = 260, but we don't trust that
static char buf[1000];


char* dpb_path_savedgames(void)
{
	HRESULT ret;
	BOOL bret;
	char* result = ".";
	// shell32.dll, version 4.71

	HMODULE l = LoadLibrary("shell32.dll");
	if( NULL == l )
	{// without shell32.lib, the system is broken
		goto PATH_RETURN;
	}

	HRESULT (*Dllgetversionproc)() =
		(HRESULT(*)())GetProcAddress(l, "DllGetVersion");
	if( NULL == Dllgetversionproc )
	{ // Version 4.0 or less, Windows 95 with Internet Explorer 3.0
		// there is no hope at this point.
		goto PATH_LIBRARY;
	}

	DLLVERSIONINFO2 VerInfo2;
	// this is supposed to work, but fails on Win98
	// VerInfo2.info1.cbSize = sizeof(DLLVERSIONINFO2);
	VerInfo2.info1.cbSize = sizeof(DLLVERSIONINFO);
	ret = Dllgetversionproc(&VerInfo2);
	if( S_OK != ret )
	{
		// problem
//		printf("Dllgetversionproc()\n");
		goto PATH_LIBRARY;
	}

	int major_version = VerInfo2.info1.dwMajorVersion;
	int minor_version = VerInfo2.info1.dwMinorVersion;
/*	printf("Major = %d, Minor = %d\n",
	VerInfo2.info1.dwMajorVersion,
	VerInfo2.info1.dwMinorVersion
	);
*/
	// https://msdn.microsoft.com/en-au/bb776779
	switch(major_version) {
	case 0:
	case 1:
	case 2:
	case 3:
	// alledgedly this can't happen, as it would fail earlier
		result = NULL;
		break;
	case 4:
	{// should check minor_version
		HRESULT (*SHGetSpecialFolderPathA)() =
		(HRESULT(*)())GetProcAddress(l, "SHGetSpecialFolderPathA");
		if( NULL == SHGetSpecialFolderPathA ) {
//			printf("no SHGetSpecialFolderPathA in version %d\n",
//				major_version);
			goto PATH_LIBRARY;
		}
		// int CSIDL_APPDATA = 0x001a;
		bret = SHGetSpecialFolderPathA(NULL, buf, 0x001a, 0);
		if( TRUE != bret )
		{// it failed
//			printf("SHGetSpecialFolderPathA = FALSE\n");
			goto PATH_LIBRARY;
		}
		result = buf;
		break;
	}
	case 5:
	case 6:
	{
		HRESULT (*SHGetFolderPathA)() =
		(HRESULT(*)())GetProcAddress(l, "SHGetFolderPathA");
		if( NULL == SHGetFolderPathA ) {
//			printf("no SHGetFolderPathA in version %d\n",
//				major_version);
			goto PATH_LIBRARY;
		}
		// int CSIDL_APPDATA = 0x001a;
		// SHGFP_TYPE_CURRENT = 0
		ret = SHGetFolderPathA(NULL, 0x001a, NULL, 0, buf);
		if( S_OK != ret )
		{// it failed
//			printf("SHGetFolderPathA = %d\n", ret);
			goto PATH_LIBRARY;
		}
		result = buf;
		break;
	}
	case 7:
	case 8:
	case 9:
	case 10:
	default:
	{
		HRESULT (*SHGetKnownFolderPath)() =
		(HRESULT(*)())GetProcAddress(l, "SHGetKnownFolderPath");
		if( NULL == SHGetKnownFolderPath ) {
//			printf("no SHGetKnownFolderPath in version %d\n",
//				major_version);
			goto PATH_LIBRARY;
		}
		FOLDERID_SavedGames;
		PWSTR ppszPath;

		ret = SHGetKnownFolderPath(&FOLDERID_SavedGames,
			0, NULL, &ppszPath);
		if( S_OK != ret )
		{
//			printf("SHGetKnownFolderPath = %d\n", ret);
			goto PATH_LIBRARY;
		}

		WideCharToMultiByte(CP_UTF8, 0, ppszPath, -1,
			buf, 1000, NULL, NULL);
		result = buf;
		// we've got what we want, but lets clean up nicely

		HMODULE o = LoadLibrary("ole32.dll");
		if( NULL == o )
		{// it failed, we don't care
			goto PATH_LIBRARY;
		}
		void (*CoTaskMemFree)() =
		(void(*)())GetProcAddress(o, "CoTaskMemFree");
		if( NULL != CoTaskMemFree )
		{
			CoTaskMemFree(ppszPath);
		}
		FreeLibrary(o);
	}}

PATH_LIBRARY:
	FreeLibrary(l);
PATH_RETURN:
	return result;
}

#else
#ifdef __APPLE__
// Apple
char* dpb_path_savedgames(void)
{
	return "~/Library/Application Support";
}

#else
// Linux
char* dpb_path_savedgames(void)
{
	return "~/.local/share";
}
#endif
#endif

#endif // DPB_PATH_IMPLEMENTATION
/*

------------------------------------------------------------------------------
This software is available under 2 licenses -- choose whichever you prefer.
------------------------------------------------------------------------------
ALTERNATIVE A - zlib License
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
------------------------------------------------------------------------------
ALTERNATIVE B - Public Domain (www.unlicense.org)
This is free and unencumbered software released into the public domain.
Anyone is free to copy, modify, publish, use, compile, sell, or distribute this
software, either in source code form or as a compiled binary, for any purpose,
commercial or non-commercial, and by any means.
In jurisdictions that recognize copyright laws, the author or authors of this
software dedicate any and all copyright interest in the software to the public
domain. We make this dedication for the benefit of the public at large and to
the detriment of our heirs and successors. We intend this dedication to be an
overt act of relinquishment in perpetuity of all present and future rights to
this software under copyright law.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
------------------------------------------------------------------------------
*/