#pragma once

#include <string>
#include <filesystem>
#include <fstream>
#include <vector>

class AbstractFilesystem {
public:
    virtual ~AbstractFilesystem() {};
    virtual void mount(std::filesystem::path virtual_path, std::filesystem::path physical_path) = 0;
    virtual void unmount(std::filesystem::path virtual_path) = 0;
    virtual std::filesystem::path resolve_physical_dir(std::filesystem::path virtual_path) = 0;

    template<typename T>
    std::vector<T> read_file(std::filesystem::path virtual_path, bool binary = false) {
        std::ifstream file(resolve_physical_dir(virtual_path), binary ? std::ios::ate | std::ios::binary : std::ios::ate);

        if(!file.is_open()) {
            std::runtime_error("VFS exception reading file. Check debugger.");
            return {};
        }

        size_t file_size = static_cast<size_t>(file.tellg());
        std::vector<T> buffer(file_size);

        file.seekg(0);
        file.read(static_cast<char*>(buffer.data()), static_cast<std::streamsize>(file_size));
        file.close();

        return buffer;
    }
};
