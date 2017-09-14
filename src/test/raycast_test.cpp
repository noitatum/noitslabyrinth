#include <SDL2/SDL.h>
#include <maze.h>
#include <vector>

struct point {
    float x, y;

    point operator-(point b) {
        return {x - b.x, y - b.y};
    }

    point operator/(float a) {
        return {x / a, y / a};
    }
};

point a{0, 0}, b{0, 0};
bool draw_line = false;

SDL_Window* init_sdl() {
    SDL_Init(SDL_INIT_VIDEO);
    return SDL_CreateWindow("TEST", SDL_WINDOWPOS_CENTERED,
                            SDL_WINDOWPOS_CENTERED, 800, 600, 0);
}

void handle_event(SDL_Event& e, SDL_Renderer* r) {
    if (e.type == SDL_MOUSEBUTTONDOWN) {
        point p{(float) e.button.x, (float) e.button.y};
        a = p;
        b = p;
        draw_line = true;
    } else if (e.type == SDL_MOUSEMOTION) {
        point p{(float) e.button.x, (float) e.button.y};
        if (draw_line) {
            b = p;
        }
    } else if (e.type == SDL_MOUSEBUTTONUP) {
        draw_line = false;
    }
}

void render_stuff(SDL_Renderer* r, maze& map) {
    const int tsize = 20;
    SDL_SetRenderDrawColor(r, 0, 0, 0, 0);
    SDL_RenderClear(r);
    SDL_SetRenderDrawColor(r, 255, 255, 255, 0);
    for (int i = 0; i < map.size(); i++)
        for (int j = 0; j < map[0].size(); j++)
            if (map[i][j]) {
                SDL_Rect tile{i * tsize, j * tsize, tsize, tsize};
                SDL_RenderFillRect(r, &tile);
            }
    point a_map = a / tsize, b_map = b / tsize;
    point dir = b_map - a_map;
    if (DDA_raycast(map, a_map.x, a_map.y, dir.x, dir.y).hit)
        SDL_SetRenderDrawColor(r, 255, 0, 0, 0);
    else
        SDL_SetRenderDrawColor(r, 0, 255, 0, 0);
    SDL_RenderDrawLine(r, a.x, a.y, b.x, b.y);
}

int main(int argc, char** argv) {
    SDL_Window* w = init_sdl();
    SDL_Renderer* r = SDL_CreateRenderer(w, -1, SDL_RENDERER_ACCELERATED);
    SDL_Event e = {0};
    maze map = make_maze(10);
    while (e.type != SDL_QUIT) {
        SDL_WaitEvent(&e);
        handle_event(e, r);
        render_stuff(r, map);
        SDL_RenderPresent(r);
    }
    SDL_DestroyRenderer(r);
    SDL_DestroyWindow(w);
    SDL_Quit();
}
