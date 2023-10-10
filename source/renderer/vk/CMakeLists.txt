add_library(${PROJECT}_VulkanRenderer SHARED
    rendererimpl.cpp
)

target_include_directories(${PROJECT}_VulkanRenderer PRIVATE ${SOURCE})
target_include_directories(${PROJECT}_VulkanRenderer PRIVATE ${SOURCE}/renderer)

find_package(fmt CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
target_link_libraries(${PROJECT}_VulkanRenderer PRIVATE fmt::fmt-header-only)
target_link_libraries(${PROJECT}_VulkanRenderer PRIVATE spdlog::spdlog_header_only)