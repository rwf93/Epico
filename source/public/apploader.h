#pragma once

#include <public/platform/platform.h>

class RefCountable {
public:
    virtual void add_ref() = 0;
    virtual void del_ref() = 0;
    virtual uint32_t get_ref() = 0;
};

template<class T, class U>
concept Derived = std::is_base_of<U, T>::value;

template<Derived<RefCountable> T, typename ContextType>
class FactoryHandle {
	using CreateFactoryType = T*(ContextType);

	struct M {
		ModuleHandle handle;
		T *interface;
	} m;

	FactoryHandle(M m) : m(std::move(m)) {}

	void release() {
		m.interface->del_ref();
		if(m.interface->get_ref() > 0)
			return;

		delete m.interface;
		platform_freelibrary(m.handle);
	}

public:
	~FactoryHandle() {
		release();
	}

	static FactoryHandle create(
		const char *binary,
		ContextType context,
		std::filesystem::path dir = "./",
		const char *factory_function = "create_factory"
	) {
		auto handle = platform_loadlibrary(binary, dir);
		auto create_factory = platform_get_function<CreateFactoryType>(handle, factory_function);
		auto interface = create_factory(context);

		return FactoryHandle(M{
			.handle = handle,
			.interface = interface
		});
	}

	T *operator->() {
		return m.interface;
	}

	operator T*() {
		return m.interface;
	}
};

#define CREATE_FACTORY(CONCRETE_IMPL, CONTEXT_TYPE)									\
	extern "C" EAPI CONCRETE_IMPL *create_factory(AppContext *context) { 			\
		static CONCRETE_IMPL *factory_impl = nullptr; 								\
		if(!factory_impl) factory_impl = new CONCRETE_IMPL (context); 				\
		return factory_impl; 														\
	}
