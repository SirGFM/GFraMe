# GFraMe

GFraMe (Game FraMework) is, as the name implies, a framework for building
games. It's based on SDL2, but it was designed to be easily modified and
to somewhat independent of a base library. It's written in C and works on
Windows and Linux (only tested on Ubuntu 14.04, though). It should be
quite easy to build for other OSs, since it has a single dependency
(for now).

The library can be somewhat divided into two modules: the 'main module'
and the 'core module'. The 'main module' comprises the functionalities
required to build a game (like the "physics engine", the sprites and its
animations sub-system, etc.). The 'core module', on the other hand, is
handles interfacing the game with a "media library" (like SDL2). This way,
if one desired to use Direct3D directly, for example, he would only have
to write a new 'core module', build a new library and link that to the
game. The game's code wouldn't need any modifications at all (well,
theoretically).

## Dependencies

On Linux, the "only" required library is SDL2. On Ubuntu (and Debian,
probably) you can get it from the package manager. Just open a terminal
and run:

```
$ sudo apt-get install libsdl2-dev
```

If your distro doesn't have it on its package manager, you can download it
from https://www.libsdl.org/download-2.0.php. As the writing of this
README, its latest stable version is 2.0.3.

On Windows, download and install MinGW (http://www.mingw.org/). Then, on
its package manager, download the core utilities and MSYS.

Afterward, download, compile and install SDL2
(https://www.libsdl.org/download-2.0.php). You could get the precompiled
dev libraries, but that would be no fun... :)

## Compiling and installing

With all dependencies installing,  open a terminal and simply run:

```
$ make
$ sudo make install
```

On Linux, it will install the libraries (.so and .a) on /usr/lib/GFraMe,
and the header files on /usr/include/GFraMe.

On Windows, it will install everything under C:\GFraMe. The libraries will
be installed on C:\GFraMe\lib and the headers on C:\GFraMe\include\GFraMe.

# Compiling with emscript

In order to compiler with emscript (and use this to build javascript games), one
should prepare its environment before hand.

On Linux, this may be as simples as downloading and installing the emscript SDK
and then running

```
$ source <EMSDK_DIR>/emsdk_set_env.sh
```
