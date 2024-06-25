#pragma once

struct SDL_Window;
struct AppContext {
	SDL_Window *window = nullptr;
	unsigned int width = 0;
	unsigned int height = 0;
};