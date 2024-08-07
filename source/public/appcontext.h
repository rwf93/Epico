#pragma once

struct SDL_Window;
struct AppContext {
	int argc;
	char **argv;

	SDL_Window *window = nullptr;
	unsigned int width = 0;
	unsigned int height = 0;

	float time;
	float time_delta;
};