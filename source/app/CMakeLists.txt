add_executable(${PROJECT}
    main.cpp
)

target_include_directories(${PROJECT} PRIVATE ${SOURCE})
target_link_libraries(${PROJECT} PRIVATE ${PROJECT}_BlankRenderer)
target_link_libraries(${PROJECT} PRIVATE ${PROJECT}_VulkanRenderer)

find_package(fmt CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
target_link_libraries(${PROJECT} PRIVATE fmt::fmt-header-only)
target_link_libraries(${PROJECT} PRIVATE spdlog::spdlog_header_only)