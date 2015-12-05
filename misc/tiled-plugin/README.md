# GFraMe's tiled plugin

Tiled plugin that exports a single tilemap that can be parsed by the GFraMe
library

## Requirements

To compile and use this plugin, it's required to have the tiled source code
downloaded/cloned. It can be downloaded from the following repository:
https://github.com/bjorn/tiled.

This plugin was tested against the commit
'78c273ceb870fece51473229f865350605992ebc', (the latest version on 2015/11/14).

If This plugin ever gets incompatible with the current tiled version, run the
following command to roll it back to the last known version to work:

```
# Go back to the desired version
$ git reset --hard 78c273ceb870fece51473229f865350605992ebc
# Compile tiled
$ qmake
$ make
```

## Compiling

Go into the tiled-plugin and run

```
$ make TILED_DIR=<directory-where-tiled-was-cloned-into>
```

This will compile and install the plugin on that tiled directory.

