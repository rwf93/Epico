#include <platform/platform.h>
#include <renderer/abstractrenderer.h>

#include <spdlog/spdlog.h>
#include <SDL2/SDL.h>

#include "appcontext.h"

int main(int argc, char *argv[]) {
    UNUSED(argc);
    UNUSED(argv);

    AppContext context;

    if(SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        spdlog::error("Couldn't init SDL: {}", SDL_GetError());
        return 0;
    };

    auto renderer = get_factory<AbstractRenderer*>("VulkanRenderer", &context);
    if(!renderer.good) {
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
