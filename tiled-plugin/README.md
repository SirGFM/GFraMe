# GFraMe's tiled plugin

Tiled plugin that exports a single tilemap that can be parsed by the GFraMe
library

## Requirements

To compile and use this plugin, it's required to have the tiled source code
downloaded/cloned. It can be downloaded from the following repository:
https://github.com/bjorn/tiled .

This plugin was made for the commit 'cef41290b159ec2fff8c31db4e86e929aef95299',
it will be eventually tested with the latest version. For now, to use it, after
cloning the tiled source, run the following command on its base directory:

```
# Go back to the desired version
$ git reset --hard cef41290b159ec2fff8c31db4e86e929aef95299
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

