add_library(BlankRenderer SHARED
    rendererimpl.cpp
)

target_include_directories(BlankRenderer PRIVATE ${SOURCE})
target_include_directories(BlankRenderer PRIVATE ${SOURCE}/renderer)

find_package(fmt CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
target_link_libraries(BlankRenderer PRIVATE fmt::fmt-header-only)
target_link_libraries(BlankRenderer PRIVATE spdlog::spdlog_header_only)

target_compile_options(BlankRenderer PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
  $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Werror>
)