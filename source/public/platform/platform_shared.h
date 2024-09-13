#pragma once

#define UNUSED(arg) (void)(arg);

#if defined(EAPI_EXPORT)
	#define EAPI SHARED_EXPORT
#else
	#define EAPI SHARED_IMPORT
#endif

typedef WIN_LINUX(HINSTANCE, void*) ModuleHandle;
typedef WIN_LINUX(FARPROC, void*) ModuleSymbol;

inline ModuleHandle platform_loadlibrary(const char *name, std::filesystem::path dir = "./") {
	std::string prefix = WIN_LINUX("", "lib");
	std::string suffix = WIN_LINUX(".dll", ".so");
	std::string fullpath = dir.make_preferred().string() + prefix + name + suffix;

	ModuleHandle handle = WIN_LINUX(::LoadLibraryA(fullpath.c_str()), dlopen(fullpath.c_str(), RTLD_NOW | RTLD_LOCAL));
	if(handle)
		return handle;

	return nullptr;
}

inline ModuleSymbol platform_get_symbol(ModuleHandle handle, const char *symbol_name) {
	return WIN_LINUX(GetProcAddress, dlsym)(handle, symbol_name);
}

template<typename T>
inline T *platform_get_function(ModuleHandle handle, const char *symbol_name) {
	return reinterpret_cast<T*>(platform_get_symbol(handle, symbol_name));
}

inline void platform_freelibrary(ModuleHandle handle) {
	WIN_LINUX(FreeLibrary, dlclose)(handle);
}

#define ONCE(BLOCK) 				\
	{								\
		static bool once = false; 	\
		if(!once) { 				\
			BLOCK					\
			once = true;			\
		}							\
	}

