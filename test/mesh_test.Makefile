COMPANY = Daniel Burke
COPYRIGHT = 2020
DESCRIPTION = Wavefront Mesh Loader Test
BINARY_NAME = meshtest
CFLAGS = -std=c11 -Ideps -Isrc -Wall -pedantic
OBJS = mesh.o log.o global.o 3dmaths.o version.o stb_ds.o
OBJS += mesh_test.o
VPATH = src test deps
WIN_LIBS =
LIN_LIBS = -lm
MAC_LIBS = -framework CoreVideo -framework Cocoa

_WIN_OBJS = win32.res $(OBJS)
_LIN_OBJS = $(OBJS)
_MAC_OBJS = $(OBJS)

include src/Makefile
