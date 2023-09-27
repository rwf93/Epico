find_package(fmt CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(magic_enum CONFIG REQUIRED)
find_package(Vulkan REQUIRED)
find_package(vk-bootstrap CONFIG REQUIRED)
find_package(unofficial-vulkan-memory-allocator CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(tinyobjloader CONFIG REQUIRED)
# find_package(imgui CONFIG REQUIRED)
find_package(imguizmo CONFIG REQUIRED)
find_package(SDL2 CONFIG REQUIRED)

add_executable(${PROJECT}
	renderer/info.cpp
	renderer/pipeline.cpp
	renderer/primitives.cpp
	renderer/mesh.cpp
	renderer/renderable.cpp
	renderer/renderer.cpp

	mathlib.cpp
	fs.cpp

	main.cpp
)

target_precompile_headers(${PROJECT} PUBLIC stdafx.h)
target_link_libraries(${PROJECT} PRIVATE fmt::fmt-header-only)
target_link_libraries(${PROJECT} PRIVATE spdlog::spdlog_header_only)
target_link_libraries(${PROJECT} PRIVATE magic_enum::magic_enum)
target_link_libraries(${PROJECT} PRIVATE Vulkan::Vulkan)
target_link_libraries(${PROJECT} PRIVATE vk-bootstrap::vk-bootstrap)
target_link_libraries(${PROJECT} PRIVATE unofficial::vulkan-memory-allocator::vulkan-memory-allocator)
target_link_libraries(${PROJECT} PRIVATE glm::glm)
target_link_libraries(${PROJECT} PRIVATE tinyobjloader::tinyobjloader)
# target_link_libraries(${PROJECT} PRIVATE imgui::imgui)
target_link_libraries(${PROJECT} PRIVATE imguizmo::imguizmo)
target_link_libraries(${PROJECT}
	PRIVATE
	$<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
	$<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
)

target_compile_options(${PROJECT} PRIVATE
	$<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
	$<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Werror>
)

target_include_directories(${PROJECT} PRIVATE ${THIRDPARTY}/imgui)
target_link_libraries(${PROJECT} PRIVATE imgui)

target_include_directories(${PROJECT} PUBLIC ${SOURCE})
target_include_directories(${PROJECT} PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

set_property(TARGET ${PROJECT} PROPERTY CXX_STANDARD 23)
set_property(TARGET ${PROJECT} PROPERTY C_STANDARD 23)