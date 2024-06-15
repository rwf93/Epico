#include "filesystemimpl.h"

void StandardFilesystem::mount(std::string virtual_dir, std::string physical_dir) {
    spdlog::info("Mounting {} to {}", virtual_dir, physical_dir);
    mounts[virtual_dir].push_back(physical_dir);
}

void StandardFilesystem::unmount(std::string virtual_dir) {
    mounts[virtual_dir].clear();
}

std::string StandardFilesystem::resolve_physical_dir(std::string virtual_dir) {
    int path_index = 0;

    for(int i = 0; i < virtual_dir.size(); i++)
        if(virtual_dir[i] == '/')
            path_index = i;

    std::string path = virtual_dir.substr(0, path_index + 1);
    std::string name = virtual_dir.substr(path.length(), virtual_dir.length());

    std::string physical_dir = "";

    for(auto &item: mounts[path]) {
        auto path_name = item + name;

        std::ifstream check(path_name);
        if(check.good())
            physical_dir = path_name;
    }

    return physical_dir;
}

static StandardFilesystem *singleton;

extern "C" EAPI StandardFilesystem *create_factory(void *user_data) {
    if(!singleton)
        singleton = new StandardFilesystem();
    return singleton;
}