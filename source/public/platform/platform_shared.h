#pragma once

#define UNUSED(arg) (void)(arg);

#if defined(EAPI_EXPORT)
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

#include <public/refcountable.h>
template<class T, class U>
concept Derived = std::is_base_of<U, T>::value;

template<Derived<RefCountable> T, typename ContextType>
class FactoryHandle {
public:
	FactoryHandle(
		const char *binary,
		ContextType context,
		std::filesystem::path dir = "./",
		const char *factory_function = "create_factory"
	)
		: handle(platform_loadlibrary(binary, dir))
		, create_factory(platform_get_function<CreateFactoryType>(handle, factory_function))
		, interface(create_factory(context)) {
			interface->add_ref();
		}

	~FactoryHandle() {
		release();
	}

	void release() {
		if(interface->get_ref() > 0) {
			interface->del_ref();
			return;
		}

		delete interface;
		platform_freelibrary(handle);
	}

	T *operator->() {
		return interface;
	}

	operator T*() {
		return interface;
	}

private:
	using CreateFactoryType = T*(ContextType);
	ModuleHandle_t handle = nullptr;
	CreateFactoryType *create_factory = nullptr;
	T *interface = nullptr;
};

#define CREATE_FACTORY(CONCRETE_IMPL, CONTEXT_TYPE)									\
	extern "C" EAPI CONCRETE_IMPL *create_factory(AppContext *context) { 			\
		static CONCRETE_IMPL *factory_impl = nullptr; 								\
		if(!factory_impl) factory_impl = new CONCRETE_IMPL (context); 				\
		return factory_impl; 														\
	}

#define ONCE(BLOCK) 				\
	{								\
		static bool once = false; 	\
		if(!once) { 				\
			BLOCK					\
			once = true;			\
		}							\
	}

