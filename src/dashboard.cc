#include <cstdio>
#include <cstdlib>
#include "config.h"
#include "rasterizer.h"
#include "gauge.h"
#include "graphics.h"
#include "model/linear.h"

using namespace std;

bool revving = false;

void process_key_fsm(SDL_KeyboardEvent &e) {
    static int status = 0;

    switch(status) {
        case 0:
            if(e.keysym.sym == SDLK_SPACE && e.type == SDL_KEYDOWN) {
                status = 1;
                revving = true;
            }
            break;
        case 1:
            if(e.keysym.sym == SDLK_SPACE && e.type == SDL_KEYUP) {
                status = 0;
                revving = false;
            }
            break;
        default:
            status = 0;
    }
}

bool event_loop() {
    SDL_Event e;
    bool quit = false;

    if(SDL_PollEvent(&e)) {
        switch(e.type) {
            case SDL_QUIT:
                quit = true;
                break;
            case SDL_KEYDOWN:
            case SDL_KEYUP:
                process_key_fsm((SDL_KeyboardEvent &)e);
                break;
        }
    }

    return quit;
}

int main() {
    LinearModel model(power);

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

    SDL_RenderPresent(renderer);

    /* process event loop */
    while(!event_loop()) {
        if(revving) model.rev_up();
        else        model.rev_down();

        SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xff);
        SDL_RenderClear(renderer);

        draw_base(renderer);
        draw_sweep(renderer, model.get_revs());
        draw_headup(renderer);
        draw_needle(renderer, model.get_revs());
        draw_speedometer(renderer, model.get_revs());

        SDL_RenderPresent(renderer);
        SDL_Delay(16);  //60 fps
    }


    /* cleanup */
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
