file(GLOB_RECURSE SHADER_FILES
    shaders/*.vert
    shaders/*.frag
    shaders/*.glsl
)

foreach(SHADER ${SHADER_FILES})
    get_filename_component(FILENAME ${SHADER} NAME)
    get_filename_component(FILENAME_WE ${SHADER} NAME_WE)

    add_custom_command(
        OUTPUT ${OUTPUT}/assets/shaders/${FILENAME_WE}.spv
        COMMAND ${Vulkan_GLSLC_EXECUTABLE} ${SHADER} -o ${OUTPUT}/assets/shaders/${FILENAME_WE}.spv
        DEPENDS ${SHADER}
        COMMENT "Compiling Shadefile: ${FILENAME}"
    )

    list(APPEND COMPILED_SHADERS ${OUTPUT}/assets/shaders/${FILENAME_WE}.spv)
endforeach()

add_custom_target(ShaderCompile ALL DEPENDS ${COMPILED_SHADERS})
add_dependencies(${PROJECT} ShaderCompile)