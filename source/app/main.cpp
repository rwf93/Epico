#include <factory.h>
#include <renderer/abstractrenderer.h>

#include <spdlog/spdlog.h>

int main(int argc, const char *argv[]) {
    auto renderer = create_vulkan_renderer();

    while(true) {
        renderer->begin();

        renderer->begin_pass();
        renderer->end_pass();

        renderer->end();
    }

    return 0;
}
