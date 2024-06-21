add_library(filesystem_std SHARED
	filesystemimpl.cpp
)

target_include_directories(filesystem_std PRIVATE ${SOURCE})
target_include_directories(filesystem_std PRIVATE ${SOURCE}/filesystem)

target_precompile_headers(filesystem_std PRIVATE stdafx.h)

find_package(fmt CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
target_link_libraries(filesystem_std PRIVATE fmt::fmt-header-only)
target_link_libraries(filesystem_std PRIVATE spdlog::spdlog_header_only)