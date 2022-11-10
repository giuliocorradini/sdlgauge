#include "rasterizer.h"
#include <cmath>
#include "SDL.h"

namespace circle {
    /*
     *  radius error: deviation from the real radius of the given point
     *  defined as |x_i^2 + y_i^2 - r^2|
     */
    int re(int x, int y, int r) {
        return static_cast<int>(
            abs(float(x*x + y*y - r*r))
        );
    }

    void plot(SDL_Renderer * &render, int x_0, int y_0, int r) {
        int x = r, y = 0;

        //calcolo per il primo ottante
        while(x >= y) {
            //plot riflesso sugli 8 ottanti
            SDL_RenderDrawPoint(render, x_0 + x, y_0 + y);  //primo quadrante
            SDL_RenderDrawPoint(render, x_0 + y, y_0 + x);

            SDL_RenderDrawPoint(render, x_0 - x, y_0 + y);  //secondo quadrante
            SDL_RenderDrawPoint(render, x_0 - y, y_0 + x);

            SDL_RenderDrawPoint(render, x_0 - x, y_0 - y);  //terzo quadrante
            SDL_RenderDrawPoint(render, x_0 - y, y_0 - x);

            SDL_RenderDrawPoint(render, x_0 + x, y_0 - y);  //quarto quadrante
            SDL_RenderDrawPoint(render, x_0 + y, y_0 - x);
            //devo scegliere la prossima x
            if((2*re(x, y, r) + (2*y + 1)) + (1 - 2*x) > 0)
                x--;

            y++;    //aumenta sempre
        }

        //ci sono dei buchini
        if(x == y) {
            SDL_RenderDrawPoint(render, x_0 + x, y_0 + y);
            SDL_RenderDrawPoint(render, x_0 + x, y_0 - y);
            SDL_RenderDrawPoint(render, x_0 - x, y_0 + y);
            SDL_RenderDrawPoint(render, x_0 - x, y_0 - y);
        }

    }

    template<typename T> bool comprised(T l, T x, T u) {
        return (l <= x && x <= u);
    }

    float theta(float x, float y, float r) {
        float angle = acos(x/r);
        return y<0 ? - angle : angle;
    }

    void plot_arc(SDL_Renderer * &render, int x_0, int y_0, int r, float a, float b) {
        //convert angles from sexagesimal to radians and invert (because picture_y = - WCS_y)
        a *= -2 * M_PI / 360;
        b *= -2 * M_PI / 360;

        int x = r, y = 0;

        //calcolo per il primo ottante
        while(x >= y) {
            float t;

            if(comprised(b, t=theta(x,y, r), a))
                SDL_RenderDrawPoint(render, x_0 + x, y_0 + y);  //quarto quadrante cartesiano (primo WCS)
            if(comprised(b, t=theta(y,x, r), a))
                SDL_RenderDrawPoint(render, x_0 + y, y_0 + x);

            t=theta(-x,y, r);
            if(comprised(b, t, a) || comprised(b, float(t-2*M_PI), a))
                SDL_RenderDrawPoint(render, x_0 - x, y_0 + y);  //terzo quadrante
            t=theta(-y,x, r);
            if(comprised(b, t, a) || comprised(b, float(t-2*M_PI), a))
                SDL_RenderDrawPoint(render, x_0 - y, y_0 + x);

            if(comprised(b, t=theta(-x,-y, r), a))
                SDL_RenderDrawPoint(render, x_0 - x, y_0 - y);  //secondo quadrante
            if(comprised(b, t=theta(-y,-x, r), a))
                SDL_RenderDrawPoint(render, x_0 - y, y_0 - x);

            if(comprised(b, t=theta(x,-y, r), a))
                SDL_RenderDrawPoint(render, x_0 + x, y_0 - y);  //primo quadrante
            if(comprised(b, t=theta(y,-x, r), a))
                SDL_RenderDrawPoint(render, x_0 + y, y_0 - x);
            

            //devo scegliere la prossima x
            if((2*re(x, y, r) + (2*y + 1)) + (1 - 2*x) > 0)
                x--;

            y++;    //aumenta sempre
        }

        //ci sono dei buchini
        if(x == y) {
            SDL_RenderDrawPoint(render, x_0 + x, y_0 + y);
            SDL_RenderDrawPoint(render, x_0 + x, y_0 - y);
            SDL_RenderDrawPoint(render, x_0 - x, y_0 + y);
            SDL_RenderDrawPoint(render, x_0 - x, y_0 - y);
        }
    }

    void get_arc_limits(const int x_0, const int y_0, const int r, const int a, const int b, int &x_a, int &y_a, int &x_b, int &y_b) {
        float a_rad = -a * 2 * M_PI / 360;
        float b_rad = -b * 2 * M_PI / 360;

        x_a = r * cos(a_rad) + x_0;
        x_b = r * cos(b_rad) + x_0;
        y_a = r * sin(a_rad) + y_0;
        y_b = r * sin(b_rad) + y_0;
    }

    void plot_crown_sector(SDL_Renderer * &render, int x_0, int y_0, int r1, int r2, float a, float b) {
        int xa1, ya1, xb1, yb1, xa2, ya2, xb2, yb2;

        //outer arc
        circle::plot_arc(render, x_0, y_0, r1, a, b);
        circle::get_arc_limits(x_0, y_0, r1, a, b, xa1, ya1, xb1, yb1);

        //inner arc
        circle::plot_arc(render, x_0, y_0, r2, a, b);
        circle::get_arc_limits(x_0, y_0, r2, a, b, xa2, ya2, xb2, yb2);

        //connection lines
        SDL_RenderDrawLine(render, xa1, ya1, xa2, ya2);
        SDL_RenderDrawLine(render, xb1, yb1, xb2, yb2);

        //fill
    }

}

namespace util {
    void fill(SDL_Renderer * &render, int x, int y, uint32_t stop_color) {
        /*
        Flood-fill (nodo, colore_prima, colore_nuovo):
        1. Se il colore di nodo è diverso da colore_prima, termina.
        2. Imposta il colore di nodo a colore_nuovo.
        3. Esegui Flood-fill (nodo ad ovest di nodo, colore_prima, colore_nuovo).
           Esegui Flood-fill (nodo a nord di nodo, colore_prima, colore_nuovo).
           Esegui Flood-fill (nodo ad est di nodo, colore_prima, colore_nuovo).
           Esegui Flood-fill (nodo a sud di nodo, colore_prima, colore_nuovo).
        4. Termina
        */

        if(x > 500 || y > 500)
            return;

        SDL_Rect p = {.x = x, .y = y, .w = 1, .h = 1};
        uint32_t pixel_value;

        SDL_RenderReadPixels(render, &p, 0, &pixel_value, 4);

        if(pixel_value == stop_color)
            return;

        SDL_RenderDrawPoint(render, x, y);
        fill(render, x, y+1, stop_color);
        fill(render, x+1, y, stop_color);
        fill(render, x, y-1, stop_color);
        fill(render, x-1, y, stop_color);
    }
}