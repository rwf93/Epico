#pragma once

#include <string>
#include <filesystem>

class AbstractVFS {
public:
    virtual ~AbstractVFS() {};
    virtual void mount(std::filesystem::path virtual_dir, std::filesystem::path physical_dir) = 0;
    virtual void unmount(std::filesystem::path virtual_dir) = 0;
    virtual std::filesystem::path resolve_physical_dir(std::filesystem::path virtual_dir) = 0;
};