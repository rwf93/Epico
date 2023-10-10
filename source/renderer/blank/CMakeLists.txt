add_library(${PROJECT}_BlankRenderer SHARED
    rendererimpl.cpp
)

target_include_directories(${PROJECT}_BlankRenderer PRIVATE ${SOURCE})
target_include_directories(${PROJECT}_BlankRenderer PRIVATE ${SOURCE}/renderer)

find_package(fmt CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
target_link_libraries(${PROJECT}_BlankRenderer PRIVATE fmt::fmt-header-only)
target_link_libraries(${PROJECT}_BlankRenderer PRIVATE spdlog::spdlog_header_only)