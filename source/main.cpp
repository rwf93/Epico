#include "globals.h"

#include "renderer/vk/pipeline.h"
#include "renderer/vk/vulkan.h"

int main(int argc, char *args[]) {
	GameGlobals game;
	render::VulkanRenderer renderer(&game);

	spdlog::set_level(spdlog::level::debug);

	if(SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		spdlog::error("SDL Failed to init with error: {}", SDL_GetError());
		return 0;
	}

	if(!create_window(&game)) return 0;
	if(!renderer.setup()) return 0;

	static bool quit = false;
	while(!quit) {
		SDL_Event e;
		while(SDL_PollEvent(&e)) {
			if(e.type == SDL_QUIT)
				quit = true;

			ImGui_ImplSDL2_ProcessEvent(&e);
		}

		if(!renderer.draw()) {
			return false;
		}
	}

	SDL_DestroyWindow(game.window);
	SDL_Quit();

	return 0;
}

bool create_window(FUNC_CREATE_WINDOW) {
	game->window = SDL_CreateWindow("Epico", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN );
	if(game->window == NULL) {
		spdlog::error("Failed to create SDL Window {}", SDL_GetError());
		return false;
	}

	return true;
}

