#include <string>
#include <vector>

#include "globals.h"

#include "fs.h"

using namespace fs;

std::string read_file(const std::filesystem::path &filename, bool binary) {
	std::ifstream file(filename, binary ? std::ifstream::ate | std::ifstream::binary : std::ifstream::ate);

	if(!file.is_open())
		std::runtime_error(fmt::format("Failed to open file {}", filename.string()));

	std::stringstream buffer;
	buffer << file.rdbuf();
	file.close();

	return buffer.str();
}