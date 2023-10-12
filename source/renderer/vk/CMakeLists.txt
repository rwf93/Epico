add_library(VulkanRenderer SHARED
    vma.cpp
    info.cpp

    vkinstance.cpp
    vksurface.cpp
    vkdevice.cpp
    vkswapchain.cpp
    vkcommandpool.cpp

    rendererimpl.cpp
)

target_include_directories(VulkanRenderer PRIVATE ${SOURCE})
target_include_directories(VulkanRenderer PRIVATE ${SOURCE}/renderer)

target_precompile_headers(VulkanRenderer PRIVATE stdafx.h)

find_package(fmt CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(Vulkan REQUIRED)
find_package(vk-bootstrap CONFIG REQUIRED)
find_package(unofficial-vulkan-memory-allocator CONFIG REQUIRED)
find_package(SDL2 CONFIG REQUIRED)
find_package(magic_enum CONFIG REQUIRED)
target_link_libraries(VulkanRenderer PRIVATE Vulkan::Vulkan)
target_link_libraries(VulkanRenderer PRIVATE vk-bootstrap::vk-bootstrap)
target_link_libraries(VulkanRenderer PRIVATE unofficial::vulkan-memory-allocator::vulkan-memory-allocator)
target_link_libraries(VulkanRenderer PRIVATE fmt::fmt-header-only)
target_link_libraries(VulkanRenderer PRIVATE spdlog::spdlog_header_only)
target_link_libraries(VulkanRenderer
	PRIVATE
	$<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
	$<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
)
target_link_libraries(VulkanRenderer PRIVATE magic_enum::magic_enum)