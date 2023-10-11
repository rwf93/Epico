add_library(${PROJECT}_VulkanRenderer SHARED
    vkinstance.cpp
    vksurface.cpp
    vkdevice.cpp
    vkswapchain.cpp

    rendererimpl.cpp
)

target_include_directories(${PROJECT}_VulkanRenderer PRIVATE ${SOURCE})
target_include_directories(${PROJECT}_VulkanRenderer PRIVATE ${SOURCE}/renderer)

target_precompile_headers(${PROJECT}_VulkanRenderer PRIVATE stdafx.h)

find_package(fmt CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(Vulkan REQUIRED)
find_package(vk-bootstrap CONFIG REQUIRED)
find_package(unofficial-vulkan-memory-allocator CONFIG REQUIRED)
find_package(SDL2 CONFIG REQUIRED)
target_link_libraries(${PROJECT}_VulkanRenderer PRIVATE Vulkan::Vulkan)
target_link_libraries(${PROJECT}_VulkanRenderer PRIVATE vk-bootstrap::vk-bootstrap)
target_link_libraries(${PROJECT}_VulkanRenderer PRIVATE unofficial::vulkan-memory-allocator::vulkan-memory-allocator)
target_link_libraries(${PROJECT}_VulkanRenderer PRIVATE fmt::fmt-header-only)
target_link_libraries(${PROJECT}_VulkanRenderer PRIVATE spdlog::spdlog_header_only)
target_link_libraries(${PROJECT}_VulkanRenderer
	PRIVATE
	$<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
	$<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
)