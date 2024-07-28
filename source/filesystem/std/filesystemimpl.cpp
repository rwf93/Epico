#include "filesystemimpl.h"

StandardFilesystem::StandardFilesystem(AppContext *app_context) {
	this->context = app_context;
	auto console = spdlog::stdout_color_mt("filesystem");
	UNUSED(console);
}

void StandardFilesystem::mount(std::filesystem::path virtual_dir, std::filesystem::path physical_dir) {
	spdlog::get("filesystem")->info("Mounting {} to {}", virtual_dir.string(), physical_dir.string());
	mounts[virtual_dir].push_back(physical_dir);
}

void StandardFilesystem::unmount(std::filesystem::path virtual_dir) {
	mounts[virtual_dir].clear();
}

std::filesystem::path StandardFilesystem::resolve_physical_dir(std::filesystem::path virtual_path) {
	std::string virtual_dir = virtual_path.string();
	int path_index = 0;

	for(size_t i = 0; i < virtual_dir.size(); i++)
		if(virtual_dir[i] == '/')
			path_index = i;

	std::string path = virtual_dir.substr(0, path_index + 1);
	std::string name = virtual_dir.substr(path.length(), virtual_dir.length());

	std::filesystem::path physical_dir = "";

	// C++ does not provide a standard way of getting the executable directory...
	// This is required if you want to call the executable from *any* location.
	// Relative paths consider the current directory!
	auto cwd = std::filesystem::weakly_canonical(std::filesystem::path(context->argv[0])).parent_path();
	for(auto &item: mounts[path]) {
		auto path_name = cwd.append(item.string() + name).make_preferred();

		std::ifstream check(path_name);
		if(check.good())
			physical_dir = path_name;
	}

	return std::filesystem::canonical(physical_dir);
}

static StandardFilesystem *singleton;

extern "C" EAPI StandardFilesystem *create_factory(void *user_data) {
	if(!singleton)
		singleton = new StandardFilesystem(reinterpret_cast<AppContext*>(user_data));
	return singleton;
}