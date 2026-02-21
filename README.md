# mio-noclip-mod
A noclip mod for MIO: Memories in Orbit

## Summary

This mod allows freely moving through the world without regard for gravity, collision, or damage surfaces.
Although damage sources aren't entirely ignored, during noclip. the player's HP is set every tick to either the amount they entered noclip with, or 1, whichever is higher.

## Controls

- All controls use the numkey pad by default. These controls can be rebound.
- `2`: Enter/exit noclip. Anything that sets position is ignored when noclip is disabled. Noclip is automatically disabled while in menu or credits.
- `1`,`3`: Decrease/Increase speed by a factor of 2
- `8`,`4`,`5`,`6`: WASD movement.
- `7`,`9`: Move along Z axis. Only works in some scenarios.
- `0`: Reset Z position to 0.
- `/`: Save a position, `*`: Teleport to that saved position. By default this is the world origin.
- `+`: Print current player position to console
- `DEL`: Reload keybind and location file.
- `Z`: Teleport to previous location in teleport list
- `X`: Teleport to next location in teleport list

The keybind file is located at `<Mio install location>\modconfig\noclip\keybinds.txt`. The keybind values use [Virtual Key Codes](https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes).

Human-readable values can also be used, such as copying from the "Constants" value in the table, or certain shorthand values (such as N0 for Numpad0, * for Multiply)

## Teleporting

This mod also provides a way to teleport to specific coordinates in a list. This list is located at `modconfig\noclip\locations.txt`

Locations can be listed like this
```
-1388.72 1551.13   Nexus Overseer
-1578 1578.36      Mel's Shop
```


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
