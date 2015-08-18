
CC = gcc
.SUFFIXES=.c .o

#==============================================================================
# Define compilation target
#==============================================================================
  TARGET := libGFraMe
  MAJOR_VERSION := 1
  MINOR_VERSION := 0
  REV_VERSION := 0
# If the DEBUG flag was set, generate another binary (so it doesn't collide
# with the release one)
  ifeq ($(DEBUG), yes)
    TARGET := $(TARGET)_dbg
  endif
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
  ifndef ($(BACKEND))
    include src/core/sdl2/Makefile
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
  ifeq ($(ARCH), x86_64)
    CFLAGS := $(CFLAGS) -m64
  else
    CFLAGS := $(CFLAGS) -m32
  endif
# Add debug flags
  ifneq ($(RELEASE), yes)
    CFLAGS := $(CFLAGS) -g -O0 -DDEBUG
  else
    ifeq ($(OS), Win)
      CFLAGS := $(CFLAGS) -O0
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
    CFLAGS := $(CFLAGS) -I"/d/windows/mingw/include"
  else
    CFLAGS := $(CFLAGS) -fPIC
  endif
# Add OpenGL flags
 # ifeq ($(USE_OPENGL), yes)
 #   CFLAGS := $(CFLAGS) -DGFRAME_OPENGL
 # endif
#==============================================================================

#==============================================================================
# Define LFLAGS (linker flags)
#==============================================================================
  LFLAGS :=
# Add libs and paths required by an especific OS
  ifeq ($(OS), Win)
    ifeq ($(ARCH), x64)
      LFLAGS := $(LFLAGS) -I"/d/windows/mingw/lib"
    else
      LFLAGS := $(LFLAGS) -I"/d/windows/mingw/mingw32/lib"
    endif
    LFLAGS := $(LFLAGS) -lmingw32 -lSDL2main
  endif
# Add SDL2 lib
  LFLAGS := $(LFLAGS) -lSDL2
# Add OpenGL lib
 # ifeq ($(USE_OPENGL), yes)
 #   ifeq ($(OS), Win)
 #     LFLAGS := $(LFLAGS) -lopengl32
 #   else
 #     LFLAGS := $(LFLAGS) -lGL
 #   endif
 # endif
#==============================================================================

#==============================================================================
# Define where source files can be found and where objects & binary are output
#==============================================================================
 VPATH := src:tst
 TESTDIR := tst
 OBJDIR := obj/$(OS)
 WDATADIR := $(OBJDIR)/wavtodata
 BINDIR := bin/$(OS)
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
 TEST_BIN := $(addprefix $(BINDIR)/, $(TEST_SRC:%.c=%$(BIN_EXT)))
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
	make RELEASE=yes static
	make RELEASE=yes shared
	make RELEASE=yes optmized
	# Remove all debug info from the binaries
	strip $(BINDIR)/$(TARGET).a
	strip $(BINDIR)/$(TARGET).$(MNV)
	strip $(BINDIR)/$(TARGET)_opt.$(MNV)
	# Delete all .o to recompile as debug
	rm -f $(OBJS)
	# Recompile the lib with debug info
	make DEBUG=yes static
	make DEBUG=yes shared
	date
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
	# Copy every shared lib (normal, optmized and debug)
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
	# Copy every shared lib (normal, optmized and debug)
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
	rm -f /c/GFraMe/lib/$(TARGET)_opt.$(MNV) 
	rm -f /c/GFraMe/lib/$(TARGET)_opt.$(MJV) 
	rm -f /c/GFraMe/lib/$(TARGET)_opt.$(SO) 
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
	rm -f $(LIBPATH)/GFraMe/$(TARGET)_opt.$(MNV) 
	rm -f $(LIBPATH)/GFraMe/$(TARGET)_opt.$(MJV) 
	rm -f $(LIBPATH)/GFraMe/$(TARGET)_opt.$(SO) 
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
	gcc -shared -Wl,-soname,$(TARGET).$(MJV) -Wl,-export-all-symbols \
	    $(CFLAGS) -o $(BINDIR)/$(TARGET).$(MNV) $(OBJS) $(LFLAGS)
else
  $(BINDIR)/$(TARGET).$(MNV): $(OBJS)
	rm -f $(BINDIR)/$(TARGET).$(MNV) $(BINDIR)/$(TARGET).$(SO)
	gcc -shared -Wl,-soname,$(TARGET).$(MJV) -Wl,-export-dynamic \
	    $(CFLAGS) -o $(BINDIR)/$(TARGET).$(MNV) $(OBJS) $(LFLAGS)
	cd $(BINDIR); ln -f -s $(TARGET).$(MNV) $(TARGET).$(MJV)
	cd $(BINDIR); ln -f -s $(TARGET).$(MJV) $(TARGET).$(SO)
endif
#==============================================================================

#==============================================================================
# Rule for creating an optimized object file (must be tested!)
#==============================================================================
ifeq ($(OS), Win)
  optmized:
	$(CC) -shared -Wl,-soname,$(TARGET)_opt.$(MJV) -Wl,-export-all-symbols \
	    $(CFLAGS) -o $(BINDIR)/$(TARGET)_opt.$(MNV) $(ALL_SRC) $(LFLAGS)
else
  optmized:
	rm -f $(BINDIR)/$(TARGET)_opt.$(MNV) $(BINDIR)/$(TARGET)_opt.$(SO)
	$(CC) -shared -Wl,-soname,$(TARGET)_opt.$(MJV) -Wl,-export-dynamic \
	    $(CFLAGS) -o $(BINDIR)/$(TARGET)_opt.$(MNV) $(ALL_SRC) $(LFLAGS)
	cd $(BINDIR); ln -f -s $(TARGET)_opt.$(MNV) $(TARGET)_opt.$(MJV)
	cd $(BINDIR); ln -f -s $(TARGET)_opt.$(MJV) $(TARGET)_opt.$(SO)
endif
#==============================================================================

#==============================================================================
# Rule for compiling any .c in its object
#==============================================================================
$(OBJDIR)/%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<
#==============================================================================

#==============================================================================
# Define compilation rule for gfmAudio so it avoids optimizations
#==============================================================================
$(OBJDIR)/core/sdl2/gfmAudio.o: src/core/sdl2/gfmAudio.c
	$(CC) $(CFLAGS) -O0 -o $@ -c $<
#==============================================================================


#==============================================================================
# Rule for creating every directory
#==============================================================================
MAKEDIRS: | $(OBJDIR)
#==============================================================================

#==============================================================================
# Rule for compiling every test (must be suffixed by _tst)
#==============================================================================
$(BINDIR)/%_tst$(BIN_EXT): $(OBJDIR)/%_tst.o
	$(CC) $(CFLAGS) -o $@ $< $(BINDIR)/$(TARGET).a $(LFLAGS) 
#==============================================================================

#==============================================================================
# Rule for actually creating every directory
#==============================================================================
$(OBJDIR):
	mkdir -p $(OBJDIR)
	mkdir -p $(OBJDIR)/tst
	mkdir -p $(OBJDIR)/core
	mkdir -p $(OBJDIR)/core/common
	mkdir -p $(OBJDIR)/core/noip
	mkdir -p $(OBJDIR)/core/sdl2
	mkdir -p $(BINDIR)
	mkdir -p $(BINDIR)/tst
#==============================================================================

.PHONY: clean mostlyclean
clean:
	rm -f $(OBJS)
	rm -f $(TEST_BIN)
	rm -f $(BINDIR)/$(TARGET)*.$(MJV)
	rm -f $(BINDIR)/$(TARGET)*.$(MNV)
	rm -f $(BINDIR)/$(TARGET)*.$(SO)
	rm -f $(BINDIR)/$(TARGET)*

mostlyclean: clean
	rmdir $(OBJDIR)/core/common
	rmdir $(OBJDIR)/core/noip
	rmdir $(OBJDIR)/core/sdl2
	rmdir $(OBJDIR)/core
	rmdir $(OBJDIR)/tst
	rmdir $(OBJDIR)
	rmdir $(BINDIR)/tst
	rmdir $(BINDIR)

