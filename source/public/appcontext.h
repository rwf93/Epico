#pragma once

struct SDL_Window;
struct AppContext {
    SDL_Window *current_window = nullptr;
};