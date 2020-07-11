# dpb

These files have been popping up in most of my projects, so this is the best way I can think of to consolidate them, to help prevent instances of me having differing versions of them across my projects.
The "dpb" are my initials, in the vein of Sean Barrett's "stb" libraries, although I wouldn't claim these are anywhere near as polished as his work. ~~Hopefully having them in one place like this may spur me on to change that.~~ Something has changed, but not much.

| library | ver | description |
|---------|-----|-------------|
| **[dpb_path.h](src/dpb_path.h)**| 1.0 | gets the correct path that a game should use to store saved games |

## Tests
### gfx_null
Creates the window, with no graphics functionality at all. For testing the window creation & management. Handles input and fullscreen, in the platform appropriate way. All of the programs use this as the base.
* Fullscreen
    * F11 on Windows & Linux
    * ⌃⌘F on Mac
* Quitting
    * Alt+F4 on Windows (provided by OS)
    * ⌘Q on Mac
    * Does Linux have a standard quit keycombo?

### gfx_gl
Initialises OpenGL, clears the window with Green, draws a blue square that is half the size of the window, and a red square around the mouse cursor.

### path_test
Bare bones test of dpb_path.h.

| OS | Output |
|----|--------|
| Windows 10 | C:\Users\Username\Saved Games |
| Windows XP (32-bit) | C:\Documents and Settings\Username\Application Data |
| Windows 98 + IE5 | C:\WINDOWS\Application Data |
| Windows 95 + IE4.72 | C:\WINDOWS\Application Data |
| Windows 3.11 | This program cannot be run in DOS mode. |
| Mac OS X | ~/Library/Application Support |
| Linux | ~/.local/share |

Yes, the version of Internet Explorer matters with Win9x. If you want to be clever and just paste this table into your program, remember that windows doesn't have to install to C:, nor does it have to install into the Windows Directory, all of this can be changed in the registry, and most strings are translated to the local language. It should also work on NT4+IE4, and all versions of windows that are newer.

## Build Environment
### Windows
* Install current Nvidia drivers (451.67)
* Install [msys2-x86_64-20200629.exe](https://www.msys2.org/)
```bash
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-imagemagick mingw-w64-x86_64-clang mingw-w64-x86_64-clang-tools-extra git vim man-pages-posix --disable-download-timeout
```

### Linux
* Install current GPU drivers and compiler
	* ```add-apt-repository ppa:graphics-drivers/ppa```
	* ```apt-get update```
	* ```apt-get install nvidia-410 vulkan-utils build-essential clang imagemagick libx11-dev libxi-dev```

### MacOS
* Install XCode


## Submodules / Credits
* [```deps/small-matrix-inverse```](https://github.com/niswegmann/small-matrix-inverse) - Nis Wegmann
* [```deps/ctap```](https://github.com/jhunt/ctap) - James Hunt
* ```deps/*gl*``` - [GLEW 2.1.0](http://glew.sourceforge.net/)
    * Add ```#define GLEW_STATIC``` to the top of ```glew.h```
* [```deps/hidapi```](https://github.com/signal11/hidapi) - Alan Ott, under BSD License

## Thanks
* Dmytro Ivanov for [osx_app_in_plain_c](https://github.com/jimon/osx_app_in_plain_c), which has been invaluable for the OSX code.

For everything else, I am to blame.
