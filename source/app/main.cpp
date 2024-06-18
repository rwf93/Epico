int main(int argc, char *argv[]) {
    UNUSED(argc);
    UNUSED(argv);

    AppContext context;
    context.width = 1280;
    context.height = 762;

    if(SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        spdlog::error("Couldn't init SDL: {}", SDL_GetError());
        return 0;
    };

    auto filesystem = get_factory<AbstractFilesystem*>("filesystem_std");
    if(!filesystem.good) {
        spdlog::error("Couldn't load VFS");
        return 0;
    }

    filesystem->mount("assets/", "../assets/");
    filesystem->mount("assets/models/", "../assets/models/");
    filesystem->mount("assets/fonts/", "../assets/fonts/");
    filesystem->mount("assets/textures/", "../assets/textures/");
    filesystem->mount("assets/shaders/", "./assets/shaders/");

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

            renderer->ui()->process_event(&event);
        }

        renderer->begin();

        renderer->clear(0, 1, 0, 0);

        renderer->end();
    }

    renderer.release();
    filesystem.release();
    SDL_Quit();

    return 0;
}
