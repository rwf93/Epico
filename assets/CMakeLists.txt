file(GLOB_RECURSE SHADER_FILES
    shaders/*.vert
    shaders/*.frag
    shaders/*.glsl
)

message(${Vulkan_GLSLC_EXECUTABLE})

foreach(SHADER ${SHADER_FILES})
    get_filename_component(FILENAME ${SHADER} NAME)

    add_custom_command(
        OUTPUT ${OUTPUT}/assets/shaders/${FILENAME}.spv
        COMMAND ${Vulkan_GLSLC_EXECUTABLE} ${SHADER} -o ${OUTPUT}/assets/shaders/${FILENAME}.spv
        DEPENDS ${SHADER}
        COMMENT "Compiling Shaderfile: ${FILENAME}"
    )

    list(APPEND COMPILED_SHADERS ${OUTPUT}/assets/shaders/${FILENAME}.spv)
endforeach()

add_custom_target(ShaderCompile ALL DEPENDS ${COMPILED_SHADERS})
add_dependencies(${PROJECT} ShaderCompile)