
CC = gcc
.SUFFIXES=.c .o

TARGET = bugsquasher

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
LGFRAMEDIR = "../"
ifeq ($(OS), Win)
    ifeq ($(ARCH), x64)
        LFLAGS += -L"/d/windows/mingw/lib/"
    else
        LFLAGS += -L"/d/windows/mingw/mingw32/lib/"
    endif
    LFLAGS += -L"$(LGFRAMEDIR)/bin/Win/" -lmingw32 -lGFraMe
    CFLAGS += -I"/d/windows/mingw/include"
else
    LFLAGS += -L"$(LGFRAMEDIR)/bin/Linux/" -lGFraMe -lm
endif
CFLAGS += -I"$(LGFRAMEDIR)/include/"
LFLAGS += -lSDL2main -lSDL2

ifeq ($(ARCH), x64)
    CFLAGS += -m64
else
    CFLAGS += -m32
endif

ifneq ($(RELEASE), yes)
    CFLAGS += -DDEBUG -O0 -g
else
    CFLAGS += -Winline -O1
endif

VPATH = src/
SRCDIR = src
BOBJDIR = obj
OBJDIR = $(BOBJDIR)/$(OS)
BINDIR = bin/$(OS)

OBJS = $(OBJDIR)/main.o $(OBJDIR)/global.o $(OBJDIR)/playstate.o \
       $(OBJDIR)/enemies.o $(OBJDIR)/player.o $(OBJDIR)/score.o \
	   $(OBJDIR)/multiplier.o $(OBJDIR)/background.o $(OBJDIR)/menustate.o \
	   $(OBJDIR)/button.o $(OBJDIR)/gameover.o

all: $(BINDIR)/$(TARGET)

$(BINDIR)/$(TARGET): MAKEDIRS libGFraMe $(OBJS)
	$(CC) $(CFLAGS) -o $(BINDIR)/$(TARGET) $(OBJS) $(LFLAGS)

$(OBJDIR)/%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

MAKEDIRS: | $(OBJDIR) $(BINDIR)

$(OBJDIR):
	mkdir -p $(OBJDIR)
	mkdir -p $(BINDIR)

libGFraMe:
	make --directory=$(LGFRAMEDIR)

.PHONY: clean mostlyclean
clean:
	rm -f $(OBJECTS) $(BINDIR)/$(TARGET)
	rm -rf $(OBJDIR)
	rm -rf $(BINDIR)

