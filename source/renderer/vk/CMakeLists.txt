add_library(${PROJECT}_VulkanRenderer SHARED
    vkinstance.cpp
    rendererimpl.cpp
)

target_include_directories(${PROJECT}_VulkanRenderer PRIVATE ${SOURCE})
target_include_directories(${PROJECT}_VulkanRenderer PRIVATE ${SOURCE}/renderer)

target_precompile_headers(${PROJECT}_VulkanRenderer PUBLIC stdafx.h)

find_package(fmt CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(Vulkan REQUIRED)
find_package(vk-bootstrap CONFIG REQUIRED)
find_package(unofficial-vulkan-memory-allocator CONFIG REQUIRED)
target_link_libraries(${PROJECT}_VulkanRenderer PRIVATE Vulkan::Vulkan)
target_link_libraries(${PROJECT}_VulkanRenderer PRIVATE vk-bootstrap::vk-bootstrap)
target_link_libraries(${PROJECT}_VulkanRenderer PRIVATE unofficial::vulkan-memory-allocator::vulkan-memory-allocator)
target_link_libraries(${PROJECT}_VulkanRenderer PRIVATE fmt::fmt-header-only)
target_link_libraries(${PROJECT}_VulkanRenderer PRIVATE spdlog::spdlog_header_only)