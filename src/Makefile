SHELL=bash
# *** Platform detection
UNAME := $(shell uname)
ifeq ($(UNAME), Darwin)
# MacOS
BUILD_DIR = $(MAC_DIR)
GLSLANG = deps/vulkan-lib/mac/glslangValidator
CC = clang -g
default: $(BINARY_NAME).app

else
# Windows & Linux need ImageMagick, lets check for it
ifeq (magick,$(findstring magick, $(shell which magick 2>&1))) # current ImageMagick looks like this
MAGICK = magick convert
else
	ifeq (convert,$(findstring convert, $(shell which convert 2>&1))) # Ubuntu ships a very old ImageMagick that looks like this
MAGICK = convert
	else
$(error Can't find ImageMagick installation, try...)
		ifeq ($(UNAME), Linux)
			$(error		apt-get install imagemagick)
		else
			$(error		https://www.imagemagick.org/script/download.php)
		endif
	endif
endif # ImageMagick check done!

ifeq ($(UNAME), Linux)
# Linux
BUILD_DIR = $(LIN_DIR)
GLSLANG = deps/vulkan-lib/lin/glslangValidator
CC = clang -g
default: $(BINARY_NAME)

else
# Windows
BUILD_DIR = $(WIN_DIR)
GLSLANG = deps/vulkan-lib/win/glslangValidator.exe
WINDRES = windres
CC = gcc -g
default: $(BINARY_NAME).exe
endif
endif


WIN_DIR = build/win
LIN_DIR = build/lin
MAC_DIR = build/mac

WIN_OBJS = $(patsubst %,$(WIN_DIR)/%,$(_WIN_OBJS))
LIN_OBJS = $(patsubst %,$(LIN_DIR)/%,$(_LIN_OBJS))
MAC_OBJS = $(patsubst %,$(MAC_DIR)/%,$(_MAC_OBJS))


$(WIN_DIR)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(LIN_DIR)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(MAC_DIR)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(MAC_DIR)/%.o: %.m
	$(CC) $(CFLAGS) -c $< -o $@

$(BINARY_NAME).exe: $(WIN_OBJS)
	$(CC) $^ $(WIN_LIBS) -o $@

$(BINARY_NAME): $(LIN_OBJS) $(LIN_DIR)/xicon.o
	$(CC) $^ $(LIN_LIBS) -o $@


$(BINARY_NAME).bin: $(MAC_OBJS)
	$(CC) $(CFLAGS) $^ $(MAC_LIBS) -o $@

# start build the win32 Resource File (contains the icon)
$(WIN_DIR)/Icon.ico: Icon.png
	$(MAGICK) -resize 256x256 $^ $@
$(WIN_DIR)/win32.rc: win32.rc.pre
	cpp -E -P $^ -o $@ -traditional -DCOMMENTS="\"$(COMMENTS)\"" -DCOPYRIGHT="\"\xA9 $(COPYRIGHT) $(COMPANY)\"" -DCOMPANY="\"$(COMPANY)\"" -DDESCRIPTION="\"$(DESCRIPTION)\"" -DGIT_VERSION="\"$(GIT_VERSION)\"" -DBINARY_NAME="\"$(BINARY_NAME)\""
$(WIN_DIR)/win32.res: $(WIN_DIR)/win32.rc $(WIN_DIR)/Icon.ico
	$(WINDRES) -I $(WIN_DIR) -O coff $< -o $@
# end build the win32 Resource File

# crazy stuff to get icons on x11
$(LIN_DIR)/x11icon: $(LIN_DIR)/x11icon.o
	$(CC) $^ -o $@
$(LIN_DIR)/icon.rgba: Icon.png
	$(MAGICK) -resize 256x256 $^ $@
$(LIN_DIR)/icon.argb: $(LIN_DIR)/icon.rgba $(LIN_DIR)/x11icon
	$(LIN_DIR)/x11icon < $(LIN_DIR)/icon.rgba > $@
$(LIN_DIR)/icon.head: $(LIN_DIR)/icon.argb
	printf "\x00\x01\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00" | cat - $^ > $@
$(LIN_DIR)/xicon.o: $(LIN_DIR)/icon.head
	ld -r -b binary $^ -o $@
# end linux icon craziness

# generate the Apple Icon file from src/Icon.png
$(MAC_DIR)/AppIcon.iconset:
	mkdir $@
$(MAC_DIR)/AppIcon.iconset/icon_512x512@2x.png: Icon.png $(MAC_DIR)/AppIcon.iconset
	cp $< $@
$(MAC_DIR)/AppIcon.iconset/icon_512x512.png: Icon.png $(MAC_DIR)/AppIcon.iconset
	sips -Z 512 $< --out $@ 1>/dev/null
$(MAC_DIR)/AppIcon.icns: $(MAC_DIR)/AppIcon.iconset/icon_512x512@2x.png $(MAC_DIR)/AppIcon.iconset/icon_512x512.png
	iconutil -c icns $(MAC_DIR)/AppIcon.iconset
# end generate Mac Icon

MAC_CONTENTS = $(BINARY_NAME).app/Contents

.PHONY: $(BINARY_NAME).app
$(BINARY_NAME).app : $(MAC_CONTENTS)/_CodeSignature/CodeResources

.DELETE_ON_ERROR :

$(MAC_CONTENTS)/Info.plist: src/Info.plist.pre
	@mkdir -p $(MAC_CONTENTS)
	sed -e "s/COPYRIGHT/$(COPYRIGHT)/" -e "s/COMPANY/$(COMPANY)/" -e "s/GIT_VERSION/$(GIT_VERSION)/" -e "s/BINARY_NAME/$(BINARY_NAME)/" < $^ > $@

$(MAC_CONTENTS)/Resources/AppIcon.icns: $(MAC_DIR)/AppIcon.icns
	@mkdir -p $(MAC_CONTENTS)/Resources
	cp $< $@

# create the version info
GIT_VERSION:=$(shell git describe --dirty --always --tags)
VERSION:=const char git_version[] = "$(GIT_VERSION)";const char binary_name[] = "$(BINARY_NAME)";
SRC_VERSION:=$(shell mkdir -p build && cat build/version.c 2>/dev/null)
ifneq ($(SRC_VERSION),$(VERSION))
$(shell echo '$(VERSION)' > build/version.c)
endif
