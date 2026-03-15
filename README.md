# mio-noclip-mod
A noclip mod for MIO: Memories in Orbit

## Summary

This mod allows freely moving through the world without regard for gravity, collision, or damage surfaces.

## Controls

- All controls use the numkey pad by default. These controls can be rebound.
- `2`: Enter/exit noclip. Anything that sets position is ignored when noclip is disabled. Noclip is automatically disabled while in menu or credits.
- `1`,`3`: Decrease/Increase speed by a factor of 2
- `8`,`4`,`5`,`6`: WASD movement.
- `7`,`9`: Move along Z axis. Only works in some scenarios.
- `0`: Reset Z position to 0.
- `DEL`: Reload keybind and location file.

The keybind file is located at `<Mio install location>\modconfig\noclip\keybinds.txt`. The keybind values use [Virtual Key Codes](https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes).

Human-readable values can also be used, such as copying from the "Constants" value in the table, or certain shorthand values (such as N0 for Numpad0, * for Multiply)

### Running in Game

1. Copy `x64/Release/noclip-mod` folder into your MIO install's `mods` directory
    - This copy step is done automatically with [noclip-mod/post-build.bat](noclip-mod/post-build.bat) on every build. Edit this to use your own `STEAMAPPS` path, if needed.
2. Launch the game
