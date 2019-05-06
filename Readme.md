dpb
===

These files have been popping up in most of my projects, so this is the best way I can think of to consolidate them, to help prevent instances of me having differing versions of them across my projects.
The "dpb" are my initials, in the vein of Sean Barrett's "stb" libraries, although I wouldn't claim these are anywhere near as polished as his work. ~~Hopefully having them in one place like this may spur me on to change that.~~ Something has changed, but not much.

| library | ver | description |
|---------|-----|-------------|
| **[dpb_path.h](src/dpb_path.h)**| 1.0 | gets the correct path that a game should use to store saved games |

gfx_null
--------
Creates the window, with no graphics functionality at all. For testing the window creation & management. Handles input and fullscreen, in the platform appropriate way. All of the programs use this as the base.
* Fullscreen
    * F11 on Windows & Linux
    * ⌃⌘F on Mac
* Quitting
    * Alt+F4 on Windows (provided by OS)
    * ⌘Q on Mac
    * Does Linux have a standard quit keycombo?

gfx_gl
------
Initialises OpenGL, clears the window with Green, draws a blue square that is half the size of the window, and a red square around the mouse cursor.

Build Environment
-----------------
### Windows
* Install [mingw-w64-install.exe](http://sourceforge.net/projects/mingw-w64/files/) 8.1.0-x86_64-posix-seh
* Add its ```bin``` directory to your path
* Install current GPU drivers
	* Nvidia 417.35
* Install [ImageMagick](http://www.imagemagick.org/script/download.php#windows)

### Linux
* Install current GPU drivers and compiler
	* ```add-apt-repository ppa:graphics-drivers/ppa```
	* ```apt-get update```
	* ```apt-get install nvidia-410 vulkan-utils build-essential clang imagemagick libx11-dev libxi-dev```

### MacOS
* Install XCode


Submodules / Credits
====================
* [```deps/small-matrix-inverse```](https://github.com/niswegmann/small-matrix-inverse) - Nis Wegmann
* [```deps/ctap```](https://github.com/jhunt/ctap) - James Hunt
* ```deps/*gl*``` - [GLEW 2.1.0](http://glew.sourceforge.net/)
    * Add ```#define GLEW_STATIC``` to the top of ```glew.h```

Thanks
======
* Dmytro Ivanov for [osx_app_in_plain_c](https://github.com/jimon/osx_app_in_plain_c), which has been invaluable for the OSX code.

For everything else, I am to blame.
