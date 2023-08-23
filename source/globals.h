#pragma once

#define UNIMPLEMENTED spdlog::error("Unimplemented @ {}:{}:{}", __func__, __FILE__, __LINE__);

struct GameGlobals {
	SDL_Window *window;
};

#define FUNC_CREATE_WINDOW GameGlobals *game
#define FUNC_READ_FILE const std::string &filename, bool binary

bool create_window(FUNC_CREATE_WINDOW);