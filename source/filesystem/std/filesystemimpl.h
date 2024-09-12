#pragma once

#include <public/appcontext.h>

class StandardFilesystem: public Filesystem {
public:
	StandardFilesystem(AppContext *context);

	void mount(std::filesystem::path virtual_path, std::filesystem::path physical_dir) override;
	void unmount(std::filesystem::path virtual_path) override;
	std::filesystem::path resolve_physical_dir(std::filesystem::path virtual_path) override;

	void add_ref() { ref_count++; }
    void del_ref() { ref_count--; }
    uint32_t get_ref() { return ref_count; }

private:
	std::atomic<uint32_t> ref_count = 0;
	std::map<std::filesystem::path, std::vector<std::filesystem::path>> mounts;
	AppContext *context;
};