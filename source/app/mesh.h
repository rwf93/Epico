#pragma once

class Mesh {
public:
    Mesh(AppContext *context, Filesystem *filesystem, RenderAPI *api):
        context(context),
        filesystem(filesystem),
        api(api),
        vbo(api->create_buffer()),
        ibo(api->create_buffer()) {}

    void load_from_file(std::filesystem::path path);
    void bind();
    void draw(uint32_t instance);
private:
    AppContext *context;
    Filesystem *filesystem;
    RenderAPI *api;
    BufferHandle vbo;
    BufferHandle ibo;
    uint32_t index_count = 0;
};