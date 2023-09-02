#include <vk_mem_alloc.h>

#include "info.h"
#include "primitives.h"

using namespace render;

VkVertexInputBindingDescription EVertex::get_binding_description() {
    VkVertexInputBindingDescription binding_description = {};

    binding_description.binding = 0;
    binding_description.stride = sizeof(EVertex);
    binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    return binding_description;
}

std::array<VkVertexInputAttributeDescription, 2> EVertex::get_attribute_descriptions() {
    std::array<VkVertexInputAttributeDescription, 2> attribute_descriptions = {};

    // set the position attribute
    attribute_descriptions[0].binding = 0;
    attribute_descriptions[0].location = 0;
    attribute_descriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attribute_descriptions[0].offset = offsetof(EVertex, pos);

    // set the color attribute
    attribute_descriptions[1].binding = 0;
    attribute_descriptions[1].location = 1;
    attribute_descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attribute_descriptions[1].offset = offsetof(EVertex, color);

    return attribute_descriptions;
}

void EMesh::allocate(VmaAllocator allocator) {
    VmaAllocationInfo alloc_info = {};
    auto allocate_info = info::allocation_create_info(VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT);

    {
        auto buffer_info = info::buffer_create_info(verticies.size() * sizeof(EVertex), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

        vmaCreateBuffer(allocator, &buffer_info, &allocate_info, &vertex_buffer.buffer, &vertex_buffer.allocation, &alloc_info);
        memcpy(alloc_info.pMappedData, verticies.data(), verticies.size() * sizeof(EVertex));
    }

    {
        auto buffer_info = info::buffer_create_info(indicies.size() * sizeof(uint32_t), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

        vmaCreateBuffer(allocator, &buffer_info, &allocate_info, &index_buffer.buffer, &index_buffer.allocation, &alloc_info);
        memcpy(alloc_info.pMappedData, indicies.data(), indicies.size() * sizeof(uint32_t));
    }
}

void EMesh::destroy(VmaAllocator allocator) {
    vmaDestroyBuffer(allocator, index_buffer.buffer, index_buffer.allocation);
    vmaDestroyBuffer(allocator, vertex_buffer.buffer, vertex_buffer.allocation);
}
