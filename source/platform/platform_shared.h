#pragma once

#ifdef EAPI_EXPORT
    #define EAPI SHARED_EXPORT
#else
    #define EAPI SHARED_IMPORT
#endif

typedef WIN_LINUX(HINSTANCE, void*) handle_t;
typedef WIN_LINUX(FARPROC, void*) symbol_t;

handle_t platform_loadlibrary(const char *name, const char *dir = WIN_LINUX(".\\", "./")) {
    std::string prefix = WIN_LINUX("", "lib");
    std::string suffix = WIN_LINUX(".dll", ".so");
    std::string fullpath = dir + prefix + name + suffix;

    handle_t handle = WIN_LINUX(::LoadLibraryA(fullpath.c_str()), dlopen(fullpath.c_str(), RTLD_NOW | RTLD_LOCAL));
    if(handle)
        return handle;

    return nullptr;
}

symbol_t platform_get_symbol(handle_t handle, const char *symbol_name) {
    return WIN_LINUX(GetProcAddress, dlsym)(handle, symbol_name);
}

template<typename T>
T *platform_get_function(handle_t handle, const char *symbol_name) {
    return reinterpret_cast<T*>(platform_get_symbol(handle, symbol_name));
}

void platform_freelibrary(handle_t handle) {
    WIN_LINUX(FreeLibrary, dlclose)(handle);
}

template<typename T>
struct FactoryHandle {
    handle_t handle = nullptr;
    T interface = nullptr;
    bool good = false; // Determines if the result of get_factory is successful
    // Free the instantiated interface, and free the loaded library.
    void release() {
        if(good)
            delete interface;
            platform_freelibrary(handle);
    }

    T operator->() { return interface; }
};

// Loads a shared library, calls it's factory function, and returns a FactoryHandle instance.
template<typename T>
FactoryHandle<T> get_factory(const char *binary, void *user_data = nullptr, const char *factory_function = "create_factory", const char *dir = WIN_LINUX(".\\", "./")) {
    handle_t handle = platform_loadlibrary(binary);

    if(!handle)
        return {nullptr, nullptr, false};

    auto create_factory = platform_get_function<T(void*)>(handle, factory_function);

    if(!create_factory) {
        platform_freelibrary(handle);
        return {nullptr, nullptr, false};
    }

    return {handle, create_factory(user_data), true};
};