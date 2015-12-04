
#==============================================================================
# Select which compiler to use (either gcc or emcc)
#==============================================================================
  ifneq (,$(findstring emscript, $(MAKECMDGOALS)))
    CC := emcc
    RELEASE := yes
    EXPORT_GIF := no
    BACKEND := emscript

    #USE_GLES2_VIDEO := yes
    USE_SDL2_VIDEO := yes
  else
    CC := gcc

    ifneq ($(NO_GL), yes)
      USE_GL3_VIDEO := yes
    endif
    USE_SDL2_VIDEO := yes

# By default, the FPS counter is enabled
    ifneq ($(FPS_COUNTER), no)
      FPS_COUNTER := yes
    endif
  endif
#==============================================================================

#==============================================================================
# Clear the suffixes' default rule, since there's an explicit one
#==============================================================================
.SUFFIXES:
#==============================================================================

#==============================================================================
# Define all targets that doesn't match its generated file
#==============================================================================
.PHONY: emscript fast fast_all release install clean emscript_clean distclean
#==============================================================================

#==============================================================================
# Define compilation target
#==============================================================================
  TARGET := libGFraMe
  MAJOR_VERSION := 2
  MINOR_VERSION := 0
  REV_VERSION := 0
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
  OBJS =                                  \
          $(OBJDIR)/gframe.o              \
          $(OBJDIR)/gfmAccumulator.o      \
          $(OBJDIR)/gfmAnimation.o        \
          $(OBJDIR)/gfmAudio_mml.o        \
          $(OBJDIR)/gfmAudio_vorbis.o     \
          $(OBJDIR)/gfmAudio_wave.o       \
          $(OBJDIR)/gfmCamera.o           \
          $(OBJDIR)/gfmError.o            \
          $(OBJDIR)/gfmFPSCounter.o       \
          $(OBJDIR)/gfmGroup.o            \
          $(OBJDIR)/gfmGroupHelpers.o     \
          $(OBJDIR)/gfmKeyNode.o          \
          $(OBJDIR)/gfmInput.o            \
          $(OBJDIR)/gfmLog.o              \
          $(OBJDIR)/gfmObject.o           \
          $(OBJDIR)/gfmParser.o           \
          $(OBJDIR)/gfmParserCommon.o     \
          $(OBJDIR)/gfmQuadtree.o         \
          $(OBJDIR)/gfmSave.o             \
          $(OBJDIR)/gfmSprite.o           \
          $(OBJDIR)/gfmSpriteset.o        \
          $(OBJDIR)/gfmString.o           \
          $(OBJDIR)/gfmText.o             \
          $(OBJDIR)/gfmTileAnimation.o    \
          $(OBJDIR)/gfmTilemap.o          \
          $(OBJDIR)/gfmTileType.o         \
          $(OBJDIR)/gfmTrie.o             \
          $(OBJDIR)/gfmUtils.o            \
          $(OBJDIR)/gfmVirtualKey.o       
# Add objects based on the current backend
  ifeq ($(USE_GL3_VIDEO), yes)
    include src/core/video/opengl3/Makefile
  endif
  ifeq ($(USE_SDL2_VIDEO), yes)
    include src/core/video/sdl2/Makefile
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
  OS := $(shell uname)
  ifeq ($(OS), MINGW32_NT-6.1)
    OS := Win
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
  CFLAGS := -Wall -I"./include/" -I"./src/include" -DHAVE_OPENGL
# Add architecture flag
  ARCH := $(shell uname -m)
  ifeq ($(OS), emscript)
    CFLAGS := $(CFLAGS) -I"$(EMSCRIPTEN)/system/include/" -m32
  else
    ifeq ($(ARCH), x86_64)
      CFLAGS := $(CFLAGS) -m64
    else
      CFLAGS := $(CFLAGS) -m32
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
  ifeq ($(OS), Win)
    CFLAGS := $(CFLAGS) -I"/d/windows/mingw/include" -I"/c/c_synth/include/"
  else
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
#==============================================================================

#==============================================================================
# Define LFLAGS (linker flags)
#==============================================================================
  LFLAGS :=
# Add libs and paths required by an especific OS
  ifeq ($(OS), Win)
    ifeq ($(ARCH), x64)
      LFLAGS := $(LFLAGS) -L"/d/windows/mingw/lib" -L"/c/c_synth/lib/"
    else
      LFLAGS := $(LFLAGS) -L"/d/windows/mingw/mingw32/lib" -L"/c/c_synth/lib/"
    endif
    LFLAGS := $(LFLAGS) -lmingw32 -lSDL2main
  else
    LFLAGS := $(LFLAGS) -L/usr/lib/c_synth/
  endif
# Add SDL2 lib
  LFLAGS := $(LFLAGS) -lSDL2
# Add the MML synthesizer
  LFLAGS := $(LFLAGS) -lCSynth
# Add OpenGL lib
 ifeq ($(USE_GL3_VIDEO), yes)
   ifeq ($(OS), Win)
     LFLAGS := $(LFLAGS) -lopengl32
   else
     LFLAGS := $(LFLAGS) -lGL
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
 ifeq ($(OS), Win)
    ifeq ($(ARCH), x64)
      LIBPATH := /d/windows/mingw/lib
    else
      LIBPATH := /d/windows/mingw/mingw32/lib
    endif
    HEADERPATH := /d/windows/mingw/include
 else
   LIBPATH := /usr/lib
   HEADERPATH := /usr/include
 endif
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
   SO := so
   MJV := $(SO).$(MAJOR_VERSION)
   MNV := $(SO).$(MAJOR_VERSION).$(MINOR_VERSION).$(REV_VERSION)
 endif
#==============================================================================

#==============================================================================
# Get the number of cores for fun stuff
#==============================================================================
  ifeq ($(OS), Win)
   CORES := 1
  else
   CORES := $$(($(shell nproc) * 2))
  endif
#==============================================================================

#==============================================================================
# Define default compilation rule
#==============================================================================
all: static tests
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
shared: MAKEDIRS $(BINDIR)/$(TARGET).$(MNV)
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
  install: release
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
else
  install: release
	# Create destiny directories
	mkdir -p $(LIBPATH)/GFraMe
	mkdir -p $(HEADERPATH)/GFraMe
	# Copy every shared lib (normal and debug)
	cp -f $(BINDIR)/$(TARGET)*.$(MNV) $(LIBPATH)/GFraMe
	# -P = don't follow sym-link
	cp -fP $(BINDIR)/$(TARGET)*.$(MJV) $(LIBPATH)/GFraMe
	cp -fP $(BINDIR)/$(TARGET)*.$(SO) $(LIBPATH)/GFraMe
	# Copy the static lib
	cp -f $(BINDIR)/$(TARGET).a $(LIBPATH)/GFraMe
	# Copy the headers
	cp -rf ./include/GFraMe/* $(HEADERPATH)/GFraMe
	# Make the lib be automatically found
	echo "$(LIBPATH)/GFraMe" > /etc/ld.so.conf.d/gframe.conf
	# Update the paths
	ldconfig
endif
#==============================================================================

#==============================================================================
# Rule for uninstalling the library
#==============================================================================
ifeq ($(OS), Win)
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
else
  uninstall:
	# Remove the libraries
	rm -f $(LIBPATH)/GFraMe/$(TARGET)_dbg.$(MNV)
	rm -f $(LIBPATH)/GFraMe/$(TARGET)_dbg.$(MJV)
	rm -f $(LIBPATH)/GFraMe/$(TARGET)_dbg.$(SO)
	rm -f $(LIBPATH)/GFraMe/$(TARGET).$(MNV)
	rm -f $(LIBPATH)/GFraMe/$(TARGET).$(MJV)
	rm -f $(LIBPATH)/GFraMe/$(TARGET).$(SO)
	rm -f $(LIBPATH)/GFraMe/$(TARGET).a
	# Remove the headers
	rm -rf $(HEADERPATH)/GFraMe/*
	# Remove its directories
	rmdir $(LIBPATH)/GFraMe
	rmdir $(HEADERPATH)/GFraMe
	# Remove the lib from the default path
	rm /etc/ld.so.conf.d/gframe.conf
	# Update the paths
	ldconfig
endif
#==============================================================================

#==============================================================================
# Rule for actually building the static library
#==============================================================================
$(BINDIR)/$(TARGET).a: $(OBJS)
	rm -f $(BINDIR)/$(TARGET).a
	ar -cvq $(BINDIR)/$(TARGET).a $(OBJS)
#==============================================================================

#==============================================================================
# Rule for actually building the shared library
#==============================================================================
ifeq ($(OS), Win)
  $(BINDIR)/$(TARGET).$(MNV): $(OBJS)
	rm -f $(BINDIR)/$(TARGET).$(MNV)
	$(CC) -shared -Wl,-soname,$(TARGET).$(MJV) -Wl,-export-all-symbols \
	    $(CFLAGS) -o $(BINDIR)/$(TARGET).$(MNV) $(OBJS) $(LFLAGS)
else
  $(BINDIR)/$(TARGET).$(MNV): $(OBJS)
	rm -f $(BINDIR)/$(TARGET).$(MNV) $(BINDIR)/$(TARGET).$(SO)
	$(CC) -shared -Wl,-soname,$(TARGET).$(MJV) -Wl,-export-dynamic \
	    $(CFLAGS) -o $(BINDIR)/$(TARGET).$(MNV) $(OBJS) $(LFLAGS)
	cd $(BINDIR); ln -f -s $(TARGET).$(MNV) $(TARGET).$(MJV)
	cd $(BINDIR); ln -f -s $(TARGET).$(MJV) $(TARGET).$(SO)
endif
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
	$(CC) $(CFLAGS) -Wno-parentheses -o $@ $< $(BINDIR)/$(TARGET).a $(LFLAGS) \
					-lm
#==============================================================================

#==============================================================================
# Rule for compiling every test (must be suffixed by _tst)
#==============================================================================
%_tst$(BIN_EXT): %_tst.c
	$(CC) $(CFLAGS) -o $@ $< $(BINDIR)/$(TARGET).a $(LFLAGS)
#==============================================================================

#==============================================================================
# Build everything as fast as possible (and using as many cores/threads as
# possible)
#==============================================================================
fast:
	make -j $(CORES) static shared
#==============================================================================

#==============================================================================
# Build everything as fast as possible (and using as many cores/threads as
# possible)
#==============================================================================
fast_all:
	make -j $(CORES) static shared && make -j $(CORES)
#==============================================================================

#==============================================================================
# Rule for actually creating every directory
#==============================================================================
$(OBJDIR):
	mkdir -p $(OBJDIR)
	mkdir -p $(OBJDIR)/tst
	mkdir -p $(OBJDIR)/core
	mkdir -p $(OBJDIR)/core/common
	mkdir -p $(OBJDIR)/core/emscript-sdl2
	mkdir -p $(OBJDIR)/core/noip
	mkdir -p $(OBJDIR)/core/sdl2
	mkdir -p $(OBJDIR)/core/video/sdl2
	mkdir -p $(OBJDIR)/core/video/opengl3
	mkdir -p $(BINDIR)
	mkdir -p $(BINDIR)/tst
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
	rmdir $(OBJDIR)/core/video/opengl3
	rmdir $(OBJDIR)/core/sdl2
	rmdir $(OBJDIR)/core/noip
	rmdir $(OBJDIR)/core/emscript-sdl2
	rmdir $(OBJDIR)/core/common
	rmdir $(OBJDIR)/core
	rmdir $(OBJDIR)/tst
	rmdir $(OBJDIR)
	rmdir $(BINDIR)/tst
	rmdir $(BINDIR)
#==============================================================================

