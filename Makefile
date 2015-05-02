
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
          $(OBJDIR)/gframe.o              \
          $(OBJDIR)/gfmSpriteset.o        \
          $(OBJDIR)/gfmString.o           \
          $(OBJDIR)/gfmUtils.o            
# Add objects based on the current backend
  ifndef ($(BACKEND))
    include src/core/sdl2/Makefile
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
    CFLAGS := $(CFLAGS) -O2
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
  else
    LFLAGS := $(LFLAGS) -lm
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
 TEST_BIN := $(addprefix $(BINDIR)/, $(TEST_SRC:%.c=%))
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
tests: MAKEDIRS static $(TEST_BIN)
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
	cd $(BINDIR); ln -f -s $(TARGET).$(MJV) $(TARGET).$(SO)
endif
#==============================================================================

#==============================================================================
# Rule for creating an optimized object file (must be tested!)
#==============================================================================
optmized:
	$(CC) -shared -Wl,-soname,$(TARGET)_opt.$(MJV) -Wl,-export-dynamic \
	    $(CFLAGS) -o $(BINDIR)/$(TARGET)_opt.$(MNV) $(ALL_SRC) $(LFLAGS)
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
MAKEDIRS: | $(OBJDIR) $(BINDIR)
#==============================================================================

#==============================================================================
# Rule for compiling every test (must be suffixed by _tst)
#==============================================================================
$(BINDIR)/%_tst: $(OBJDIR)/%_tst.o
	$(CC) $(CFLAGS) -o $@ $< $(BINDIR)/$(TARGET).a $(LFLAGS) 
#==============================================================================

#==============================================================================
# Rule for actually creating every directory
#==============================================================================
$(OBJDIR):
	mkdir -p $(OBJDIR)
	mkdir -p $(OBJDIR)/tst
	mkdir -p $(OBJDIR)/core
	mkdir -p $(OBJDIR)/core/sdl2
	mkdir -p $(BINDIR)
	mkdir -p $(BINDIR)/tst
#==============================================================================

.PHONY: clean mostlyclean
clean:
	rm -f $(OBJS)
	rm -f $(TEST_BIN)
	rm -f $(BINDIR)/$(TARGET).$(MJV)
	rm -f $(BINDIR)/$(TARGET).$(MNV)
	rm -f $(BINDIR)/$(TARGET).$(SO)
	rm -f $(BINDIR)/$(TARGET)*

