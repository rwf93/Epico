#pragma once

#define LOGGER spdlog::get("api_vulkan")

#include <magic_enum.hpp>

// Queue of functions, destroy() iterates the queue in reverse and calls them.
template<typename Return = void, typename ...Args>
class FunctorQueue {
	using Functor = std::function<Return(Args...)>;
public:
	void push(Functor &&f) {
		functors.push_back(f);
	}

	void call_forward(Args... params) {
		for(auto it = functors.begin(); it != functors.end(); it++)
			(*it)(params...);
	}

	void call_backward(Args... params) {
		for(auto it = functors.rbegin(); it != functors.rend(); it++)
			(*it)(params...);
	}

	void destroy_forward(Args... params) {
		call_forward(params...);
		functors.clear();
	}

	void destroy_backward(Args... params) {
		call_backward(params...);
		functors.clear();
	}

private:
	std::deque<Functor> functors;
};

template<>
struct fmt::formatter<VkResult> : fmt::formatter<std::string> {
	auto format(VkResult my, format_context &ctx) const -> decltype(ctx.out()) {
		return fmt::format_to(ctx.out(), "{}", magic_enum::enum_name(my));
	}
};

#define VK_CHECK(f)                                                 \
{                                                                   \
	VkResult result = (f);                                          \
	assert(result == VK_SUCCESS);                                   \
	if(result != VK_SUCCESS) {                                      \
		LOGGER->error("VkResult is {} in {} @ {}", result, __FILE__, __LINE__);  \
		std::abort();                                               \
	}                                                               \
}

#define VK_ALIGN(size, alignment) ( (size + alignment - 1) & ~(alignment - 1) )
#define VK_ALIGN_BOUNDS(size, alignment) ( alignment > 0 ? VK_ALIGN(size, alignment) : size )
static_assert(VK_ALIGN(1024, 0) != 1024);
static_assert(VK_ALIGN_BOUNDS(1024, 0) == 1024);

#define VK_TRACY_MEMORY_OVERLOADS 		\
	void *operator new(size_t size) { 	\
        void *p = ::operator new(size); \
        TracyAlloc(p, size); 			\
        return p; 						\
    } 									\
    void operator delete(void *p) { 	\
        TracyFree(p); 					\
        free(p); 						\
    }