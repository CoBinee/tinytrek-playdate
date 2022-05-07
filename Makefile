HEAP_SIZE      = 8388208
STACK_SIZE     = 61800

PRODUCT = tinytrek.pdx

# Locate the SDK
SDK = ${PLAYDATE_SDK_PATH}
ifeq ($(SDK),)
	SDK = $(shell egrep '^\s*SDKRoot' ~/.Playdate/config | head -n 1 | cut -c9-)
endif

ifeq ($(SDK),)
$(error SDK path not found; set ENV value PLAYDATE_SDK_PATH)
endif

######
# IMPORTANT: You must add your source folders to VPATH for make to find them
# ex: VPATH += src1:src2
######

VPATH += src

# List C source files here
SRC = \
	src/main.c src/Iocs.c \
	src/Aseprite.c src/Scene.c src/Actor.c \
	src/Application.c \
	src/Title.c \
	src/Game.c \
	src/Console.c src/Display.c src/Report.c

# List all user directories here
UINCDIR = 

# List user asm files
UASRC = 

# List all user C define here, like -D_DEBUG=1
UDEFS = 

# Define ASM defines here
UADEFS = 

# List the user directory to look for the libraries here
ULIBDIR =

# List all user libraries here
ULIBS =

include $(SDK)/C_API/buildsupport/common.mk

# phony targets
.PHONY:		tool resource

# Build tools
tool:	
	@g++ -o tools/ttf2fnt `sdl2-config --cflags --libs` -lSDL2_image -lSDL2_ttf -std=c++11 -Wno-format-security tools/src/ttf2fnt.cpp
	@g++ -o tools/chr2png -lpng -std=c++11 -Wno-format-security tools/src/chr2png.cpp

# Build resource
resource:	font image sound launcher

font:
	@tools/ttf2fnt -w=16 -h=16 -x=1 -y=1 -k=1 -white -o Source/fonts/misaki_gothic.fnt res/fonts/misaki_ttf_2021-05-05/misaki_gothic.ttf
	@tools/ttf2fnt -w=8 -h=8 -k=0 -white -o Source/fonts/font-mini.fnt res/fonts/misaki_ttf_2021-05-05/misaki_gothic.ttf
	@cp res/fonts/font-* Source/fonts/

image:
	@cp res/images/*.png Source/images/
	@cp res/images/*.json Source/images/

sound:
	@for f in res/sounds/*.aif; do \
	ffmpeg -y -i $$f -acodec adpcm_ima_wav $${f%.aif}.wav; \
	done
	@cp res/sounds/*.wav Source/sounds/

launcher:
	@cp res/launcher/*.png Source/launcher/
