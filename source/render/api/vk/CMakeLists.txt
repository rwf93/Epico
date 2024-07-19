add_library(api_vk SHARED
		vma.cpp

		vkinfo.cpp
		vkconvert.cpp

		vkinstance.cpp
		vksurface.cpp
		vkdevice.cpp
		vkswapchain.cpp

		vkcommandpool.cpp
		vkcommand.cpp

		vkresourcemanager.cpp
		vktexture.cpp
		vktextureview.cpp
		vkbuffer.cpp

		vklayoutmanager.cpp
		vklayoutbuilder.cpp
		vklayout.cpp

		vkprogrammanager.cpp
		vkgraphicsprogrambuilder.cpp
		vkgraphicsprogram.cpp

		vkimgui.cpp

		rendererimpl.cpp
)

target_include_directories(api_vk PRIVATE ${SOURCE})
target_include_directories(api_vk PRIVATE ${SOURCE}/renderer)

target_precompile_headers(api_vk PRIVATE stdafx.h)

find_package(fmt CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(volk CONFIG REQUIRED)
find_package(vk-bootstrap CONFIG REQUIRED)
find_package(unofficial-vulkan-memory-allocator CONFIG REQUIRED)
find_package(SDL2 CONFIG REQUIRED)
find_package(magic_enum CONFIG REQUIRED)
target_link_libraries(api_vk PRIVATE volk::volk volk::volk_headers)
target_link_libraries(api_vk PRIVATE vk-bootstrap::vk-bootstrap)
target_link_libraries(api_vk PRIVATE unofficial::vulkan-memory-allocator::vulkan-memory-allocator)
target_link_libraries(api_vk PRIVATE fmt::fmt-header-only)
target_link_libraries(api_vk PRIVATE spdlog::spdlog_header_only)
target_link_libraries(api_vk
	PRIVATE
	$<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
	$<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
)
target_link_libraries(api_vk PRIVATE magic_enum::magic_enum)

target_compile_definitions(imgui PUBLIC IMGUI_IMPL_VULKAN_NO_PROTOTYPES)
target_include_directories(api_vk PRIVATE ${THIRDPARTY}/imgui)
target_link_libraries(api_vk PRIVATE imgui)

#target_compile_definitions(tracy PUBLIC TRACY_ENABLE)
target_include_directories(api_vk PRIVATE ${THIRDPARTY}/tracy)
target_link_libraries(api_vk PRIVATE tracy)

target_compile_options(api_vk PRIVATE
	$<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
	$<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Werror>
)