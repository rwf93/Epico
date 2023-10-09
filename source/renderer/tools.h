#pragma once

template<>
struct fmt::formatter<VkResult> : fmt::formatter<std::string> {
	auto format(VkResult my, format_context &ctx) const -> decltype(ctx.out()) {
		return fmt::format_to(ctx.out(), "{}", magic_enum::enum_name(my));
	}
};

#define VK_CHECK_RET(f, ret)																			\
{																										\
	VkResult res = (f);																					\
	if (res != VK_SUCCESS) {																			\
		spdlog::error("VkResult is {} in {} @ {}", res, __FILE__, __LINE__);							\
		assert(res == VK_SUCCESS);																		\
		return ret;																						\
	}																									\
}

#define VK_CHECK_BOOL(f) VK_CHECK_RET(f, false);
// such a hack
#define VK_CHECK_VOID(f) VK_CHECK_RET(f, ;);

#define VK_ALIGN(size, alignment) ( (size + alignment - 1) & ~(alignment - 1) )
#define VK_ALIGN_BOUNDS(size, alignment) ( alignment > 0 ? VK_ALIGN(size, alignment) : size )

static_assert(VK_ALIGN(1024, 0) != 1024);
static_assert(VK_ALIGN_BOUNDS(1024, 0) == 1024);