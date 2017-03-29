# GFraMe

GFraMe, which stands for 'Game FraMework', originated from my desire to build my
own game framework and, in doing so, learn and understand how one should work.

The library has been tested quite a few times. It's was used on my last 3
[Ludum Dare](http://ludumdare.com/compo), all of which are on my github. It
targets Windows and Linux (it has been tested only on Debian-based distros...
namely, Ubuntu), but there are plans to also support HTML5, through
[emscripten](http://kripken.github.io/emscripten-site/), and Android, through
[Android NDK](http://developer.android.com/tools/sdk/ndk/index.html).

Although it's written in C and the user never register callbacks within the lib,
the framework has some ideas related to OOP. This can be noticed when using the
module for collision test (gfmQuadtree), which can handle tilemaps, objects and
sprites, and returns the two overlaping objects, from which the actual "object"
pointer may be retrieved.

Head over to the wiki to find out more about it!


## Features overview

<details>
  <summary>Fixed time step</summary>
  Time elapsed between frames is counted in milliseconds and guaranteed to be
  constant between frames. When that isn't possible (e.g., 60FPS would require
  16.666...ms), frames will vary in a consistent manner (e.g., 17ms/17ms/16ms).
</details>

<details>
  <summary>"Action-oriented" input</summary>
  To handle input, "virtual keys" must be created to represent action. Keys,
  gamepad buttons e mouse buttons can be assigned to those "v-keys". This should
  make customizable controls easier to implement.

  *NOTE:* Inputs can't be bound to more than one action!
</details>

<details>
  <summary>"Frame-oriented" input</summary>
  If a button was pressed on the previous frame and it was re-pressed before a
  issuing a new frame (that is, there was no frame with the button released),
  the framework corretly detect it as being just pressed again.
</details>

<details>
  <summary>Auto batched rendering</summary>
  If OpenGL is set as the video backend, draws with the same texture are batched
  until either a different texture is used or 8192 sprites are batched.
  
  *NOTE 1:* This feature requires OpenGL 3.1 or better

  *NOTE 2:* The limit was selected to try to limit the use of VRAM for batching
  to 576KB. More VRAM is actually needed for storing textures and others
  objects.

  *NOTE 3:* The limit of 8192 sprites may be lower. It depends on
  GL_MAX_TEXTURE_BUFFER_SIZE being at least 49152 (2 texels per sprite with 3 
  separated buffers).
</details>

<details>
  <summary>Assets management</summary>
  Assets are accessed only through their filename. There's no need to know the
  complete path to those files. Whoever, this causes two implications:

  1. There *must* be an 'assets/' directory on the same level as the
  executable.
  2. All assets must be located on that 'assets/' folder. One can separate
  assets into sub-directories, but this would required the sub-directory to
  be listed on the filename (e.g., "sub_dir/texture.bmp").

  On load, known file identifiers are looked up on the asset, to check if the
  file is supported or not. One such example is the first 2 "BM" bytes on
  bitmap files.
</details>

<details>
  <summary>MML support</summary>
  Audio may be loaded from MML files. Those can be correctly versioned, requires
  less disk space and are great for pixel-art games.

  *NOTE 1:* This features makes use of

  [libCSynth](https://github.com/SirGFM/c_synth)
  *NOTE 2:* Unloading a specific MML is still not supported. This shouldn't be a
  problem on desktop environment, though...
</details>

<details>
  <summary>Camera system</summary>
  Single camera used to convert world-space to screen space. There are already
  functionalities to follow objects.
</details>

<details>
  <summary>Non-recursive collision detection</summary>
  Collision detection is implemented through a quadtree structure. Although
  those are usually recursive and make use of callbacks to handle overlaps,
  it was implemented iteratively.

  Whenever a collision test detects two overlapping objects, the quadtree halts
  execution (akin to 'yield' statements in some languages) and returns the
  overlapping objects. Collision may be later resumed.
</details>

<details>
  <summary>Sorted particle rendering</summary>
  Particles may be sorted by their vertical position and/or time alive. Only
  particles that are within the camera are sorted.

  *NOTE:* Sorting is done in a single thread and is quite CPU-intesive. Since a
  binary tree is used, multiple threads (each with its own list of elements to
  be sorted) could be used for the first few sub-trees.
</details>

<details>
  <summary>Particle collision</summary>
  Particles may be collided through the quadtree. It's possible to only check
  every other particle (or 1 every 3), so it's a little lighter on the CPU.
</details>

<details>
  <summary>Gamepad support</summary>
  When gamepads are connected, they are sorted into 'ports' (similar to how
  gamepads used to be connected to physically numbered ports on consoles).

  Whenever a gamepad is disconnected, its 'port' is closed. A new controller
  will be connected on this "released" 'port', keeping the previous order
  unchanged.

  To assign a gamepad button to an action, the button and the desired port
  must be passed. Axis may be used either as simple triggers (setting the
  deadzone to trigger it) or by getting its current value (as floats in the
  range [-1, 1]).
</details>

<details>
  <summary>Save file</summary>
  Simple files that associates IDs to values. Can be used to store a game's
  state and/or highscore.
</details>

<details>
  <summary>Tiled plugin</summary>
  Since [Tiled](https://github.com/bjorn/tiled) can be extended with custom
  plugins, it was selected as the default tilemap/level editor to be used with
  the framework.

  This plugin can export a level's tilemap, it's types for specific tiles (e.g.,
  floor, spike, checkpoint etc.) and objects.
</details>

<details>
  <summary>File parser</summary>
  Parser for a JSON-like objects. It's quite useful for defining enemies,
  objects, events etc.
</details>


## Dependencies

GFraMe depends on [SDL2](https://www.libsdl.org/) and
[libCSynth](https://github.com/SirGFM/c_synth).

On Debian-based distros (only tested on Ubuntu!), SDL2 may be installed through
the package manager. Simply run:

```
$ sudo apt-get install libsdl2-dev
```

libCSynth must be downloaded and compiled:

```
$ git clone git@github.com:SirGFM/c_synth.git
$ cd c_synth
$ sudo make install DEBUG=yes
$ sudo make install RELEASE=yes
```


## Installing

Simply run make:

```
sudo make install DEBUG=yes
sudo make install RELEASE=yes
```

If you would like to install GFraMe without OpenGL, set NO_GL to yes:

```
sudo make install DEBUG=yes NO_GL=yes
sudo make install RELEASE=yes NO_GL=yes
```


## Tests/Demos

There are a few demo applications on 'tst/'. The preferred way to compile them
is:

```
$ make clean
$ make fast_all
```

For some dumb reason, all demos are statically linked. Cleaning everything
beforehand makes sure that 'fast_all' will re-compile the library in debug mode.
Also, the rules 'fast' and 'fast_all' will use as spawn many jobs to build
everything (hopefully) faster.


## Cross-compiling for Windows

For some reason, when cross-compiling for Windows from Linux, the compiler
complains about a `glActiveTexture` redefinition. If you try to remove it from
`src/core/video/opengl3/gfmVideo_opengl3_glFuncs.*`, the linker will complain
that `glActiveTexture` isn't defined.

The hacky solution I found to overcome that was to remove that definition from
SDL2's header.

For SDL2 2.0.5, that function's definition can be found on `SDL2/SDL_opengl.h`,
on line 1871. Enclose it between `#if !defined(_WIN32) && !defined(_WIN64)` and
`#endif`.

[MXE](https://github.com/mxe/mxe) is highly advised for cross compiling for
Windows from Linux!

