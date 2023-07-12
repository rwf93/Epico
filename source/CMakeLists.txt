add_executable(${PROJECT}
	main.cpp
    vulkan.cpp
)

target_precompile_headers(${PROJECT} PUBLIC pch.h)

find_package(fmt CONFIG REQUIRED)
target_link_libraries(${PROJECT} PRIVATE fmt::fmt-header-only)

find_package(spdlog CONFIG REQUIRED)
target_link_libraries(${PROJECT} PRIVATE spdlog::spdlog_header_only)

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