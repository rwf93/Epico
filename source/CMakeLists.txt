add_executable(${PROJECT}
	main.cpp
    fs.cpp

    renderer/vk/vulkan.cpp
    renderer/vk/pipeline.cpp
)

set_property(TARGET ${PROJECT} PROPERTY CXX_STANDARD 23)
set_property(TARGET ${PROJECT} PROPERTY C_STANDARD 23)

include_directories(${SOURCE})

target_precompile_headers(${PROJECT} PUBLIC stdafx.h)

find_package(fmt CONFIG REQUIRED)
target_link_libraries(${PROJECT} PRIVATE fmt::fmt-header-only)

find_package(spdlog CONFIG REQUIRED)
target_link_libraries(${PROJECT} PRIVATE spdlog::spdlog_header_only)

find_package(magic_enum CONFIG REQUIRED)
target_link_libraries(${PROJECT} PRIVATE magic_enum::magic_enum)

find_package(Vulkan REQUIRED)
target_link_libraries(${PROJECT} PRIVATE Vulkan::Vulkan)

find_package(vk-bootstrap CONFIG REQUIRED)
target_link_libraries(${PROJECT} PRIVATE vk-bootstrap::vk-bootstrap)

find_package(unofficial-vulkan-memory-allocator CONFIG REQUIRED)
target_link_libraries(${PROJECT} PRIVATE unofficial::vulkan-memory-allocator::vulkan-memory-allocator)

find_package(glm CONFIG REQUIRED)
target_link_libraries(${PROJECT} PRIVATE glm::glm)

find_package(assimp CONFIG REQUIRED)
target_link_libraries(${PROJECT} PRIVATE assimp::assimp)

find_package(imgui CONFIG REQUIRED)
target_link_libraries(${PROJECT} PRIVATE imgui::imgui)

find_package(SDL2 CONFIG REQUIRED)
target_link_libraries(${PROJECT}
    PRIVATE
    $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
    $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
)