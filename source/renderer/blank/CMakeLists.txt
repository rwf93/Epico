add_library(renderer_blank SHARED
		rendererimpl.cpp
)

target_include_directories(renderer_blank PRIVATE ${SOURCE})
target_include_directories(renderer_blank PRIVATE ${SOURCE}/renderer)

find_package(fmt CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
target_link_libraries(renderer_blank PRIVATE fmt::fmt-header-only)
target_link_libraries(renderer_blank PRIVATE spdlog::spdlog_header_only)

target_compile_options(renderer_blank PRIVATE
	$<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
	$<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Werror>
)