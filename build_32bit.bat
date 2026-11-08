@echo off
setlocal

REM SDL3 and SDL3_image must both be 32-bit MinGW development packages.
REM Each folder should contain include\ and lib\; bin\ should contain its DLL.
set "SDL_DIR=C:\SDL3"
set "SDL_IMAGE_DIR=C:\SDL3_image"

REM If g++ is not on PATH, replace g++ below with its full path.
g++ -m32 -std=c++17 -O2 -Wall -Wextra src\main.cpp -o PokemonAshHorizon.exe ^
  -I"%SDL_DIR%\include" -I"%SDL_IMAGE_DIR%\include" ^
  -L"%SDL_DIR%\lib" -L"%SDL_IMAGE_DIR%\lib" -lSDL3_image -lSDL3

if errorlevel 1 (
    echo.
    echo Build failed. Check the compiler and both SDL_DIR paths.
    pause
    exit /b 1
)

if exist "%SDL_DIR%\bin\SDL3.dll" copy /Y "%SDL_DIR%\bin\SDL3.dll" .
if exist "%SDL_IMAGE_DIR%\bin\SDL3_image.dll" copy /Y "%SDL_IMAGE_DIR%\bin\SDL3_image.dll" .
echo.
echo Build complete: PokemonAshHorizon.exe
echo Keep SDL3.dll and SDL3_image.dll beside the EXE.
pause
