COMPANY = Daniel Burke
COPYRIGHT = 2019
DESCRIPTION = Windowing Test
BINARY_NAME = gfx_null
CFLAGS = -std=c11 -Isrc
OBJS = gfx_null.o log.o global.o version.o
VPATH = src test

WIN_LIBS = -luser32 -lshell32
LIN_LIBS = -lm -lX11
MAC_LIBS = -framework CoreVideo -framework Cocoa

_WIN_OBJS = win32.o win32.res $(OBJS)
_LIN_OBJS = linux_xlib.o $(OBJS)
_MAC_OBJS = osx.o $(OBJS)

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


clean:
	@rm -rf build $(BINARY_NAME) $(BINARY_NAME).exe $(BINARY_NAME).bin $(BINARY_NAME).app

$(shell mkdir -p build/win build/lin build/mac)