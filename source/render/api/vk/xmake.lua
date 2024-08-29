option("tracing")
	set_default(false)
	set_showmenu(true)
	add_defines("TRACY_ENABLE")
	set_category("api_vk")
	set_description("Tracing through Tracy")

option("validation")
	set_default(false)
	add_defines("CONFIG_VALIDATION")
	set_showmenu(true)
	set_category("api_vk")
	set_description("Vulkan Validation Layers")

target("api_vk")
	set_kind("shared")
	add_files("*.cpp")
	add_defines("EAPI_EXPORT")
	set_languages("cxx23")
	add_includedirs("$(projectdir)/source/", "$(projectdir)/thirdparty/imgui/")
	add_deps("imgui")
	add_packages(
		"fmt",
		"libsdl",
		"spdlog",
		"vulkan-headers",
		"volk",
		"spdlog",
		"vk-bootstrap",
		"vulkan-memory-allocator",
		"magic_enum",
		"tracy"
	)
	add_rules("defaults_rule", { precompiled_header = path.absolute("./stdafx.h") })
	add_options("tracing", "validation")