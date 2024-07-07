add_library(tracy STATIC
    TracyClient.cpp
)

# GCC causes issues without -fPIC on linux...
target_compile_options(tracy PRIVATE $<$<PLATFORM_ID:Linux>:-fPIC>)