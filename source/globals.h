#pragma once

struct gameGlobals {
	SDL_Window *window;
};

#define FUNC_CREATE_WINDOW gameGlobals *game
#define FUNC_READ_FILE const std::string &filename, bool binary

bool create_window(FUNC_CREATE_WINDOW);
std::vector<char> read_file(FUNC_READ_FILE = false);