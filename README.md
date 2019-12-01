# GameEngine pocket
A game engine for creating GameBoy (Color) inspired games.

Currently only Windows is supported

## Version status
The project is currently versioned at 0.8.0, which means the first 80% of the work is done. Cleaning it all up for a 1.0.0 release will be the second 80%.

# Build instructions
Open a `cmd.exe` window with 64bit Microsoft Visual Studio 2017 tools available in the path. (Open the "x64 Native Tools Command Prompt for VS 2017" from the start menu or execute the relevant `vcvarsall.bat x64` in a cmd shell)

There are `build.bat` scripts to build the different parts of the project. 
- When run for the first time, you need to run them with `build.bat full` to create necessary directories. 
- After that a simple `build.bat` will create a debug build. 
- `build.bat release` will create a release build. 
- `build.bat clean` will remove the directories created in the full build.

To build everything at once, use the `buildall.bat` which takes the same arguments and simply calls the `build.bat` files for all the sub-projects.

To create a release zip, call `release.sh` with the name for the finished file (without .zip) as an argument. (Yes, this is a bash script. I use Windows Subsystem for Linux.)
`release.sh` depends on the zip program.

# How to use
Coming Soon™

# Missing major features
- Controller support
- Key rebinding
- Proper handling of non-60hz refresh rates. For now just use exclusive fullscreen which should set the display to 60hz.
