file(GLOB SHADER_FILES
	shaders/*.vert
	shaders/*.frag
	shaders/*.tesc
	shaders/*.tese
	shaders/*.rchit
	shaders/*.rmiss
	shaders/*.rgen
)

macro(populate_build DIRNAME)
	add_custom_command(
		OUTPUT ${OUTPUT}/${DIRNAME}
		COMMAND ${CMAKE_COMMAND} -E make_directory ${OUTPUT}/${DIRNAME}
		COMMENT "Populating ${DIRNAME} folder"
	)

	list(APPEND POPULATED_FOLDERS ${OUTPUT}/${DIRNAME})
endmacro()

populate_build(assets)
populate_build(assets/shaders)

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

add_custom_target(asset_preprocess ALL DEPENDS
	${POPULATED_FOLDERS}
	${COMPILED_SHADERS}
)
add_dependencies(${PROJECT} asset_preprocess)
