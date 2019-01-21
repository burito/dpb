
CFLAGS = -std=c11 -Ideps/include -Isrc
VPATH = src test build deps
BUILD_DIR = build
CC = gcc -g

OBJ_GLOBAL = $(BUILD_DIR)/log.o $(BUILD_DIR)/global.o

OBJ_GL = $(OBJ_GLOBAL) $(BUILD_DIR)/gl_test.o

win32_null.exe: $(BUILD_DIR)/win32.o $(BUILD_DIR)/gfx_null.o $(OBJ_GLOBAL)
	$(CC) -luser32 -lshell32 $^ -o $@

win32_gl.exe: $(BUILD_DIR)/win32.o $(BUILD_DIR)/gfx_gl_win.o $(BUILD_DIR)/glew.o $(OBJ_GL)
	$(CC) $^ -luser32 -lshell32 -lopengl32 -lgdi32  -o $@

xlib_null.exe: $(BUILD_DIR)/linux_xlib.o $(BUILD_DIR)/gfx_null.o $(OBJ_GLOBAL)
	$(CC) $^ -lX11 -lm  -o $@

xlib_gl.exe: $(BUILD_DIR)/linux_xlib.o $(BUILD_DIR)/gfx_gl_lin.o $(BUILD_DIR)/glew.o $(OBJ_GL)
	$(CC) $^ -luser32 -lshell32 -lopengl32 -lgdi32  -o $@




test_3dmaths.exe: $(BUILD_DIR)/3dmaths.o $(BUILD_DIR)/3dmaths_test.o


clean:
	@rm -rf $(BUILD_DIR)



$(BUILD_DIR)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.exe:
	$(CC) $^ -o $@


# Create build directories
$(shell	mkdir -p $(BUILD_DIR))
