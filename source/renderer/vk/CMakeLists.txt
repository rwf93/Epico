add_library(renderer_vk SHARED
    vma.cpp
    info.cpp

    vkinstance.cpp
    vksurface.cpp
    vkdevice.cpp
    vkswapchain.cpp
    vkcommandpool.cpp
    vkimage.cpp
    vkimgui.cpp

    rendererimpl.cpp
)

target_include_directories(renderer_vk PRIVATE ${SOURCE})
target_include_directories(renderer_vk PRIVATE ${SOURCE}/renderer)

target_precompile_headers(renderer_vk PRIVATE stdafx.h)

find_package(fmt CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(Vulkan REQUIRED)
find_package(vk-bootstrap CONFIG REQUIRED)
find_package(unofficial-vulkan-memory-allocator CONFIG REQUIRED)
find_package(SDL2 CONFIG REQUIRED)
find_package(magic_enum CONFIG REQUIRED)
target_link_libraries(renderer_vk PRIVATE Vulkan::Vulkan)
target_link_libraries(renderer_vk PRIVATE vk-bootstrap::vk-bootstrap)
target_link_libraries(renderer_vk PRIVATE unofficial::vulkan-memory-allocator::vulkan-memory-allocator)
target_link_libraries(renderer_vk PRIVATE fmt::fmt-header-only)
target_link_libraries(renderer_vk PRIVATE spdlog::spdlog_header_only)
target_link_libraries(renderer_vk
	PRIVATE
	$<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
	$<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
)
target_link_libraries(renderer_vk PRIVATE magic_enum::magic_enum)

target_include_directories(renderer_vk PRIVATE ${THIRDPARTY}/imgui)
target_link_libraries(renderer_vk PRIVATE imgui)

target_compile_options(renderer_vk PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
  $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Werror>
)