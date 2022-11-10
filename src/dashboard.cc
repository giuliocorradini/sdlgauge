#include <cstdio>
#include <cstdlib>
#include <array>
#include <string>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL2_gfxPrimitives.h>
#include "rasterizer.h"

using namespace std;

const int WINDOW_HEIGHT = 500;
const int WINDOW_WIDTH = 500;

float needle_angle(float target_rpm) {
    const float base_angle = -30;
    const float max_rpm = 6000;

    return target_rpm / max_rpm * 240 + base_angle;
}

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
                              SDL_WINDOW_SHOWN);
    
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

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
    SDL_RenderFillRect(renderer, nullptr);

    SDL_RenderPresent(renderer);
    SDL_RenderClear(renderer);

    return true;
}

bool event_loop() {
    SDL_Event e;
    bool quit = false;

    if(SDL_PollEvent(&e)) {
        switch(e.type) {
            case SDL_QUIT:
                quit = true;
                break;
        }
    }

    return quit;
}


namespace assets {          //this order follows the design order bottom-up
    SDL_Texture *bezel;
    SDL_Texture *inner_ring;
    SDL_Texture *overlay;
    SDL_Texture *needle;
    SDL_Texture *outer_sweep;
    SDL_Texture *inner_sweep;
    array<SDL_Texture *, 10> speedometer_digits;
}

/*
 *  Clears render context by filling with RGBA 0
 *  black with alpha channel = 0
 */
void clear_render_context(SDL_Renderer * &renderer) {
    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
    SDL_RenderClear(renderer);
}

/*
 *  Loads PNG asset into optimized surface for a given window surface
 */
SDL_Texture *load_optimized_asset(SDL_Renderer * &renderer, const char *path) {
    SDL_Texture *texture = IMG_LoadTexture(renderer, path);
    assert(texture);

    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);

    return texture;
}

/*
 *  Populates "assets" namespace objects with content from the assets directory.
 *  Calls load_optimized_asset for each member of the namespace. 
 */
void load_assets(SDL_Renderer * &renderer) {
    //Load digits textures
    array<string, 10> digits_filename = {
        "assets/speedometer/0.png",
        "assets/speedometer/1.png",
        "assets/speedometer/2.png",
        "assets/speedometer/3.png",
        "assets/speedometer/4.png",
        "assets/speedometer/5.png",
        "assets/speedometer/6.png",
        "assets/speedometer/7.png",
        "assets/speedometer/8.png",
        "assets/speedometer/9.png"
    };
    for(int i=0; i<10; i++) {
        assets::speedometer_digits.at(i) = load_optimized_asset(renderer, digits_filename.at(i).c_str());
    }

    assets::bezel = load_optimized_asset(renderer, "assets/bezel.png");
    assets::overlay = load_optimized_asset(renderer, "assets/overlay_digits.png");
    assets::inner_ring = load_optimized_asset(renderer, "assets/inner_ring.png");

    assets::needle = load_optimized_asset(renderer, "assets/needle.png");

    assets::outer_sweep = load_optimized_asset(renderer, "assets/outer_sweep.png");
    assets::inner_sweep = load_optimized_asset(renderer, "assets/inner_sweep.png");
}

void draw_base(SDL_Renderer * &renderer) {
    static SDL_Rect bezel_pos = {.x=0, .y=0, .w=500, .h=376};
    SDL_RenderCopy(renderer, assets::bezel, NULL, &bezel_pos);

    static SDL_Rect inner_ring_position = {.x=36, .y=39, .w=428, .h=318};
    SDL_RenderCopy(renderer, assets::inner_ring, NULL, &inner_ring_position);

    static SDL_Rect overlay_position = {.x=3, .y=0, .w=494, .h=384};
    SDL_RenderCopy(renderer, assets::overlay, NULL, &overlay_position);
};

SDL_Texture *sweep;
SDL_Texture *sweep_mask;
void draw_sweep(SDL_Renderer * &renderer, int rpm) {
    /* Prepare arc mask */
    SDL_SetRenderTarget(renderer, sweep_mask);
    clear_render_context(renderer);
    filledPieRGBA(renderer, 250, 250, 250, 150, needle_angle(rpm) - 180, 0xff, 0xff, 0xff, 0xff);   //outer arc

    //Render onto sweep texture
    SDL_SetRenderTarget(renderer, sweep);
    clear_render_context(renderer);

    //Copy full arcs
    static SDL_FRect outer_sweep_pos = {.x=0, .y=0, .w=500, .h=376};
    SDL_RenderCopyF(renderer, assets::outer_sweep, NULL, &outer_sweep_pos);
    static SDL_FRect inner_sweep_pos = {.x=37, .y=41, .w=426.4, .h=316.6};
    SDL_RenderCopyF(renderer, assets::inner_sweep, NULL, &inner_sweep_pos);

    //Apply mask
    SDL_RenderCopy(renderer, sweep_mask, NULL, NULL);

    //Reset drawing target to window context
    SDL_SetRenderTarget(renderer, nullptr);

    //Render texture onto window
    SDL_RenderCopy(renderer, sweep, NULL, NULL);
};

void draw_needle(SDL_Renderer * &renderer, int rpm) {
    static SDL_FPoint needle_rot_center = {.x = 254, .y = 136};
    static SDL_FRect needle_target_position = {.x=250-254, .y=250-136, .w=270, .h=273};
    SDL_RenderCopyExF(renderer, assets::needle, NULL, &needle_target_position, needle_angle(rpm), &needle_rot_center, SDL_FLIP_NONE);
};

int main() {

    /* SDL object reference */
    //Renderer (GPU abstraction)
    SDL_Renderer *renderer = nullptr;

    //Window handle
    SDL_Window *window = NULL;

    //Surface handle
    SDL_Surface *surface = NULL;


    /* Setup */
    if(!setup(window, surface, renderer)) {
        exit(-1);
    }

    load_assets(renderer);

    sweep = SDL_CreateTexture(renderer, SDL_GetWindowPixelFormat(window), SDL_TEXTUREACCESS_TARGET, 500, 500);
    assert(sweep);
    SDL_SetTextureBlendMode(sweep, SDL_BLENDMODE_BLEND);

    sweep_mask = SDL_CreateTexture(renderer, SDL_GetWindowPixelFormat(window), SDL_TEXTUREACCESS_TARGET, 500, 500);
    assert(sweep_mask != nullptr);
    SDL_SetTextureBlendMode(sweep_mask, SDL_BLENDMODE_MOD);

    SDL_RenderPresent(renderer);

    int nang = -30;

    /* process event loop */
    while(!event_loop()) {
        SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xff);
        SDL_RenderClear(renderer);
        draw_base(renderer);

        draw_sweep(renderer, nang);
        draw_needle(renderer, nang);
        nang += 2;
        if(nang >= 210)
            nang = -30;

        //prepare pie mask
        SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);
        circle::plot_crown_sector(renderer, 250, 250, 250, 216, -30, 210);  //outer
        circle::plot_crown_sector(renderer, 250, 250, 213, 161, -30, 210);  //inner

        SDL_RenderPresent(renderer);
        SDL_Delay(16);  //60 fps
    }


    /* cleanup */
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}