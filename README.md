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
  16.666...ms), frames will vary in a consistent matter (e.g., 17ms/17ms/16ms).
</details>

<details>
  <summary>"Action-oriented" input</summary>
  Instead of delegating to the lib's user to check whether an action was
  performed in any of the assigned keys (e.g., assigning jump to both up key,
  'w' and spacebar), "virtual keys" may be created and have inputs (keyboard
  keys, mouse buttons and gamepad buttons) assigned to them.

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
  *NOTE 1:* This feature requires OpenGL 3.1 or better

  If OpenGL is set as the video backend, draws with the same texture are batched
  until either a different texture is used or 8192 sprites are batched. This
  limit was selected to try to limit the use of VRAM for batching to 576KB.

  *NOTE 2:* The limit of 8192 sprites may be lower. It depends on
  GL_MAX_TEXTURE_BUFFER_SIZE being at least 49152 (2 texels per sprite with 3 
  separated buffers).
</details>

<details>
  <summary>Assets management</summary>
  There's no need to know the full path to an asset, nor its type. The library
  only needs the asset name (e.g., "texture.bmp", "song.mml", "sfx.wav", ...)
  and it handles the rest. File extensions doesn't even need to be correct. The
  framework looks into a file's identifier (usually within its first bytes) to
  retrieve the file's type.

  *NOTE:* Assets are required to be inside a 'assets' directory (on the same
  level as the game's binary file). 
</details>

<details>
  <summary>MML support</summary>
  Other than supporting WAVE files for audio, the framework can also handle MML
  files. Those can be correctly versioned, requires less disk space and are
  great for pixel-art games.

  *NOTE 1:* This features makes use of
  [libCSynth](https://github.com/SirGFM/c_synth)
  *NOTE 2:* Unloading a specific MML is still not supported. This shouldn't be a
  problem on desktop environment, though...
</details>

<details>
  <summary>Non-recursive collision detection</summary>
  The library uses a quadtree for collision detection. Instead of traversing it
  recursively (and requiring a callback for handling the collision/overlap), it
  does so iteratively and stops execution whenever a collision is detected (akin
  to 'yield' statement in some languages). The user may than handle it however
  wanted and, then, continue traversing the quadtree.
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
  TODO: write details...
</details>

<details>
  <summary>Save file</summary>
  TODO: write details...
</details>

<details>
  <summary>Camera system</summary>
  TODO: write details...
</details>

<details>
  <summary>File parser</summary>
  TODO: write details...
</details>

<details>
  <summary>Tiled plugin</summary>
  TODO: write details...
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
$ sudo make install
```


## Installing

Simply run make:

```
$ sudo make install
```

If you would like to install GFraMe without OpenGL, set NO_GL to yes:

```
$ sudo make install NO_GL=yes
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

