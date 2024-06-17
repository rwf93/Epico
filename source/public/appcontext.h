#pragma once

struct SDL_Window;
struct AppContext {
    SDL_Window *window = nullptr;
    int width = 0;
    int height = 0;
};