#pragma once

#define UNIMPLEMENTED spdlog::error("Unimplemented @ {}:{}:{}", __func__, __FILE__, __LINE__);

struct gameGlobals {
	SDL_Window *window;
};

#define FUNC_CREATE_WINDOW gameGlobals *game
#define FUNC_READ_FILE const std::string &filename, bool binary

bool create_window(FUNC_CREATE_WINDOW);
template<typename T> std::vector<T> read_file(const std::string &filename, bool binary) {
	// onelining was a mistake
	std::ifstream file(filename, binary ? std::ios::ate | std::ios::binary : std::ios::ate);

	if(!file.is_open())
		std::runtime_error(fmt::format("Failed to open file {}", filename));

	size_t file_size = (size_t)file.tellg();
	std::vector<T> buffer(file_size + sizeof(T) - 1 / sizeof(T));

	file.seekg(0);
	file.read(reinterpret_cast<char*>(buffer.data()), static_cast<std::streamsize>(file_size));
	file.close();

	return buffer;
}
