int main(int argc, char *args[]) {
	gameGlobals game;

	if(SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		fmt::println("SDL Failed to init with error: {}", SDL_GetError());
		return 0;
	}

	if(!create_window(&game)) 			 return 0;
	if(!create_vk_instance(&game)) 		 return 0;
	if(!create_surface(&game)) 			 return 0;
	if(!create_device(&game)) 			 return 0;
	if(!create_swapchain(&game)) 		 return 0;
	if(!get_queues(&game))				 return 0;
	if(!create_render_pass(&game))		 return 0;
	if(!create_graphics_pipeline(&game)) return 0;

	fmt::println("Epico instance: {}", 	(void*)game.instance.instance);
	fmt::println("Epico dewice: {}", 	(void*)game.device.device);
	fmt::println("Epico swapchain: {}", (void*)game.swapchain.swapchain);

	static bool quit = false;
	while(!quit) {
		SDL_Event e;
		while(SDL_PollEvent(&e)) {
			if(e.type == SDL_QUIT)
				quit = true;
		}
	}

	cleanup(&game);

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

void cleanup(FUNC_CLEANUP) {
	vkDestroyPipelineLayout(game->device, game->graphics_pipeline_layout, nullptr);
	vkDestroyRenderPass(game->device, game->render_pass, nullptr);

	vkb::destroy_swapchain(game->swapchain);
	vkb::destroy_device(game->device);
	vkb::destroy_surface(game->instance, game->surface);
	vkb::destroy_instance(game->instance);

	SDL_DestroyWindow(game->window);
	SDL_Quit();
}