#pragma once

#define UNUSED(arg) (void)(arg);

#ifdef EAPI_EXPORT
	#define EAPI SHARED_EXPORT
#else
	#define EAPI SHARED_IMPORT
#endif

typedef WIN_LINUX(HINSTANCE, void*) ModuleHandle_t;
typedef WIN_LINUX(FARPROC, void*) ModuleSymbol_t;

inline ModuleHandle_t platform_loadlibrary(const char *name, std::filesystem::path dir = "./") {
	std::string prefix = WIN_LINUX("", "lib");
	std::string suffix = WIN_LINUX(".dll", ".so");
	std::string fullpath = dir.make_preferred().string() + prefix + name + suffix;

	ModuleHandle_t handle = WIN_LINUX(::LoadLibraryA(fullpath.c_str()), dlopen(fullpath.c_str(), RTLD_NOW | RTLD_LOCAL));
	if(handle)
		return handle;

	return nullptr;
}

inline ModuleSymbol_t platform_get_symbol(ModuleHandle_t handle, const char *symbol_name) {
	return WIN_LINUX(GetProcAddress, dlsym)(handle, symbol_name);
}

template<typename T>
inline T *platform_get_function(ModuleHandle_t handle, const char *symbol_name) {
	return reinterpret_cast<T*>(platform_get_symbol(handle, symbol_name));
}

inline void platform_freelibrary(ModuleHandle_t handle) {
	WIN_LINUX(FreeLibrary, dlclose)(handle);
}

template<typename T>
struct FactoryHandle {
	ModuleHandle_t handle = nullptr;
	T interface = nullptr;
	bool good = false;

	void release() {
		if(good) {
			delete interface;
			platform_freelibrary(handle);
		}
	}

	T operator->() {
		if(good)
			return interface;
		return nullptr;
	}
};

// Loads a shared library, calls it's factory function, and returns a FactoryHandle instance.
template<typename T>
inline FactoryHandle<T> get_factory(const char *binary, const char *factory_function = "create_factory", std::filesystem::path dir = "./") {
	using CreateFactoryType = T();
	CreateFactoryType *create_factory;
	T factory_result;

	ModuleHandle_t handle = platform_loadlibrary(binary, dir);

	if(!handle)
		goto fail;

	create_factory = platform_get_function<CreateFactoryType>(handle, factory_function);

	if(!create_factory)
		goto fail_factory;

	factory_result = create_factory();
	if(!factory_result)
		goto fail_factory;

	return {handle, factory_result, true};
fail_factory:
	platform_freelibrary(handle);
fail:
	return {nullptr, nullptr, false};
};

#define CREATE_FACTORY(CONCRETE_IMPL) 							\
	extern "C" EAPI CONCRETE_IMPL *create_factory() { 			\
		static CONCRETE_IMPL *factory_impl = nullptr; 			\
		if(!factory_impl) factory_impl = new CONCRETE_IMPL (); \
		return factory_impl; 									\
	}

#define ONCE(BLOCK) 				\
	{								\
		static bool once = false; 	\
		if(!once) { 				\
			BLOCK					\
			once = true;			\
		}							\
	}

