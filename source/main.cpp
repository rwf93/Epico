#include "globals.h"
#include "renderer/renderer.h"

int main(int argc, char *args[]) {
	UNUSED(argc);
	UNUSED(args);

	GameGlobals game = {};
	render::Renderer renderer = {};

#if !defined(NDEBUG)
	spdlog::set_level(spdlog::level::debug);
#else
	spdlog::set_level(spdlog::level::info);
#endif

	if(SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		spdlog::error("SDL Failed to init with error: {}", SDL_GetError());
		return 0;
	}

	if(!create_window(&game)) return 0;
	if(!renderer.setup(&game)) return 0;

	static bool quit = false;
	game.quit = &quit;

	clock_t start_time = std::clock();

	while(!quit) {
		clock_t time = std::clock();
		game.time_delta = static_cast<float>(time - start_time) / CLOCKS_PER_SEC;
		game.time = static_cast<float>(time) / CLOCKS_PER_SEC;

		SDL_Event e;
		while(SDL_PollEvent(&e)) {
			switch(e.type) {
				case SDL_QUIT: quit = true; break;
				default: break;
			}

			ImGui_ImplSDL2_ProcessEvent(&e);
		}


		if(!renderer.begin()) break;
		if(!renderer.end()) break;

		start_time = time;
	}

	SDL_DestroyWindow(game.window);
	SDL_Quit();

	return 0;
}

bool create_window(GameGlobals *game) {
	game->window = SDL_CreateWindow("Epico", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN );
	if(game->window == NULL) {
		spdlog::error("Failed to create SDL Window {}", SDL_GetError());
		return false;
	}

	return true;
}

