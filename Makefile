
CFLAGS = -std=c11 -Ideps/include -Isrc
VPATH = src test build
BUILD_DIR = build
CC = clang -g



test_3dmaths.exe: $(BUILD_DIR)/3dmaths.o $(BUILD_DIR)/3dmaths_test.o


clean:
	@rm -rf $(BUILD_DIR)



$(BUILD_DIR)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.exe:
	$(CC) $^ -o $@


# Create build directories
$(shell	mkdir -p $(BUILD_DIR))
