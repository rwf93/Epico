file(GLOB SHADER_FILES
    shaders/*.vert
    shaders/*.frag
)

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

file(GLOB FONT_FILES fonts/*.ttf)
file(GLOB MODEL_FILES models/*.obj models/*.gltf models/*.gltb)
file(GLOB TEXTURE_FILES textures/*.ktx)

macro(asset_link ASSET_FILES DIRNAME)
    foreach(FILE ${${ASSET_FILES}})
        get_filename_component(FILENAME ${FILE} NAME)
        set(LOCATION ${DIRNAME}/${FILENAME})

        add_custom_command(
            OUTPUT ${LOCATION}
            COMMAND ${CMAKE_COMMAND} -E copy ${FILE} ${LOCATION}
            DEPENDS ${FILE}
            COMMENT "Linking Files: ${FILENAME}"
        )

        list(APPEND LINKED_ASSETS ${LOCATION})
    endforeach()
endmacro()

asset_link(FONT_FILES ${OUTPUT}/assets/fonts)
asset_link(MODEL_FILES ${OUTPUT}/assets/models)
asset_link(TEXTURE_FILES ${OUTPUT}/assets/textures)

add_custom_target(AssetLink ALL DEPENDS ${LINKED_ASSETS})
add_dependencies(${PROJECT} AssetLink)