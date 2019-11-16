
BUILD_DIR = build

default: all

all: gfx_null gfx_gl path_test test_3dmaths
.PHONY: default all gfx_null gfx_gl path_test test_3dmaths


gfx_null:
	$(MAKE) -f test/gfx_null.Makefile

gfx_gl:
	$(MAKE) -f test/gfx_gl.Makefile

path_test:
	$(MAKE) -f test/path_test.Makefile


test_3dmaths:
	$(MAKE) -f test/3dmaths_test.Makefile


clean:
	@rm -rf $(BUILD_DIR) gfx_* *.exe *.bin *.app


# Create build directories
$(shell	mkdir -p $(BUILD_DIR))
