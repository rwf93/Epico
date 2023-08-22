#pragma once

namespace fs {

template<typename T>
std::vector<T> read_file(const std::filesystem::path &filename, bool binary) {
    // onelining was a mistake
	std::ifstream file(filename, binary ? std::ios::ate | std::ios::binary : std::ios::ate);

	if(!file.is_open())
		std::runtime_error(fmt::format("Failed to open file {}", filename.string()));

	size_t file_size = (size_t)file.tellg();
	std::vector<T> buffer(file_size + sizeof(T) - 1 / sizeof(T));

	file.seekg(0);
	file.read(reinterpret_cast<char*>(buffer.data()), static_cast<std::streamsize>(file_size));
	file.close();

	return buffer;
}

template<typename T>
std::vector<T> read_asset(const std::filesystem::path &filename, bool binary) {
	return fs::read_file<T>(std::filesystem::current_path().append("assets/").append(filename.string()), binary);
}

}