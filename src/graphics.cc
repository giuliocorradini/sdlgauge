#include "graphics.h"
#include "config.h"
#include <SDL.h>
#include <SDL_image.h>
#include <SDL2_gfxPrimitives.h>

bool setup(SDL_Window * &window, SDL_Surface * &surface, SDL_Renderer * &renderer) {

    //Init SDL subsystem for VIDEO only (this is a flag)
    if(SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("Could not initialise video: %s\n", SDL_GetError());
        return false;
    }

    //Init SDL Image module
    if(!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {  //IMG_Init returns the init'd flags
        printf("Could not initialise SDL Image");
        return false;
    }

    //Create a new window
    window = SDL_CreateWindow("Dashboard",
                              SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              WINDOW_WIDTH, WINDOW_HEIGHT,
                              SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI);
    
    if(window == nullptr) {
        printf("Window could not be created: %s\n", SDL_GetError());
        return false;
    }

    //Initialise renderer
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if(!renderer) {
        printf("Can't initialise rendererer. %s\n", SDL_GetError());
        exit(-1);
    }

    //Configure Hi-DPI mode
    int render_width, render_height;
    SDL_GetRendererOutputSize(renderer, &render_width, &render_height);
    if(render_width != WINDOW_WIDTH) {
        float width_scale = render_width / WINDOW_WIDTH;
        float height_scale = render_height / WINDOW_HEIGHT;

        SDL_RenderSetScale(renderer, width_scale, height_scale);
    }

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
    SDL_RenderFillRect(renderer, nullptr);

    SDL_RenderPresent(renderer);
    SDL_RenderClear(renderer);

    return true;
}