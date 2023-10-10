#include <factory.h>
#include <renderer/abstractrenderer.h>

#include <spdlog/spdlog.h>

int main(int argc, const char *argv[]) {
    AbstractRenderer *renderer = create_vulkan_renderer();

    renderer->begin();
    renderer->end();

    delete renderer;

    return 0;
}
