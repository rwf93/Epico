target("api_vk")
	set_kind("shared")
	add_files("*.cpp")
	add_defines("EAPI_EXPORT")
	set_languages("cxx20")
	add_includedirs("$(projectdir)/source/", "$(projectdir)/thirdparty/tracy/", "$(projectdir)/thirdparty/imgui/")
	add_deps("tracy", "imgui")
	add_packages(
		"fmt",
		"libsdl",
		"spdlog",
		"vulkan-headers",
		"volk",
		"spdlog",
		"vk-bootstrap",
		"vulkan-memory-allocator",
		"magic_enum"
	)
	add_rules("defaults_rule", { precompiled_header = path.absolute("./stdafx.h") })