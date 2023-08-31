#pragma once

namespace render {

struct EVertex {
    glm::vec3 pos;
    glm::vec3 color;

    static VkVertexInputBindingDescription get_binding_description();
    static std::array<VkVertexInputAttributeDescription, 2> get_attribute_descriptions();
};

struct EBuffer {
    VkBuffer buffer;
    VmaAllocation allocation;

    operator VkBuffer() { return buffer; };
    operator VmaAllocation() { return allocation; };
};

struct EMesh {
    void allocate(VmaAllocator allocator);
    void destroy(VmaAllocator allocator);

    std::vector<EVertex> verticies = {};
    std::vector<uint16_t> indicies = {};

    EBuffer vertex_buffer = {};
    EBuffer index_buffer = {};
};

struct EUniformBufferObject {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 projection;
};

}