#include <vk_mem_alloc.h>
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

    {
        VkBufferCreateInfo buffer_info = {};
        buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        buffer_info.size = verticies.size() * sizeof(EVertex);
        buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

        VmaAllocationCreateInfo allocate_info = {};
        allocate_info.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
        allocate_info.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

        vmaCreateBuffer(allocator, &buffer_info, &allocate_info, &vertex_buffer.buffer, &vertex_buffer.allocation, &alloc_info);
        memcpy(alloc_info.pMappedData, verticies.data(), verticies.size() * sizeof(EVertex));
    }

    {
        VkBufferCreateInfo buffer_info = {};
        buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        buffer_info.size = indicies.size() * sizeof(uint16_t);
        buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

        VmaAllocationCreateInfo allocate_info = {};
        allocate_info.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
        allocate_info.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

        vmaCreateBuffer(allocator, &buffer_info, &allocate_info, &index_buffer.buffer, &index_buffer.allocation, &alloc_info);
        memcpy(alloc_info.pMappedData, indicies.data(), indicies.size() * sizeof(indicies));
    }
}

void EMesh::destroy(VmaAllocator allocator) {
    vmaDestroyBuffer(allocator, index_buffer.buffer, index_buffer.allocation);
    vmaDestroyBuffer(allocator, vertex_buffer.buffer, vertex_buffer.allocation);
}
