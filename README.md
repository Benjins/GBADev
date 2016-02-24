GBADev: The repository where I make GBA games.
===========================
Or at least I try to...
---------------------------

Build Status: [![Build Status](https://travis-ci.org/Benjins/GBADev.svg?branch=master)](https://travis-ci.org/Benjins/GBADev)

These demos are built using [devKitPro](http://devkitpro.org/), using a small bit of their libgba library to initialize the games.  Other than that, the code is freestanding.

A lot of the code is heavily inspired by the Tonc tutorials on [Coranac](http://www.coranac.com/tonc/text/toc.htm), which are created by [Jasper Vijn](http://www.coranac.com).  However, this code does not link against the Tonc libraries, or any of Vijn's code. 

Some art assets are from outside sources.  These have their license info in their directories.  All of my code and "art" is licensed under the MIT license, more info in LICENSE.txt.

The code is structured so that a directory represents a game project, and contains all the code and art assets for that project (excpet for tileMapper, which contains the code for the tile mapping program).  Because these directories are designed to be as self-contained as possible, there's quite a bit of duplicate code between them.

Each directory contains a `main.c`, which is what is built.  All other source and header files must be `#include`'d by `main.c`, since it is the only translation unit.
 
A look inside build-run.bat will show what steps are used to compile the code.  However, these files are specialised for my own use, as they build the rom and thebn load up an emulator.

The code will compile as is.  However, any changes to art assets will require running pack-assets.bat [name of project directory].  The code is cross-platform, however the exact build and run scripts may need to be changed.  The general philosophy of the asset packers (sprites, backgrounds, sounds, and midi) is this: 

 * A text file enumerates the assets, named the same across projects (topDown/assets.txt, sdkTest/assets.txt, etc.)
 * Each line of the file indicates an asset, which is converted from art source to game data
 * The game data is output as a header with the same name as the text file (assets.h, sounds.h, midi.h, etc).  This is included in the project's main.c

~~The tileMapper program is Windows-specific, and will not run on Mac/Linux without heavy modification.~~

The tileMapper and animer programs used to be Windows-specific.  As of now, tileMapper can run on Linux through the X11 libraries.  However, the open file dialog has not been ported.

TODO:
 * Finish up DirectSound work
 * Make tileMapper support multiple backgrounds
 * Make level editor for topDown
 * Some kind of debugging method
 * Animation tool:
   - ~~Normalize/canonicalize animations, prevent negative key durations~~
   - ~~Button to add new animations~~
   - Change duration of last keyframe
   - Way to rename them?
   - Save button in gui, not just 'W'
 * ~~Export header from animations file~~
 * Keep track of sprites exported in asset.h, prevent dupes
 * Remove Animations and Timers