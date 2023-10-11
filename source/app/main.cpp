#include <factory.h>
#include <renderer/abstractrenderer.h>

#include <spdlog/spdlog.h>
#include <SDL2/SDL.h>

int main(int argc, char *args[]) {
    if(SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        spdlog::info("Couldn't init SDL: {}", SDL_GetError());
        return 0;
    };

    auto renderer = create_vulkan_renderer();

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

    SDL_Quit();

    return 0;
}
