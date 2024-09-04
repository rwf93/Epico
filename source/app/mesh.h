#pragma once

class Mesh {
public:
    Mesh(Filesystem *filesystem, RenderAPI *api):
        filesystem(filesystem),
        api(api),
        vbo(api->create_buffer()),
        ibo(api->create_buffer()) {}

    void load_from_file(std::filesystem::path path);
    void load_from_array(std::span<Vertex> verticies);

    void process(aiMesh *mesh);

    void bind();
    void draw(uint32_t count = 1, uint32_t first_instance = 0);
private:
    Filesystem *filesystem;
    RenderAPI *api;

    BufferHandle vbo;
    BufferHandle ibo;
    uint32_t index_count = 0;
};