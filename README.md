# SDL Gauge Dashboard

SDL-based gauge dashboard.

This project was born to simulate a digital cockpit and get in acquitance with 2D graphics and the SDL library.

## Dependencies

On macOS you need SDL2, SDL2 Image and SDL2 GFX as well as a compiler toolchain and Cmake.

With brew:

```sh
brew install cmake sdl2 sdl2_image sdl2_gfx
```

## Compiling

Like any Cmake-based project, create a new directory named `build` and enter it. Then start Cmake configuration
and start compiling with Make.

```sh
mkdir build
cd build

cmake ..
make
```
