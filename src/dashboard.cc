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
const float max_rpm = 6000;

const int power = 10;

const int MAX_SPEED = 75;

/*
 *  Converts given RPMs to the angle (in degrees) the needle should turn to.
 */
float needle_angle(float target_rpm) {
    const float base_angle = -30;

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

bool revving = false;
int revs = 0;

void rev_up() {
    revs += power;
    if(revs >= 6000)
        revs = 6000;
}

void rev_down() {
    revs -= power;
    if(revs < 0)
        revs = 0;
}

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


namespace assets {          //this order follows the design order bottom-up
    SDL_Texture *base;
    SDL_Texture *needle;
    SDL_Texture *sweep;
    array<SDL_Texture *, 10> speedometer_digits;
    SDL_Texture *speedometer_unit;
    SDL_Texture *headup;
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
    if(!texture) {
        printf("%s\n", SDL_GetError());
    }

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


    assets::base = load_optimized_asset(renderer, "assets/base.png");
    assets::needle = load_optimized_asset(renderer, "assets/needle.png");

    assets::sweep = load_optimized_asset(renderer, "assets/sweep.png");

    assets::speedometer_unit = load_optimized_asset(renderer, "assets/speedometer/KMH.png");

    assets::headup = load_optimized_asset(renderer, "assets/headup.png");
}

void draw_base(SDL_Renderer * &renderer) {
    static SDL_Rect base_pos = {.x=0, .y=0, .w=500, .h=500};
    SDL_RenderCopy(renderer, assets::base, NULL, &base_pos);
};

/*
 *  Creates a new texture in the specified renderer context, of the window dimensions.
 */
SDL_Texture *create_canvas_texture(SDL_Renderer * &renderer) {
    SDL_Texture *t = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_TARGET, WINDOW_WIDTH, WINDOW_HEIGHT);
    assert(t);
    SDL_SetTextureBlendMode(t, SDL_BLENDMODE_BLEND);

    return t;
}

void get_clip_for_quadrant(SDL_Rect &clip, int quadrant) {
    clip = {
        .x = quadrant < 2 ? 0 : WINDOW_WIDTH / 2,
        .y = (quadrant == 0 || quadrant == 3) ? WINDOW_HEIGHT / 2 : 0,
        .w = WINDOW_WIDTH / 2,
        .h =  WINDOW_HEIGHT / 2
    };
}

void copy_clipped_sweep_on_canvas(SDL_Renderer * &renderer, SDL_Texture * &canvas, SDL_Rect &clip) {
    // Set clip sweep as current render target, and clear it
    SDL_SetRenderTarget(renderer, canvas);

    // Copy sweep base into texture
    SDL_RenderSetClipRect(renderer, &clip);
    SDL_RenderCopy(renderer, assets::sweep, NULL, NULL);

    //Reset drawing target to window context
    SDL_SetRenderTarget(renderer, nullptr);
}

void getMaskForQuadrant(int angle, int quadrant, int16_t tx[], int16_t ty[]) {
    static const float radius = WINDOW_HEIGHT / 2;
    float rad_angle = angle * M_PI / 180;
    switch(quadrant) {
        case 0:
            tx[0] = WINDOW_WIDTH/2, ty[0] = WINDOW_HEIGHT/2;
            tx[1] = WINDOW_WIDTH/2, ty[1] = WINDOW_HEIGHT;
            tx[2] = 0, ty[2] = WINDOW_HEIGHT;
            tx[3] = 0, ty[3] = WINDOW_HEIGHT/2 - radius * tan(rad_angle);
            break;
        case 1:
            tx[0] = WINDOW_WIDTH/2, ty[0] = WINDOW_HEIGHT/2;
            tx[1] = 0, ty[1] = WINDOW_HEIGHT/2;
            tx[2] = 0, ty[2] = 0;
            tx[3] = angle < 45 ? 0 : WINDOW_WIDTH/2 - radius / tan(rad_angle), ty[3] = angle < 45 ? WINDOW_HEIGHT/2 - radius * tan(rad_angle) : 0;
            break;
        case 2:
            tx[0] = WINDOW_WIDTH/2, ty[0] = WINDOW_HEIGHT/2;
            tx[1] = WINDOW_WIDTH/2, ty[1] = 0;
            tx[2] = WINDOW_WIDTH, ty[2] = 0;
            tx[3] = angle < 135 ? WINDOW_WIDTH/2 - radius / tan(rad_angle) : WINDOW_WIDTH, ty[3] = angle < 135 ? 0 : WINDOW_HEIGHT/2 + radius * tan(rad_angle);
            break;
        case 3:
            tx[0] = WINDOW_WIDTH/2, ty[0] = WINDOW_HEIGHT/2;
            tx[1] = WINDOW_WIDTH, ty[1] = WINDOW_HEIGHT/2;
            tx[2] = WINDOW_WIDTH, ty[2] = WINDOW_HEIGHT/2 + radius * tan(rad_angle);
            tx[3] = WINDOW_WIDTH/2, ty[3] = WINDOW_HEIGHT/2;
            break;
    }
}

void prepareMask(SDL_Renderer * &renderer, SDL_Texture * &mask, int angle, int quadrant) {
    // Draw the last part of the sweep with masking
    SDL_SetRenderTarget(renderer, mask);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderFillRect(renderer, nullptr);

    //The coordinates already refer to a WINDOW_WIDTH * WINDOW_HEIGHT canvas
    int16_t tx[4], ty[4];
    getMaskForQuadrant(angle, quadrant, tx, ty);
    filledPolygonColor(
        renderer,
        tx,
        ty,
        4,
        0xffffffff
    );
    SDL_SetRenderTarget(renderer, nullptr);
}


void draw_sweep(SDL_Renderer * &renderer, int rpm) {
    int angle = static_cast<int>(needle_angle(rpm));
    int quadrant = (angle + 90) / 90;

    
    // Draw the part of sweep that don't require masking
    SDL_Rect clip;
    for(int i=0; i<quadrant; i++) {
        SDL_Texture *quad_texture = create_canvas_texture(renderer);

        // Copy sweep on canvas
        get_clip_for_quadrant(clip, i);
        copy_clipped_sweep_on_canvas(renderer, quad_texture, clip);

        //  Then copy it on the actual renderer
        SDL_RenderCopy(renderer, quad_texture, NULL, NULL);
    }
    

    static SDL_Texture *clip_sweep = create_canvas_texture(renderer);
    static SDL_Texture *mask = create_canvas_texture(renderer);
    SDL_SetTextureBlendMode(
        mask,
        SDL_ComposeCustomBlendMode(
            SDL_BLENDFACTOR_ZERO,   //src*0
            SDL_BLENDFACTOR_SRC_COLOR,  //dst*src
            SDL_BLENDOPERATION_ADD,  //dst*src+src*0 = dst*src
            SDL_BLENDFACTOR_ZERO,
            SDL_BLENDFACTOR_SRC_ALPHA,
            SDL_BLENDOPERATION_ADD
        )
    );
    //SDL_SetTextureBlendMode(clip_sweep, SDL_BLENDMODE_MOD);

    prepareMask(renderer, mask, angle, quadrant);

    SDL_SetRenderTarget(renderer, clip_sweep);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderFillRect(renderer, nullptr);
    SDL_Rect quadclip;
    get_clip_for_quadrant(quadclip, quadrant);

    // Copy sweep base into texture
    SDL_RenderCopy(renderer, assets::sweep, nullptr, nullptr);
    SDL_RenderCopy(renderer, mask, nullptr, nullptr);

    SDL_SetRenderTarget(renderer, nullptr);

    SDL_RenderCopy(renderer, clip_sweep, NULL, NULL);
};

void draw_needle(SDL_Renderer * &renderer, int rpm) {
    static SDL_FPoint needle_rot_center = {.x = 254, .y = 136};
    static SDL_FRect needle_target_position = {.x=250-254, .y=250-136, .w=270, .h=273};
    SDL_RenderCopyExF(renderer, assets::needle, NULL, &needle_target_position, needle_angle(rpm), &needle_rot_center, SDL_FLIP_NONE);
};

void draw_speedometer(SDL_Renderer * &renderer, int rpm) {
    using namespace assets;

    int speed = rpm / max_rpm * MAX_SPEED;
    
    //Select digits
    SDL_Texture *ten = speedometer_digits.at(speed / 10 % 10);
    int tenx, teny;
    SDL_QueryTexture(ten, nullptr, nullptr, &tenx, &teny);

    SDL_Texture *unit = speedometer_digits.at(speed % 10);
    int unitx, unity;
    SDL_QueryTexture(unit, nullptr, nullptr, &unitx, &unity);

    SDL_FRect tens_target = {
        .x = static_cast<float>(WINDOW_WIDTH / 2 - tenx),
        .y = static_cast<float>((WINDOW_HEIGHT - teny) / 2.),
        .w = static_cast<float>(tenx),
        .h = static_cast<float>(teny)
    };
    SDL_FRect units_target = {
        .x = static_cast<float>((WINDOW_WIDTH - (speed>=10 ? 0 : unitx)) / 2),
        .y = static_cast<float>((WINDOW_HEIGHT - unity) / 2.),
        .w = static_cast<float>(unitx),
        .h = static_cast<float>(unity)
    };

    if(speed >= 10)
        SDL_RenderCopyF(renderer, ten, NULL, &tens_target);
    SDL_RenderCopyF(renderer, unit, NULL, &units_target);

    int sunitx, sunity;
    SDL_QueryTexture(speedometer_unit, nullptr, nullptr, &sunitx, &sunity);
    SDL_FRect speed_unit_target = {
        .x = static_cast<float>((WINDOW_WIDTH - sunitx) / 2),
        .y = static_cast<float>((WINDOW_HEIGHT + teny) / 2 + 10),
        .w = static_cast<float>(sunitx),
        .h = static_cast<float>(sunity)
    };
    SDL_RenderCopyF(renderer, speedometer_unit, NULL, &speed_unit_target);
}

void draw_headup(SDL_Renderer * &renderer) {
    static SDL_Rect base_pos = {.x=0, .y=0, .w=500, .h=500};
    SDL_RenderCopy(renderer, assets::headup, NULL, &base_pos);
}

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

    SDL_RenderPresent(renderer);

    /* process event loop */
    while(!event_loop()) {
        SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xff);
        SDL_RenderClear(renderer);

        draw_base(renderer);

        draw_sweep(renderer, revs);
        draw_headup(renderer);
        draw_needle(renderer, revs);
        if(revving) rev_up();
        else        rev_down();

        draw_speedometer(renderer, revs);

        SDL_RenderPresent(renderer);
        SDL_Delay(16);  //60 fps
    }


    /* cleanup */
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
