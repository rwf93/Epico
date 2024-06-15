#include <platform/platform.h>

#include <public/appcontext.h>
#include <public/abstractrenderer.h>
#include <public/abstractvfs.h>

#include <spdlog/spdlog.h>
#include <SDL2/SDL.h>

int main(int argc, char *argv[]) {
    UNUSED(argc);
    UNUSED(argv);

    AppContext context;

    if(SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        spdlog::error("Couldn't init SDL: {}", SDL_GetError());
        return 0;
    };

    auto filesystem = get_factory<AbstractVFS*>("filesystem_std");
    if(!filesystem.good) {
        spdlog::error("Couldn't load VFS");
        return 0;
    }

    filesystem->mount("assets/", "../assets/");
    filesystem->mount("assets/models/", "../assets/models/");
    filesystem->mount("assets/fonts/", "../assets/fonts/");
    filesystem->mount("assets/textures/", "../assets/textures/");
    filesystem->mount("shaders/", "./assets/shaders/");

    auto renderer = get_factory<AbstractRenderer*>("renderer_vk", &context);
    if(!renderer.good) {
        spdlog::error("Couldn't load renderer");
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
    filesystem.release();
    SDL_Quit();

    return 0;
}
