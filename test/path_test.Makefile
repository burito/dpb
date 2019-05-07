COMPANY = Daniel Burke
COPYRIGHT = 2019
DESCRIPTION = Path Test
BINARY_NAME = pathtest
CFLAGS = -std=c11 -Isrc -Wall -pedantic
OBJS = path_test.o
VPATH = src test
WIN_LIBS =
LIN_LIBS =
MAC_LIBS = -framework CoreVideo -framework Cocoa

_WIN_OBJS = win32.res $(OBJS)
_LIN_OBJS = $(OBJS)
_MAC_OBJS = $(OBJS)

include src/Makefile
