#pragma once

#define UNIMPLEMENTED spdlog::error("Unimplemented @ {}:{}:{}", __func__, __FILE__, __LINE__);
#define UNUSED(_VAR) ((void)(_VAR));

struct GameGlobals {
	SDL_Window *window;
	bool *quit;
};

bool create_window(GameGlobals *game);