![Flappy demo](https://github.com/eisbehr/gep/raw/misc-files/misc/flappy.gif)

# GameEngine pocket
A game engine for creating GameBoy (Color) inspired games.

Currently only Windows is supported

## Version status
The project is currently versioned at 0.8.0, which means the first 80% of the work is done. Cleaning it all up for a 1.0.0 release will be the second 80%.

# How to build
Open a `cmd.exe` window with 64bit Microsoft Visual Studio 2017 tools available in the path. (Open the "x64 Native Tools Command Prompt for VS 2017" from the start menu or execute the relevant `vcvarsall.bat x64` in a cmd shell)

There are `build.bat` scripts to build the different parts of the project. 
- When run for the first time, you need to run them with `build.bat full` to create necessary directories. 
- After that a simple `build.bat` will create a debug build. 
- `build.bat release` will create a release build. 
- `build.bat clean` will remove the directories created in the full build.

To build everything at once, use the `buildall.bat` which takes the same arguments and simply calls the `build.bat` files for all the sub-projects.

To create a release zip, call `release.sh` with the name for the finished file (without .zip) as an argument. (Yes, this is a bash script. I use Windows Subsystem for Linux.)
`release.sh` depends on the zip program.

# How to create a game
Your game will be loaded by the `gep.exe` as a file called `game.dll`.
The dll needs a few exported symbols that the exe will load.

One `update()` function
``` c
DLLEXPORT void update(gep_state *GS)
```
and the following custom strings
``` c
DLLEXPORT const char *ORG_CONF_PATH = "org";
DLLEXPORT const char *GAMENAME_CONF_PATH = "game_conf";

DLLEXPORT const char *GAMENAME = "The Name of the Game 2: Revenge of the Game";
```
The `ORG_CONF_PATH` and `GAMENAME_CONF_PATH` will be used together to determine the path for the config file.

the `Gamename` will be used in the window title.

# How to play a game
Put your `game.dll` next to the `gep.exe` and `SDL2.dll` files. Then launch gep.exe. 

## Controls
|Key|Virtpad|Description
|---|-------|-----------
|ESC|   |Switch between the game and the settings screen
|W|Up|d-pad up
|A|Left|d-pad left
|S|Down|d-pad down
|D|Right|d-pad right
|Q|L1|left bumper
|E|R1|right bumper
|1|L2|left trigger
|3|R2|right trigger
|Arrow Up|A|A face button
|Arrow Down|B|B face button
|Arrow Left|X|X face button
|Arrow Right|Y|Y face button
|Return|Menu|Button to activate a game menu
|Backspace|Option|Button to activate game options
| | |
|F9| |**DEBUG** Open window showing the tile map
|F10| |**DEBUG** Open window showing the window map
|F11| |**DEBUG** Open window showing the tile and sprite memory
# Missing major features
- Controller support
- Key rebinding
- Proper handling of non-60hz refresh rates. For now just use exclusive fullscreen which should set the display to 60hz.
- a save file system of some kind
