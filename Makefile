
#==============================================================================
# Import the configurations
#==============================================================================
  include Makefile.conf
#==============================================================================

#==============================================================================
# Clear the suffixes' default rule, since there's an explicit one
#==============================================================================
.SUFFIXES:
#==============================================================================

#==============================================================================
# Define all targets that doesn't match its generated file
#==============================================================================
.PHONY: emscript fast fast_all release install install_win install_x \
        uninstall uninstall_win uninstall_x clean emscript_clean distclean
#==============================================================================

#==============================================================================
# Define compilation target
#==============================================================================
  TARGET := libGFraMe
  MAJOR_VERSION := 2
  MINOR_VERSION := 3
  REV_VERSION := 3
# If the DEBUG flag was set, generate another binary (so it doesn't collide
# with the release one)
  ifeq ($(DEBUG), yes)
    TARGET := $(TARGET)_dbg
  endif
#==============================================================================

#==============================================================================
# Clean the backend objects
#==============================================================================
  BKEND_OBJS = 
#==============================================================================

#==============================================================================
# Define every object required by compilation
#==============================================================================
  OBJS = \
          $(OBJDIR)/gframe.o \
          $(OBJDIR)/gfmAccumulator.o \
          $(OBJDIR)/gfmAnimation.o \
          $(OBJDIR)/gfmCamera.o \
          $(OBJDIR)/gfmDebug.o \
          $(OBJDIR)/gfmError.o \
          $(OBJDIR)/gfmGeometry.o \
          $(OBJDIR)/gfmGroup.o \
          $(OBJDIR)/gfmHitbox.o \
          $(OBJDIR)/gfmInput.o \
          $(OBJDIR)/gfmLog.o \
          $(OBJDIR)/gfmObject.o \
          $(OBJDIR)/gfmParser.o \
          $(OBJDIR)/gfmQuadtree.o \
          $(OBJDIR)/gfmSave.o \
          $(OBJDIR)/gfmSprite.o \
          $(OBJDIR)/gfmSpriteset.o \
          $(OBJDIR)/gfmString.o \
          $(OBJDIR)/gfmText.o \
          $(OBJDIR)/gfmTilemap.o \
          $(OBJDIR)/gfmUtils.o \
          $(OBJDIR)/util/gfmAudio_mml.o \
          $(OBJDIR)/util/gfmAudio_vorbis.o \
          $(OBJDIR)/util/gfmAudio_wave.o \
          $(OBJDIR)/util/gfmFPSCounter.o \
          $(OBJDIR)/util/gfmGroupHelpers.o \
          $(OBJDIR)/util/gfmKeyNode.o \
          $(OBJDIR)/util/gfmParserCommon.o \
          $(OBJDIR)/util/gfmTileAnimation.o \
          $(OBJDIR)/util/gfmTileType.o \
          $(OBJDIR)/util/gfmTrie.o \
          $(OBJDIR)/util/gfmVideo_bmp.o \
          $(OBJDIR)/util/gfmVirtualKey.o \
          $(OBJDIR)/core/event/desktop/gfmEvent_desktop.o \
          $(OBJDIR)/core/loadAsync/gfmLoadAsync_SDL2.o
# Add objects based on the current backend
  ifeq ($(USE_GL3_VIDEO), yes)
    include src/core/video/opengl3/Makefile
  endif
  ifeq ($(USE_SDL2_VIDEO), yes)
    include src/core/video/sdl2/Makefile
  endif
  ifeq ($(USE_SWSDL2_VIDEO), yes)
    include src/core/video/sw_sdl2/Makefile
  endif

  ifndef ($(BACKEND))
    include src/core/sdl2/Makefile
  endif
  ifeq ($(BACKEND), emscript)
    include src/core/emscript-sdl2/Makefile
  endif
# Use the stdio file interface on desktops
  OBJS += $(OBJDIR)/core/common/gfmFile.o
# Add GIF exporter, by default
  ifneq ($(EXPORT_GIF), no)
    OBJS += $(OBJDIR)/core/common/gfmGifExporter.o
  else
    OBJS += $(OBJDIR)/core/noip/gfmGifExporter.o
  endif
  OBJS += $(BKEND_OBJS)
#==============================================================================

#==============================================================================
# Set OS flag
#==============================================================================
  UNAME := $(shell uname)
  OS ?= $(UNAME)
  ifneq (, $(findstring $(UNAME), Windows_NT))
    OS := Win
    UNAME := Win
  endif
  ifneq (, $(findstring $(UNAME), MINGW))
    OS := Win
    UNAME := Win
  endif
  ifneq (, $(findstring $(UNAME), MSYS))
    OS := Win
    UNAME := Win
  endif
  ifeq ($(CC), emcc)
    OS := emscript
  endif
#==============================================================================

#==============================================================================
# Set the binary extensions (if on Windows, since some rules requires it)
#==============================================================================
  ifeq ($(OS), Win)
    BIN_EXT := .exe
  else
    BIN_EXT :=
  endif
#==============================================================================

#==============================================================================
# Define CFLAGS (compiler flags)
#==============================================================================
# Add all warnings and default include path
  CFLAGS := $(CFLAGS) -Wall -I"./include/" -I"./src/include" -DHAVE_OPENGL
# Add architecture flag
  ARCH ?= $(shell uname -m)
  ifeq ($(OS), emscript)
    CFLAGS := $(CFLAGS) -I"$(EMSCRIPTEN)/system/include/" -m32 -DALIGN=4
  else
    ifeq ($(ARCH), x86_64)
      CFLAGS := $(CFLAGS) -m64 -DALIGN=8
    else
      CFLAGS := $(CFLAGS) -m32 -DALIGN=4
    endif
  endif
# Add debug flags
  ifeq ($(OS), emscript)
    CFLAGS := $(CFLAGS) -O2
  else
    ifneq ($(RELEASE), yes)
      CFLAGS := $(CFLAGS) -g -O0 -DDEBUG
    else
      CFLAGS := $(CFLAGS) -O3
    endif
  endif
# Force fps counter, if requested
  ifeq ($(FPS_COUNTER), yes)
    CFLAGS := $(CFLAGS) -DFORCE_FPS
  endif
# Set flags required by OS
  ifeq ($(UNAME), Win)
    CFLAGS := $(CFLAGS) -I"/d/windows/mingw/include" -I"/c/c_synth/include/"
  endif
  ifneq ($(OS), Win)
    CFLAGS := $(CFLAGS) -fPIC
  endif
# Set the current compiler
  ifeq ($(OS), emscript)
    CFLAGS := $(CFLAGS) -DEMCC -s USE_SDL=2
  endif

  ifeq ($(USE_GL3_VIDEO), yes)
    CFLAGS := $(CFLAGS) -DUSE_GL3_VIDEO
  endif
  ifeq ($(USE_SDL2_VIDEO), yes)
    CFLAGS := $(CFLAGS) -DUSE_SDL2_VIDEO
  endif
  ifeq ($(USE_SWSDL2_VIDEO), yes)
    CFLAGS := $(CFLAGS) -DUSE_SWSDL2_VIDEO
  endif
#==============================================================================

#==============================================================================
# Define LDFLAGS (linker flags)
#==============================================================================
# Add libs and paths required by an especific OS
  ifeq ($(UNAME), Win)
    ifeq ($(ARCH), x64)
      LDFLAGS := $(LDFLAGS) -L"/d/windows/mingw/lib" -L"/c/c_synth/lib/"
    else
      LDFLAGS := $(LDFLAGS) -L"/d/windows/mingw/mingw32/lib" -L"/c/c_synth/lib/"
    endif
  endif
  ifeq ($(OS), Win)
    LDFLAGS := $(LDFLAGS) -lmingw32 -lSDL2main
  else
    LDFLAGS := $(LDFLAGS) -L/usr/lib/c_synth/
  endif
# Add SDL2 lib
  LDFLAGS := $(LDFLAGS) -lSDL2
# Add the MML synthesizer
  LDFLAGS := $(LDFLAGS) -lCSynth
# Add OpenGL lib
 ifeq ($(USE_GL3_VIDEO), yes)
   ifeq ($(OS), Win)
     LDFLAGS := $(LDFLAGS) -lopengl32
   else
     LDFLAGS := $(LDFLAGS) -lGL
   endif
 endif
#==============================================================================

#==============================================================================
# Define where source files can be found and where objects & binary are output
#==============================================================================
 VPATH := src:tst
 TESTDIR := tst
 OBJDIR := obj/$(OS)
 BINDIR := bin/$(OS)
 WDATADIR := $(OBJDIR)/wavtodata

 PREFIX ?= /usr
 LIBPATH := $(PREFIX)/lib/GFraMe
 HEADERPATH := $(PREFIX)/include/GFraMe
#==============================================================================

#==============================================================================
# (Automatically) create a list with every .c file used
#==============================================================================
 ALL_SRC := $(OBJS:$(OBJDIR)%.o=src%.c)
#==============================================================================

#==============================================================================
# Automatically look up for tests and compile them
#==============================================================================
 TEST_SRC := $(wildcard $(TESTDIR)/*.c)
 TEST_BIN := $(TEST_SRC:%.c=%$(BIN_EXT))
#==============================================================================

#==============================================================================
# Make the objects list constant (and the icon, if any)
#==============================================================================
 OBJS := $(OBJS)
#==============================================================================

#==============================================================================
# Set shared library's extension
#==============================================================================
 ifeq ($(OS), Win)
   SO := dll
   MJV := $(SO)
   MNV := $(SO)
 else
   SO ?= so
   MJV := $(SO).$(MAJOR_VERSION)
   MNV := $(SO).$(MAJOR_VERSION).$(MINOR_VERSION).$(REV_VERSION)
 endif
#==============================================================================

#==============================================================================
# Get the number of cores for fun stuff
#==============================================================================
 ifeq ($(UNAME), Win)
   CORES := 1
 else
   CORES := $$(($(shell nproc) * 2))
 endif
#==============================================================================

#==============================================================================
# Define default compilation rule
#==============================================================================
all: static shared tests
	date
#==============================================================================

#==============================================================================
# Define the release rule, to compile everything on RELEASE mode (it's done in
# quite an ugly way.... =/)
#==============================================================================
release: MAKEDIRS
	# Remove all old binaries
	make clean
	# Compile everything in release mode
	make RELEASE=yes fast
	# Remove all debug info from the binaries
	strip $(BINDIR)/$(TARGET).a
	strip $(BINDIR)/$(TARGET).$(MNV)
	# Delete all .o to recompile as debug
	rm -f $(OBJS)
	# Recompile the lib with debug info
	make DEBUG=yes fast
	date
#==============================================================================

#==============================================================================
# Rule for building a object file for emscript
#==============================================================================
emscript: bin/emscript/$(TARGET).bc
#==============================================================================

#==============================================================================
# Rule for cleaning emscript build... It's required to modify the CC
#==============================================================================
emscript_clean: clean
#==============================================================================

#==============================================================================
# Rule for building the static lib
#==============================================================================
static: MAKEDIRS $(BINDIR)/$(TARGET).a
#==============================================================================

#==============================================================================
# Rule for building the shared libs
#==============================================================================
shared: MAKEDIRS $(BINDIR)/$(TARGET).$(SO)
#==============================================================================

#==============================================================================
# Rule for building tests
#==============================================================================
tests: MAKEDIRS static $(TEST_BIN)
#==============================================================================

#==============================================================================
# Rule for installing the library
#==============================================================================
ifeq ($(OS), Win)
install: install_win
else
install: install_x
endif

install_win: release
	# Create destiny directories
	mkdir -p /c/GFraMe/lib/
	mkdir -p /c/GFraMe/include/GFrame
	# Copy every shared lib (normal and debug)
	cp -f $(BINDIR)/$(TARGET)*.$(MNV) /c/GFraMe/lib
	# -P = don't follow sym-link
	cp -fP $(BINDIR)/$(TARGET)*.$(MJV) /c/GFraMe/lib
	cp -fP $(BINDIR)/$(TARGET)*.$(SO) /c/GFraMe/lib
	# Copy the headers
	cp -rf ./include/GFraMe/* /c/GFraMe/include/GFrame

install_x: release
	# Create destiny directories
	mkdir -p $(LIBPATH)
	mkdir -p $(HEADERPATH)
	# Copy every shared lib (normal and debug)
	cp -f $(BINDIR)/$(TARGET)*.$(MNV) $(LIBPATH)
	# -P = don't follow sym-link
	cp -fP $(BINDIR)/$(TARGET)*.$(MJV) $(LIBPATH)
	cp -fP $(BINDIR)/$(TARGET)*.$(SO) $(LIBPATH)
	# Copy the static lib
	cp -f $(BINDIR)/$(TARGET).a $(LIBPATH)
	# Copy the headers
	cp -rf ./include/GFraMe/* $(HEADERPATH)
	# Make the lib be automatically found
	echo "$(LIBPATH)" > /etc/ld.so.conf.d/gframe.conf
	# Update the paths
	ldconfig
#==============================================================================

#==============================================================================
# Rule for uninstalling the library
#==============================================================================
ifeq ($(OS), Win)
uninstall: uninstall_win
else
uninstall: uninstall_x
endif

uninstall:
	# Remove the libraries
	rm -f /c/GFraMe/lib/$(TARGET)_dbg.$(MNV)
	rm -f /c/GFraMe/lib/$(TARGET)_dbg.$(MJV)
	rm -f /c/GFraMe/lib/$(TARGET)_dbg.$(SO)
	rm -f /c/GFraMe/lib/$(TARGET).$(MNV)
	rm -f /c/GFraMe/lib/$(TARGET).$(MJV)
	rm -f /c/GFraMe/lib/$(TARGET).$(SO)
	# Remove the headers
	rm -rf /c/GFraMe/include/*
	# Remove its directories
	rmdir /c/GFraMe/lib/
	rmdir /c/GFraMe/include/
	rmdir /c/GFraMe/

uninstall_x:
	# Remove the libraries
	rm -f $(LIBPATH)/$(TARGET)_dbg.$(MNV)
	rm -f $(LIBPATH)/$(TARGET)_dbg.$(MJV)
	rm -f $(LIBPATH)/$(TARGET)_dbg.$(SO)
	rm -f $(LIBPATH)/$(TARGET).$(MNV)
	rm -f $(LIBPATH)/$(TARGET).$(MJV)
	rm -f $(LIBPATH)/$(TARGET).$(SO)
	rm -f $(LIBPATH)/$(TARGET).a
	# Remove the headers
	rm -rf $(HEADERPATH)/*
	# Remove its directories
	rmdir $(LIBPATH)
	rmdir $(HEADERPATH)
	# Remove the lib from the default path
	rm /etc/ld.so.conf.d/gframe.conf
	# Update the paths
	ldconfig
#==============================================================================

#==============================================================================
# Rule for actually building the static library
#==============================================================================
$(BINDIR)/$(TARGET).a: $(OBJS)
	rm -f $(BINDIR)/$(TARGET).a
	$(AR) -cvq $(BINDIR)/$(TARGET).a $(OBJS)
#==============================================================================

#==============================================================================
# Rule for actually building the shared library
#==============================================================================
# Win
$(BINDIR)/$(TARGET).dll: $(OBJS)
	$(CC) -shared -Wl,-soname,$(TARGET).dll -Wl,-export-all-symbols \
	    $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS)

# Linux
$(BINDIR)/$(TARGET).so: $(BINDIR)/$(TARGET).$(MJV)
	rm -f $@
	cd $(BINDIR); ln -f -s $(TARGET).$(MJV) $(TARGET).so

ifneq ($(SO), $(MJV))
$(BINDIR)/$(TARGET).$(MJV): $(BINDIR)/$(TARGET).$(MNV)
	rm -f $@
	cd $(BINDIR); ln -f -s $(TARGET).$(MNV) $(TARGET).$(MJV)
endif

ifneq ($(SO), $(MNV))
$(BINDIR)/$(TARGET).$(MNV): $(OBJS)
	$(CC) -shared -Wl,-soname,$(TARGET).$(MJV) -Wl,-export-dynamic \
	    $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS)
endif

# Mac OS X
$(BINDIR)/$(TARGET).dylib: $(OBJS)
	$(CC) -dynamiclib $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS)
#==============================================================================

#==============================================================================
# Rule for compiling any .c in its object
#==============================================================================
$(OBJDIR)/%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<
#==============================================================================

#==============================================================================
# Build a emscript (LLVM) binary, to be used when compiling for HTML5
#==============================================================================
$(BINDIR)/$(TARGET).bc: MAKEDIRS $(OBJS)
	$(CC) -o $@ $(CFLAGS) $(OBJS)
#==============================================================================

#==============================================================================
# Rule for creating every directory
#==============================================================================
MAKEDIRS: | $(OBJDIR)
#==============================================================================

#==============================================================================
# Rule for compiling every test (must be suffixed by _tst)
# There's also a small cheat for ignoring some warnings caused by macros
#==============================================================================
tst/gframe_lots_of_particles_tst$(BIN_EXT): tst/gframe_lots_of_particles_tst.c
	$(CC) $(CFLAGS) -Wno-parentheses -o $@ $< $(BINDIR)/$(TARGET).a $(LDFLAGS) \
					-lm

tst/gframe_print_bmp_bytes_tst$(BIN_EXT): tst/gframe_print_bmp_bytes_tst.c
	$(CC) $(CFLAGS) -Wno-parentheses -o $@ $< $(BINDIR)/$(TARGET).a $(LDFLAGS) \
					-lm
#==============================================================================

#==============================================================================
# Rule for compiling every test (must be suffixed by _tst)
#==============================================================================
%_tst$(BIN_EXT): %_tst.c
	$(CC) $(CFLAGS) -o $@ $< $(BINDIR)/$(TARGET).a $(LDFLAGS)
#==============================================================================

#==============================================================================
# Build everything as fast as possible (and using as many cores/threads as
# possible)
#==============================================================================
fast: MAKEDIRS
	make -j $(CORES) static shared
#==============================================================================

#==============================================================================
# Build everything as fast as possible (and using as many cores/threads as
# possible)
#==============================================================================
fast_all: MAKEDIRS
	make -j $(CORES) static shared && make -j $(CORES)
#==============================================================================

#==============================================================================
# Rule for actually creating every directory
#==============================================================================
$(OBJDIR):
	mkdir -p $(OBJDIR)
	mkdir -p $(OBJDIR)/core
	mkdir -p $(OBJDIR)/core/common
	mkdir -p $(OBJDIR)/core/emscript-sdl2
	mkdir -p $(OBJDIR)/core/event/
	mkdir -p $(OBJDIR)/core/event/desktop
	mkdir -p $(OBJDIR)/core/loadAsync
	mkdir -p $(OBJDIR)/core/noip
	mkdir -p $(OBJDIR)/core/sdl2
	mkdir -p $(OBJDIR)/core/video/sdl2
	mkdir -p $(OBJDIR)/core/video/sw_sdl2
	mkdir -p $(OBJDIR)/core/video/opengl3
	mkdir -p $(OBJDIR)/tst
	mkdir -p $(OBJDIR)/util
	mkdir -p $(BINDIR)
	mkdir -p $(BINDIR)
#==============================================================================

#==============================================================================
# Removes all built objects (use emscript_clean to clear the emscript stuff)
#==============================================================================
clean:
	rm -f $(OBJS)
	rm -f $(TEST_BIN)
	rm -f $(BINDIR)/$(TARGET)*.$(MJV)
	rm -f $(BINDIR)/$(TARGET)*.$(MNV)
	rm -f $(BINDIR)/$(TARGET)*.$(SO)
	rm -f $(BINDIR)/$(TARGET)*
#==============================================================================

#==============================================================================
# Remove all built objects and target directories
#==============================================================================
distclean: clean
	rmdir $(OBJDIR)/core/video/sdl2
	rmdir $(OBJDIR)/core/video/sw_sdl2
	rmdir $(OBJDIR)/core/video/opengl3
	rmdir $(OBJDIR)/core/sdl2
	rmdir $(OBJDIR)/core/noip
	rmdir $(OBJDIR)/core/loadAsync
	rmdir $(OBJDIR)/core/event/desktop
	rmdir $(OBJDIR)/core/event/
	rmdir $(OBJDIR)/core/emscript-sdl2
	rmdir $(OBJDIR)/core/common
	rmdir $(OBJDIR)/core
	rmdir $(OBJDIR)/tst
	rmdir $(OBJDIR)/util
	rmdir $(OBJDIR)
	rmdir $(BINDIR)
#==============================================================================

