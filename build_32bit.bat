@echo off
setlocal

REM Change this to your SDL3 development package folder.
REM It should contain include\SDL3\SDL.h and lib\libSDL3.dll.a.
set "SDL_DIR=C:\SDL3"

REM If g++ is not on PATH, replace g++ below with its full path.
g++ -m32 -std=c++17 -O2 -Wall -Wextra src\main.cpp -o PokemonAshHorizon.exe -I"%SDL_DIR%\include" -L"%SDL_DIR%\lib" -lSDL3

if errorlevel 1 (
    echo.
    echo Build failed. Check the compiler, SDL_DIR, and SDL3 x86 library.
    pause
    exit /b 1
)

if exist "%SDL_DIR%\bin\SDL3.dll" copy /Y "%SDL_DIR%\bin\SDL3.dll" .
echo.
echo Build complete: PokemonAshHorizon.exe
pause
