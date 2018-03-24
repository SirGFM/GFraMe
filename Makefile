#=========================================================================
# This Makefile have the following targets:
#   - linux32
#   - linux64
#   - win32
#   - win64
#   - *_debug

#=========================================================================
# Set the target and the lib's version
TARGET := libGFraMe
MAJOR := 0
MINOR := $(MAJOR).4
REV := $(MINOR).1

#=========================================================================
# Run make using bash, so syntax for conditionals works as expected
SHELL := /bin/bash

#=========================================================================
# Set the default CC (may be overriden into something like mingw)
CC ?= gcc

#=========================================================================
# Parse the configuration from the target goal
ifneq (, $(findstring _debug, $(MAKECMDGOALS)))
    MODE := debug
else
    MODE := release
endif
ifneq (, $(findstring linux, $(MAKECMDGOALS)))
    OS := linux
    STRIP := strip
endif
ifneq (, $(findstring win, $(MAKECMDGOALS)))
    ifdef $(OS)
        ifeq ($(OS), linux)
            $(error More than a single OS target was specified)
        endif
    endif
    OS := win
endif
ifneq (, $(findstring 32, $(MAKECMDGOALS)))
    ARCH := 32
endif
ifneq (, $(findstring 64, $(MAKECMDGOALS)))
    ifdef $(ARCH)
        ifneq ($(ARCH), 32)
            $(error More than a single target architecture was specified)
        endif
    endif
    ARCH := 64
endif

#=========================================================================
# Setup CFLAGS and LDFLAGS
myCFLAGS := $(CFLAGS) -Wall -I"./include/" -DHAVE_OPENGL
ifeq ($(OS), win)
    myLDFLAGS := $(LDFLAGS) -lmingw32 -lSDL2main
else
    myLDFLAGS := $(LDFLAGS) -lm
    myCFLAGS := $(myCFLAGS) -fPIC
endif
myLDFLAGS := $(myLDFLAGS) -lSDL2

ifeq ($(ARCH), 64)
    myCFLAGS := $(myCFLAGS) -m64
else
    myCFLAGS := $(myCFLAGS) -m32
endif

ifeq ($(MODE), debug)
    myCFLAGS := $(myCFLAGS) -DGFRAME_DEBUG -O0 -g
endif

#=========================================================================
# Paths and objects
VPATH := src/:tst/
TGTDIR := $(OS)$(ARCH)_$(MODE)
OBJDIR := obj/$(TGTDIR)
WDATADIR := $(OBJDIR)/wavtodata
BINDIR := bin/$(TGTDIR)

OBJS = $(OBJDIR)/gframe_accumulator.o $(OBJDIR)/gframe_animation.o \
    $(OBJDIR)/gframe_assets.o $(OBJDIR)/gframe_object.o \
    $(OBJDIR)/gframe_screen.o $(OBJDIR)/gframe_sprite.o \
    $(OBJDIR)/gframe_spriteset.o $(OBJDIR)/gframe_texture.o \
    $(OBJDIR)/gframe_timer.o $(OBJDIR)/gframe_util.o \
    $(OBJDIR)/gframe_tilemap.o $(OBJDIR)/gframe_audio.o \
    $(OBJDIR)/gframe_audio_player.o $(OBJDIR)/gframe_messagebox.o \
    $(OBJDIR)/gframe_save.o $(OBJDIR)/gframe_hitbox.o \
    $(OBJDIR)/gframe_tween.o $(OBJDIR)/gframe_pointer.o \
    $(OBJDIR)/gframe_keys.o $(OBJDIR)/gframe_controller.o \
    $(OBJDIR)/gframe.o $(OBJDIR)/gframe_log.o \
    $(WDATADIR)/chunk.o $(WDATADIR)/fmt.o $(WDATADIR)/wavtodata.o

#=========================================================================
# Helper build targets
.PHONY: help linux32 linux64 linux32_debug linux64_debug win32 win64 \
    win32_debug win64_debug clean

help:
	@ echo "Build targets:"
	@ echo "  linux32"
	@ echo "  linux64"
	@ echo "  linux32_debug"
	@ echo "  linux64_debug"
	@ echo "  win32"
	@ echo "  win64"
	@ echo "  win32_debug"
	@ echo "  win64_debug"
	@ echo "  clean"

linux32: bin/linux32_release/$(TARGET).so bin/linux32_release/$(TARGET).a
linux32_debug: bin/linux32_debug/$(TARGET).so bin/linux32_debug/$(TARGET).a
linux64: bin/linux64_release/$(TARGET).so bin/linux64_release/$(TARGET).a
linux64_debug: bin/linux64_debug/$(TARGET).so bin/linux64_debug/$(TARGET).a
win32: bin/win32_release/$(TARGET).dll bin/win32_release/$(TARGET).a
win32_debug: bin/win32_debug/$(TARGET).dll bin/win32_debug/$(TARGET).a
win64: bin/win64_release/$(TARGET).dll bin/win64_release/$(TARGET).a
win64_debug: bin/win64_debug/$(TARGET).dll bin/win64_debug/$(TARGET).a

#=========================================================================
# Build targets for Linux
bin/$(TGTDIR)/$(TARGET).so: | bin/$(TGTDIR)/$(TARGET).so.$(MAJOR)
	@ echo "[LNK] $@"
	@ cd bin/$(TGTDIR)/; ln -s $(TARGET).so.$(MAJOR) $(TARGET).so

bin/$(TGTDIR)/$(TARGET).so.$(MAJOR): | bin/$(TGTDIR)/$(TARGET).so.$(MINOR)
	@ echo "[LNK] $@"
	@ cd bin/$(TGTDIR)/; ln -s $(TARGET).so.$(MINOR) $(TARGET).so.$(MAJOR)

bin/$(TGTDIR)/$(TARGET).so.$(MINOR): | bin/$(TGTDIR)/$(TARGET).so.$(REV)
	@ echo "[LNK] $@"
	@ cd bin/$(TGTDIR)/; ln -s $(TARGET).so.$(REV) $(TARGET).so.$(MINOR)

bin/$(TGTDIR)/$(TARGET).so.$(REV): $(OBJS)
	@ echo "[ CC] $@"
	@ $(CC) -shared -Wl,-soname,$(TARGET).$(MJV) -Wl,-export-dynamic \
	    $(myCFLAGS) -o $@ $^ $(myLDFLAGS)
	@ if [ "$(MODE)" == "release" ]; then echo "[STP] $@"; fi
	@ if [ "$(MODE)" == "release" ]; then $(STRIP) $@; fi

#=========================================================================
# Build target for Windows
bin/$(TGTDIR)/$(TARGET).dll: $(OBJS)
	@ echo "[ CC] $@"
	@ $(CC) -shared -Wl,-soname,$(TARGET).$(MJV) -Wl,-export-all-symbols \
	    $(myCFLAGS) -o $@ $^ $(myLDFLAGS)
	@ if [ "$(MODE)" == "release" ]; then echo "[STP] $@"; fi
	@ if [ "$(MODE)" == "release" ]; then $(STRIP) $@; fi

#=========================================================================
# Common build targets
bin/$(TGTDIR)/$(TARGET).a: $(OBJS)
	@ echo "[ AR] $@"
	@ $(AR) -cq $@ $^

obj/$(TGTDIR)/%.o: %.c
	@ echo "[ CC] $< -> $@"
	@ $(CC) $(myCFLAGS) -o $@ -c $<

$(OBJS): | obj/$(TGTDIR) $(WDATADIR) $(BINDIR)

obj/$(TGTDIR):
	@ echo "[MKD] $@"
	@ mkdir -p obj/$(TGTDIR) obj/$(TGTDIR)/opengl

$(WDATADIR):
	@ echo "[MKD] $@"
	@ mkdir -p $(WDATADIR)

$(BINDIR):
	@ echo "[MKD] $@"
	@ mkdir -p $(BINDIR)

clean:
	@ echo "[ RM] .*"
	@ rm -rf obj/
	@ rm -rf bin/
