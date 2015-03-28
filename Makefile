
CC = gcc
.SUFFIXES=.c .o

#==============================================================================
# Define compilation target
#==============================================================================
  TARGET := libGFraMe
  MAJOR_VERSION := 1
  MINOR_VERSION := 0
  REV_VERSION := 0
#==============================================================================

#==============================================================================
# Define every object required by compilation
#==============================================================================
  OBJS =                                  \
          $(OBJDIR)/gframe_accumulator.o  \
          $(OBJDIR)/gframe_animation.o    \
          $(OBJDIR)/gframe_assets.o       \
          $(OBJDIR)/gframe_object.o       \
          $(OBJDIR)/gframe_screen.o       \
          $(OBJDIR)/gframe_sprite.o       \
          $(OBJDIR)/gframe_spriteset.o    \
          $(OBJDIR)/gframe_texture.o      \
          $(OBJDIR)/gframe_timer.o        \
          $(OBJDIR)/gframe_util.o         \
          $(OBJDIR)/gframe_tilemap.o      \
          $(OBJDIR)/gframe_audio.o        \
          $(OBJDIR)/gframe_audio_player.o \
          $(OBJDIR)/gframe_messagebox.o   \
    	  $(OBJDIR)/gframe_save.o         \
          $(OBJDIR)/gframe_hitbox.o       \
	      $(OBJDIR)/gframe_tween.o        \
          $(OBJDIR)/gframe_pointer.o      \
          $(OBJDIR)/gframe_keys.o         \
          $(OBJDIR)/gframe_controller.o   \
	      $(OBJDIR)/gframe.o              \
          $(OBJDIR)/gframe_log.o          \
    	  $(WDATADIR)/chunk.o             \
          $(WDATADIR)/fmt.o               \
          $(WDATADIR)/wavtodata.o
# Add the objects used with OpenGL
  ifeq ($(USE_OPENGL), yes)
    OBJS +=                               \
            $(OBJDIR)/gframe_opengl.o     \
            $(OBJDIR)/opengl/opengl_wrapper.o 
  endif
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
    CFLAGS := $(CFLAGS) -O1
  endif
# Set flags required by OS
  ifeq ($(OS), Win)
    CFLAGS := $(CFLAGS) -I"/d/windows/mingw/include"
  else
    CFLAGS := $(CFLAGS) -fPIC
  endif
# Add OpenGL flags
  ifeq ($(USE_OPENGL), yes)
    CFLAGS := $(CFLAGS) -DGFRAME_OPENGL
  endif
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
  else
    LFLAGS := $(LFLAGS) -lm
  endif
# Add SDL2 lib
  LFLAGS := $(LFLAGS) -lSDL2
# Add OpenGL lib
  ifeq ($(USE_OPENGL), yes)
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
all: static shared tests
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
tests: MAKEDIRS static $(BINDIR)/test_controller $(BINDIR)/test_collision \
       $(BINDIR)/test_animation
#==============================================================================

#==============================================================================
# Rule for installing the library
#==============================================================================
install: static shared
	# Create destiny directories
	mkdir -p $(LIBPATH)/GFraMe
	mkdir -p $(HEADERPATH)/GFraMe
	# Copy the shared lib
	cp -f $(BINDIR)/$(TARGET).$(MJV) $(LIBPATH)/GFraMe
	cp -f $(BINDIR)/$(TARGET).$(MNV) $(LIBPATH)/GFraMe
	cp -f $(BINDIR)/$(TARGET).$(SO) $(LIBPATH)/GFraMe
	# Copy the static lib
	cp -f $(BINDIR)/$(TARGET).a $(LIBPATH)/GFraMe
	# Copy the headers
	cp -rf ./include/GFraMe/* $(HEADERPATH)/GFraMe
	# Make the lib be automatically found
	echo "$(LIBPATH)/GFraMe" > /etc/ld.so.conf.d/gframe.conf
#==============================================================================

#==============================================================================
# Rule for uninstalling the library
#==============================================================================
uninstall:
	# Remove the libraries
	rm -f $(LIBPATH)/GFraMe/$(TARGET).$(MJV)
	rm -f $(LIBPATH)/GFraMe/$(TARGET).$(MNV)
	rm -f $(LIBPATH)/GFraMe/$(TARGET).$(SO)
	rm -f $(LIBPATH)/GFraMe/$(TARGET).a
	# Remove the headers
	rm -rf $(HEADERPATH)/GFraMe/*
	# Remove its directories
	rmdir $(LIBPATH)/GFraMe
	rmdir $(HEADERPATH)/GFraMe
	# Remove the lib from the default path
	rm /etc/ld.so.conf.d/gframe.conf
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
	rm -f $(BINDIR)/$(TARGET).$(MNV) $(TARGET).$(SO)
	gcc -shared -Wl,-soname,$(TARGET).$(MJV) -Wl,-export-dynamic \
	    $(CFLAGS) -o $(BINDIR)/$(TARGET).$(MNV) $(OBJS) $(LFLAGS)
	ldconfig -n $(BINDIR)
	cd $(BINDIR); ln -s $(TARGET).$(MJV) $(TARGET).$(SO)
endif
#==============================================================================

#==============================================================================
# Rule for compiling any .c in its object
#==============================================================================
$(OBJDIR)/%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<
#==============================================================================

#==============================================================================
# Rule for creating every directory
#==============================================================================
MAKEDIRS: | $(OBJDIR) $(WDATADIR) $(BINDIR)
#==============================================================================

#==============================================================================
#  
#==============================================================================
$(BINDIR)/test_controller: $(OBJDIR)/gframe_test_controller.o
	gcc $(CFLAGS) -DGFRAME_DEBUG -O0 -g -o $(BINDIR)/test_controller $(OBJDIR)/gframe_test_controller.o $(BINDIR)/$(TARGET).a $(LFLAGS)
#==============================================================================

#==============================================================================
#  
#==============================================================================
$(BINDIR)/test_collision: $(OBJDIR)/gframe_test_collision.o
	gcc $(CFLAGS) -DGFRAME_DEBUG -O0 -g -o $(BINDIR)/test_collision $(OBJDIR)/gframe_test_collision.o $(BINDIR)/$(TARGET).a $(LFLAGS)
#==============================================================================

#==============================================================================
#  
#==============================================================================
$(BINDIR)/test_animation: $(OBJDIR)/gframe_test_animation.o
	gcc $(CFLAGS) -DGFRAME_DEBUG -O0 -g -o $(BINDIR)/test_animation $(OBJDIR)/gframe_test_animation.o $(BINDIR)/$(TARGET).a $(LFLAGS)
#==============================================================================

#==============================================================================
# Rule for actually creating every directory
#==============================================================================
$(OBJDIR):
	mkdir -p $(OBJDIR)
	mkdir -p $(OBJDIR)/opengl
	mkdir -p $(WDATADIR)
	mkdir -p $(BINDIR)
#==============================================================================

.PHONY: clean
clean:
	rm -f $(OBJS) $(BINDIR)/$(TARGET)* $(BINDIR)/test_*

