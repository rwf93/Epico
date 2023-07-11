int main(int argc, char *args[]) {
	gameGlobals game;
	vulkanRenderer renderer(&game);

	//renderPassConstructor render_pass(&game, &renderer);
	//renderPipelineConstructor graphics_pipeline(&game, &renderer, &render_pass)

	if(SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		fmt::println("SDL Failed to init with error: {}", SDL_GetError());
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
		}
	}

	SDL_DestroyWindow(game.window);
	SDL_Quit();

	return 0;
}

bool create_window(FUNC_CREATE_WINDOW) {
	game->window = SDL_CreateWindow("Epico", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_SHOWN | SDL_WINDOW_VULKAN );
	if(game->window == NULL) {
		fmt::println("Failed to create SDL Window {}", SDL_GetError());
		return false;
	}

	return true;
}

std::vector<char> read_file(FUNC_READ_FILE) {
	// onelining was a mistake
	std::ifstream file(filename, binary ? std::ios::ate | std::ios::binary : std::ios::ate);

	if(!file.is_open())
		std::runtime_error(fmt::format("Failed to open file {}", filename));

	size_t file_size = (size_t)file.tellg();
	std::vector<char> buffer(file_size);

	file.seekg(0);
	file.read(buffer.data(), static_cast<std::streamsize>(file_size));
	file.close();

	return buffer;
}
