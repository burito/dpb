COMPANY = Daniel Burke
COPYRIGHT = 2012-2016
DESCRIPTION = Windowing Test
COMMENTS = https://github.com/burito/dpb/
BINARY_NAME = gfx_null
OBJS = gfx_null.o log.o global.o version.o
VPATH = src test build

WIN_LIBS = -luser32 -lshell32
LIN_LIBS = -lm -lX11
MAC_LIBS = -framework CoreVideo -framework Cocoa

_WIN_OBJS = win32.o win32.res $(OBJS)
_LIN_OBJS = linux_xlib.o $(OBJS)
_MAC_OBJS = osx.o $(OBJS)

include src/Makefile

clean:
	@rm -rf build $(BINARY_NAME) $(BINARY_NAME).exe $(BINARY_NAME).bin $(BINARY_NAME).app

$(shell mkdir -p build/win build/lin build/mac)