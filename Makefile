
BUILD_DIR = build

gfx_null:
	$(MAKE) -f test/gfx_null.Makefile

gfx_gl:
	$(MAKE) -f test/gfx_gl.Makefile



test_3dmaths.exe: $(BUILD_DIR)/3dmaths.o $(BUILD_DIR)/3dmaths_test.o


clean:
	@rm -rf $(BUILD_DIR)


# Create build directories
$(shell	mkdir -p $(BUILD_DIR))
