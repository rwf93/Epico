add_executable(${PROJECT}
	main.cpp
)

target_precompile_headers(${PROJECT} PUBLIC pch.h)

find_package(fmt CONFIG REQUIRED)
target_link_libraries(${PROJECT} PRIVATE fmt::fmt-header-only)

find_package(Vulkan REQUIRED)
target_link_libraries(${PROJECT} PRIVATE Vulkan::Vulkan)

find_package(vk-bootstrap CONFIG REQUIRED)
target_link_libraries(${PROJECT} PRIVATE vk-bootstrap::vk-bootstrap)

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

execute_process(
    COMMAND ${GIT_EXECUTABLE} rev-parse HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_HEAD
)

message(${GIT_HEAD})