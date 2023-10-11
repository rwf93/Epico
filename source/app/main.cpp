#include <platform/platform.h>

#include <renderer/abstractrenderer.h>

#include <spdlog/spdlog.h>
#include <SDL2/SDL.h>

int main(int argc, char *args[]) {
    if(SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        spdlog::error("Couldn't init SDL: {}", SDL_GetError());
        return 0;
    };

    auto renderer = get_factory<AbstractRenderer*>("VulkanRenderer");
    if(!renderer.good) {
        spdlog::error("Could not load renderer binary...");
        return 0;
    }

    static bool quit = false;
    while(!quit) {
        SDL_Event event;
        while(SDL_PollEvent(&event)) {
            if(event.type == SDL_QUIT) quit = true;
        }

        renderer->begin();

        renderer->begin_pass();
        renderer->end_pass();

        renderer->end();
    }

    renderer.release();
    SDL_Quit();

    return 0;
}
