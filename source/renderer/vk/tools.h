#pragma once

#include <magic_enum.hpp>

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
        spdlog::error("VkResult is {} in {} @ {}", result, __FILE__, __LINE__);  \
        std::abort();                                               \
    }                                                               \
}

#define VK_ALIGN(size, alignment) ( (size + alignment - 1) & ~(alignment - 1) )
#define VK_ALIGN_BOUNDS(size, alignment) ( alignment > 0 ? VK_ALIGN(size, alignment) : size )
static_assert(VK_ALIGN(1024, 0) != 1024);
static_assert(VK_ALIGN_BOUNDS(1024, 0) == 1024);
