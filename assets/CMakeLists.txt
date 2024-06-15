file(GLOB SHADER_FILES
    shaders/*.vert
    shaders/*.frag
    shaders/*.tesc
    shaders/*.tese
    shaders/*.rchit
    shaders/*.rmiss
    shaders/*.rgen
)

foreach(SHADER ${SHADER_FILES})
    get_filename_component(FILENAME ${SHADER} NAME)

    add_custom_command(
        OUTPUT ${OUTPUT}/assets/shaders/${FILENAME}.spv
        COMMAND ${CMAKE_COMMAND} -E make_directory ${OUTPUT}/assets/shaders
        COMMAND ${Vulkan_GLSLC_EXECUTABLE} ${SHADER} -o ${OUTPUT}/assets/shaders/${FILENAME}.spv
        DEPENDS ${SHADER}
        COMMENT "Compiling Shaderfile: ${FILENAME}"
    )

    list(APPEND COMPILED_SHADERS ${OUTPUT}/assets/shaders/${FILENAME}.spv)
endforeach()

add_custom_target(shader_compile ALL DEPENDS ${COMPILED_SHADERS})
add_dependencies(${PROJECT} shader_compile)