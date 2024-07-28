add_rules("mode.debug", "mode.release")
set_defaultmode("debug")
set_warnings("allextra", "error")

add_requires(
	"fmt 11.0.1",
	"stb 2024.06.01",
	"spdlog v1.14.1",
	"vulkan-headers",
	"volk",
	"vk-bootstrap",
	"magic_enum",
	"vulkan-memory-allocator",
	"glm"
)

add_requires("glslang 1.3.283+0", {
	configs = {
		binaryonly = true
	}
})

add_requires("libsdl", {
	configs = {
		sdlmain = false,
		shared = true
	}
})

-- Hacks to get around the fact XMake is being really inflexible with some of the stuff I originally did in CMake.
rule("defaults_rule")
 	on_load(function(target)
		import("core.base.task")
        import("core.project.project")

		local precompiled_header = target:extraconf("rules", "defaults_rule", "precompiled_header") or nil
		if precompiled_header then
			--target:add("forceincludes", precompiled_header)
			target:set("pcxxheader", precompiled_header)
		end

		target:set("targetdir", "$(projectdir)/output/$(os)_$(arch)")
	end)

	on_install(function(target)
		import("core.base.task")
        import("core.project.project")
		import("target.action.install")

		install(target, {
			bindir = "./$(host)_$(arch)",
			libdir = "./$(host)_$(arch)",
			includedir = "./$(host)_$(arch)"
		})
	end)

	before_build(function(target)
		import("core.base.task")
        import("core.project.project")

		for name, package in pairs(target:pkgs()) do
			for _, file in ipairs(table.wrap(package:get("libfiles"))) do
				if file:endswith(".dll") or file:endswith(".so") or file:endswith(".dylib") then
					os.vcp(file, "$(projectdir)/output/$(os)_$(arch)")
				end
			end
		end
	end)

includes(
	"thirdparty",
	"assets",
	"source"
)