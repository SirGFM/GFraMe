
CC = gcc
.SUFFIXES=.c .o

TARGET = libGFraMe

ifndef $(ARCH)
    ARCH=$(shell uname -m)
    ifeq ($(ARCH), x86_64)
        ARCH=x64
    else
        ARCH=x86
    endif
endif
ifndef $(OS)
    OS=$(shell uname)
    ifeq ($(OS), MINGW32_NT-6.1)
        OS=Win
    endif
endif

CFLAGS = -Wall -I"./include/" -DHAVE_OPENGL
LFLAGS = 
ifeq ($(OS), Win)
    ifeq ($(ARCH), x64)
        LFLAGS += -L"/d/windows/mingw/lib/"
    else
        LFLAGS += -L"/d/windows/mingw/mingw32/lib/"
    endif
    LFLAGS += -lmingw32
    CFLAGS += -I"/d/windows/mingw/include"
else
    LFLAGS += -lm
    CFLAGS += -fPIC
endif
LFLAGS += -lSDL2main -lSDL2

ifeq ($(ARCH), x64)
    CFLAGS += -m64
else
    CFLAGS += -m32
endif

ifneq ($(RELEASE), yes)
    CFLAGS += -DGFRAME_DEBUG -O0 -g
else
    #CFLAGS += -Winline -O1
endif

VPATH = src/
SRCDIR = src
BOBJDIR = obj
OBJDIR = $(BOBJDIR)/$(OS)
WDATADIR = $(OBJDIR)/wavtodata
BINDIR = bin/$(OS)

OBJS = $(OBJDIR)/gframe_accumulator.o $(OBJDIR)/gframe_animation.o \
       $(OBJDIR)/gframe_assets.o $(OBJDIR)/gframe_object.o \
       $(OBJDIR)/gframe_screen.o $(OBJDIR)/gframe_sprite.o \
       $(OBJDIR)/gframe_spriteset.o $(OBJDIR)/gframe_texture.o \
       $(OBJDIR)/gframe_timer.o $(OBJDIR)/gframe_util.o \
       $(OBJDIR)/gframe_tilemap.o $(OBJDIR)/gframe_audio.o \
       $(OBJDIR)/gframe_audio_player.o $(OBJDIR)/gframe_messagebox.o \
	   $(OBJDIR)/gframe_save.o $(OBJDIR)/gframe_hitbox.o \
	   $(OBJDIR)/gframe_tween.o $(OBJDIR)/gframe_pointer.o \
	   $(OBJDIR)/gframe.o $(OBJDIR)/gframe_log.o \
	   $(WDATADIR)/chunk.o $(WDATADIR)/fmt.o $(WDATADIR)/wavtodata.o

ifeq ($(USE_OPENGL), yes)
    CFLAGS += -DGFRAME_OPENGL
    
    OBJS += $(OBJDIR)/gframe_opengl.o
endif

all: static

static: MAKEDIRS $(BINDIR)/$(TARGET).a

$(BINDIR)/$(TARGET).a: $(OBJS)
	rm -f $(BINDIR)/$(TARGET).a
	ar -cvq $(BINDIR)/$(TARGET).a $(OBJS)

$(OBJDIR)/%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

MAKEDIRS: | $(OBJDIR) $(WDATADIR) $(BINDIR)

$(OBJDIR):
	mkdir -p $(OBJDIR)
	mkdir -p $(WDATADIR)
	mkdir -p $(BINDIR)

.PHONY: clean mostlyclean
clean:
	rm -f $(OBJECTS) $(BINDIR)/$(TARGET)*
	rm -rf $(OBJDIR)
	rm -rf $(BINDIR)

