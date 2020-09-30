COMPANY = Daniel Burke
COPYRIGHT = 2019
DESCRIPTION = 3D Maths Test
BINARY_NAME = mathtest
CFLAGS = -std=c11 -Ideps -Isrc -Wall -pedantic
OBJS = 3dmaths_test.o 3dmaths.o
VPATH = src test
WIN_LIBS =
LIN_LIBS =
MAC_LIBS = -framework CoreVideo -framework Cocoa

_WIN_OBJS = win32.res $(OBJS)
_LIN_OBJS = $(OBJS)
_MAC_OBJS = $(OBJS)

include src/Makefile
