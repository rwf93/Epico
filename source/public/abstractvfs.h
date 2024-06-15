#pragma once

#include <string>

class AbstractVFS {
public:
    virtual ~AbstractVFS() {};
    virtual void mount(std::string virtual_dir, std::string physical_dir) = 0;
    virtual void unmount(std::string virtual_dir) = 0;
    virtual std::string resolve_physical_dir(std::string virtual_dir) = 0;
};