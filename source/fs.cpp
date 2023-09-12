#include <string>
#include <vector>

#include "globals.h"

#include "fs.h"

std::string fs::read_string_file(const std::filesystem::path &filename) {
	std::vector<char> char_data = fs::read_file<char>(filename, false);
	return std::string(char_data.begin(), char_data.end());
}

std::string fs::read_string_local(const std::filesystem::path &filename) {
	std::vector<char> char_data = fs::read_local<char>(filename, false);
	return std::string(char_data.begin(), char_data.end());
}

std::string fs::read_string_asset(const std::filesystem::path &filename) {
	std::vector<char> char_data = fs::read_asset<char>(filename, false);
	return std::string(char_data.begin(), char_data.end());
}