#pragma once

#include <SDL.h>

namespace circle {
    /*
     *  plot circle
     *  render: render context as SDL_Render pointer
     *  (x_0, y_0) is the center of the circle
     *  r: radius
     */
    void plot(SDL_Renderer * &render, int x_0, int y_0, int r);

    /*
     *  plot arc of circumference
     *  render: render context as SDL_Render pointer
     *  (x_0, y_0) center of the arc
     *  r: radius
     *  a, b: angular limits with a<b for counterclockwise arc or a>b for clockwise
     */
    void plot_arc(SDL_Renderer * &render, int x_0, int y_0, int r, float a, float b);
    void get_arc_limits(const int x_0, const int y_0, const int r, const int a, const int b, int &x_a, int &y_a, int &x_b, int &y_b);

    void plot_crown_sector(SDL_Renderer * &render, int x_0, int y_0, int r1, int r2, float a, float b);
}

namespace util {
    //fill hangs in an infinite recursion loop, not available
    //void fill(SDL_Renderer * &render, int x, int y, uint32_t stop_color);
}