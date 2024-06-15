#pragma once

class StandardFilesystem: public AbstractVFS {
public:
    void mount(std::string virtual_dir, std::string physical_dir) override;
    void unmount(std::string virtual_dir) override;
    std::string resolve_physical_dir(std::string virtual_dir) override;
private:
    std::map<std::string, std::vector<std::string>> mounts;
};