COMPANY = Daniel Burke
COPYRIGHT = 2019
DESCRIPTION = Path Test
BINARY_NAME = pathtest
CFLAGS = -std=c11 -Isrc
OBJS = path_test.o
VPATH = src test
WIN_LIBS = -luser32 -lshell32
LIN_LIBS = -lm -lX11
MAC_LIBS = -framework CoreVideo -framework Cocoa

_WIN_OBJS = win32.res $(OBJS)
_LIN_OBJS = $(OBJS)
_MAC_OBJS = $(OBJS)

include src/Makefile
