#pragma once

template<>
struct fmt::formatter<VkResult> : fmt::formatter<std::string>
{
    auto format(VkResult my, format_context &ctx) const -> decltype(ctx.out())
    {
        return fmt::format_to(ctx.out(), "{}", magic_enum::enum_name(my));
    }
};

#define VK_CHECK_RESULT(f)																				\
{																										\
	VkResult res = (f);																					\
	if (res != VK_SUCCESS) {																			\
		spdlog::error("VkResult is {} in {} @ {}", res, __FILE__, __LINE__);							\
		assert(res == VK_SUCCESS);																		\
		return false;																					\
	}																									\
}
