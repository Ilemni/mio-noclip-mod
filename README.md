# mio-noclip-mod
A noclip mod for MIO: Memories in Orbit

## Summary

This mod allows freely moving through the world without regard for gravity, collision, or damage surfaces.
Although damage sources aren't entirely ignored, during noclip. the player's HP is set every tick to either the amount they entered noclip with, or 1, whichever is higher.

## Controls

- All controls use the numkey pad
- `2` to enter/exit noclip. Anything that sets position is ignored when noclip is disabled. Noclip is automatically disabled while in menu or credits.
- `1`,`3` to decrease/increase speed by a factor of 2
- `8`,`4`,`5`,`6` work as WASD.
- `7`,`9` to move along Z axis. Only works in some scenarios.
- `0` to reset Z position to 0.
- `/` to save a position, `*` to teleport to that saved position. By default this is the world origin.
- `+` to print current position to console


## Prerequisites

- CMake 3.20 or higher
- MSVC (Visual Studio 2019 or later)
- vcpkg
- Ninja
  

## Building

### Using CMake Presets (Recommended)
```powershell
# Configure
cmake --preset x64-release

# Build
cmake --build --preset x64-release
```

### Running in Game

1. Copy `build/x64-release/bin/noclip.dll` into your MIO install's `mods` directory
2. Launch the game
