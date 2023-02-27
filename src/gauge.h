#pragma once

#include <SDL.h>

void draw_headup(SDL_Renderer * &renderer);
void draw_speedometer(SDL_Renderer * &renderer, int rpm);
void draw_needle(SDL_Renderer * &renderer, int rpm);
void draw_sweep(SDL_Renderer * &renderer, int rpm);
void draw_base(SDL_Renderer * &renderer);
void load_assets(SDL_Renderer * &renderer);