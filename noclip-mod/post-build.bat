rd %1 /S /Q
xcopy %2 %1 /E /I /Y
copy %3 %4

:: Ilemni: Hacky solution to copy the mod to the game's mods folder
set STEAMAPPS=C:\Program Files (x86)\Steam\steamapps
set MODS_FOLDER=%STEAMAPPS%\common\MIO\mods
set MOD_NAME=noclip-mod

if exist "%MODS_FOLDER%\" (
    xcopy %1 "%MODS_FOLDER%\%MOD_NAME%\" /E /I /Y
)
