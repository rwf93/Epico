#pragma once

#include <public/appcontext.h>

class StandardFilesystem: public Filesystem {
public:
	StandardFilesystem(AppContext *app_context);

	void mount(std::filesystem::path virtual_path, std::filesystem::path physical_dir) override;
	void unmount(std::filesystem::path virtual_path) override;
	std::filesystem::path resolve_physical_dir(std::filesystem::path virtual_path) override;
private:
	std::map<std::filesystem::path, std::vector<std::filesystem::path>> mounts;
	AppContext *context;
};