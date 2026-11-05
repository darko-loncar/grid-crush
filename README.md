# Gridcrush

Gridcrush is a work-in-progress falling-block puzzle game written in C with SDL3. It includes a familiar seven-piece mode; new features and improvements are planned.

## Features

- Ghost piece and a three-piece preview queue
- Wall-kick-assisted rotation
- Soft drop, hard drop, pause, and new-game controls
- Animated line clears and a generated piece-lock sound
- Nintendo-style line-clear scoring and gravity progression


## Controls

| Key | Action |
| --- | --- |
| Left / Right | Move the active piece |
| Up | Rotate clockwise |
| Down | Soft drop while held |
| Space | Hard drop |
| P | Pause or resume |
| Escape | Quit |

Use the right-side **New Game** and **Quit** buttons as alternatives to keyboard controls.

## Build and run

The CMake configuration is intended to support both Linux and Windows when a C compiler, CMake, SDL3, and SDL3_ttf are installed. The project has been built and tested on Linux; a Windows build has not yet been verified.

### Make

Install a C compiler, Make, pkg-config, SDL3 development files, and SDL3_ttf development files. Then, from the project directory:

```sh
make
make test
./gridcrush
```

### CMake

Install SDL3 and SDL3_ttf with CMake package configuration files available to `find_package`, then run:

```sh
cmake -S . -B build
cmake --build build --config Release
ctest --test-dir build --build-config Release --output-on-failure
```

The build copies the required `assets/` directory next to the game executable. The Make build expects to be launched from the project directory so it can find `assets/Orbitron.ttf`.

## Project layout

- `main.c`, `ui.c`: game loop, input, and interface
- `tetroid.c`, `tetroid.h`: piece generation, movement rules, scoring, and level progression
- `matrix.c`, `matrix.h`: generic matrix operations
- `draw.c`, `draw.h`: SDL rendering helpers
- `sound.c`, `sound.h`: generated piece-lock sound
- `tests/`: matrix and game-logic tests
- `assets/`: bundled font used by the interface

## Notices

The bundled Orbitron font is third-party material. Verify its source and include its required license and attribution before redistributing it.

Gridcrush is an independent project and is not affiliated with or endorsed by The Tetris Company.
