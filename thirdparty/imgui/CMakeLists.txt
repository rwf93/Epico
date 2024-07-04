find_package(SDL2 CONFIG REQUIRED)
find_package(Vulkan REQUIRED)

add_library(imgui STATIC
	imgui.cpp
	imgui_demo.cpp
	imgui_draw.cpp
	imgui_impl_sdl2.cpp
	imgui_impl_vulkan.cpp
	imgui_tables.cpp
	imgui_widgets.cpp
)

target_link_libraries(imgui
	PRIVATE
	$<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
	$<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
)

# GCC causes issues without -fPIC on linux...
target_compile_options(imgui PRIVATE $<$<PLATFORM_ID:Linux>:-fPIC>)

target_compile_definitions(imgui PUBLIC IMGUI_IMPL_VULKAN_NO_PROTOTYPES)
