COMPANY = Daniel Burke
COPYRIGHT = 2019
DESCRIPTION = OpenGL Test
BINARY_NAME = gfx_gl
CFLAGS = -std=c11 -Ideps/include -Isrc -DOLD_OPENGL
OBJS =  log.o global.o version.o gfx_gl.o
VPATH = src test deps

WIN_LIBS = -luser32 -lshell32 -lopengl32 -lgdi32
LIN_LIBS = -lm -lX11 -lGL
MAC_LIBS = -framework CoreVideo -framework Cocoa -framework OpenGL

_WIN_OBJS = win32.o win32.res gfx_gl_win.o glew.o $(OBJS)
_LIN_OBJS = linux_xlib.o gfx_gl_lin.o glew.o $(OBJS)
_MAC_OBJS = osx.o gfx_gl_osx.o $(OBJS)

include src/Makefile

# this has to list everything inside the app bundle
$(MAC_CONTENTS)/_CodeSignature/CodeResources : \
	$(MAC_CONTENTS)/MacOS/$(BINARY_NAME) \
	$(MAC_CONTENTS)/Resources/AppIcon.icns \
	$(MAC_CONTENTS)/Info.plist
	codesign --force --deep --sign - $(BINARY_NAME).app

# copies the binary, and tells it where to find libraries
$(MAC_CONTENTS)/MacOS/$(BINARY_NAME): $(BINARY_NAME).bin
	@mkdir -p $(MAC_CONTENTS)/MacOS
	cp $< $@

