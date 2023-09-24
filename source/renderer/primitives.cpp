#include <vk_mem_alloc.h>

#include "info.h"
#include "tools.h"
#include "primitives.h"

using namespace render;

std::vector<VkVertexInputBindingDescription> EVertex::get_binding_descriptions() {
	std::vector<VkVertexInputBindingDescription> binding_descriptions = {};

	binding_descriptions.push_back({
		.binding = 0,
		.stride = sizeof(EVertex),
		.inputRate = VK_VERTEX_INPUT_RATE_VERTEX
	});

	return binding_descriptions;
}

std::vector<VkVertexInputAttributeDescription> EVertex::get_attribute_descriptions() {
	std::vector<VkVertexInputAttributeDescription> attribute_descriptions = {};

	// set the position attribute
	attribute_descriptions.push_back({
		.location = 0,
		.binding = 0,
		.format = VK_FORMAT_R32G32B32_SFLOAT,
		.offset = offsetof(EVertex, pos)
	});

	// set the color attribute
	attribute_descriptions.push_back({
		.location = 1,
		.binding = 0,
		.format = VK_FORMAT_R32G32B32_SFLOAT,
		.offset = offsetof(EVertex, color)
	});

	attribute_descriptions.push_back({
		.location = 2,
		.binding = 0,
		.format = VK_FORMAT_R32G32B32_SFLOAT,
		.offset = offsetof(EVertex, normal)
	});

	return attribute_descriptions;
}

VkResult EBuffer::allocate(VmaAllocator vma_allocator,
							VmaAllocationCreateInfo *create_info,
							VkBufferCreateInfo *buffer_info,
							VmaAllocationInfo *allocation_info
) {
	assert(vma_allocator != VK_NULL_HANDLE);
	this->allocator = vma_allocator;

	return vmaCreateBuffer(allocator, buffer_info, create_info, &buffer, &allocation, allocation_info);
}

void EBuffer::destroy() {
	vmaDestroyBuffer(allocator, buffer, allocation);
}

void EMesh::allocate(VmaAllocator vma_allocator) {
	assert(vma_allocator != VK_NULL_HANDLE);
	this->allocator = vma_allocator;

	VmaAllocationInfo alloc_info = {};
	auto staging_allocate_info = info::allocation_create_info(VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT, 0, VMA_MEMORY_USAGE_CPU_ONLY);
	auto staging_buffer_info = info::buffer_create_info(verticies.size() * sizeof(EVertex), VK_BUFFER_USAGE_TRANSFER_SRC_BIT);

	auto allocate_info = info::allocation_create_info(0, 0, VMA_MEMORY_USAGE_GPU_ONLY);

	staging_vertex_buffer.allocate(allocator, &staging_allocate_info, &staging_buffer_info, &alloc_info);
    memcpy(alloc_info.pMappedData, verticies.data(), verticies.size() * sizeof(EVertex));

  	staging_index_buffer.allocate(allocator, &staging_allocate_info, &staging_buffer_info, &alloc_info);
    memcpy(alloc_info.pMappedData, indicies.data(), indicies.size() * sizeof(uint32_t));

	{
		auto buffer_info = info::buffer_create_info(verticies.size() * sizeof(EVertex), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
		vertex_buffer.allocate(allocator, &allocate_info, &buffer_info);
	}

	{
		auto buffer_info = info::buffer_create_info(indicies.size() * sizeof(uint32_t), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
		index_buffer.allocate(allocator, &allocate_info, &buffer_info);
	}
}

void EMesh::send_to_gpu(VkCommandBuffer command) {
	VkBufferCopy vertex_copy;
	vertex_copy.dstOffset = 0;
	vertex_copy.srcOffset = 0;
	vertex_copy.size = verticies.size() * sizeof(EVertex);

	VkBufferCopy index_copy;
	index_copy.dstOffset = 0;
	index_copy.srcOffset = 0;
	index_copy.size = indicies.size() * sizeof(uint32_t);

	vmaFlushAllocation(allocator, staging_vertex_buffer, 0, VK_WHOLE_SIZE);
	vmaFlushAllocation(allocator, staging_index_buffer, 0, VK_WHOLE_SIZE);

	vkCmdCopyBuffer(command, staging_vertex_buffer, vertex_buffer, 1, &vertex_copy);
	vkCmdCopyBuffer(command, staging_index_buffer, index_buffer, 1, &index_copy);
}

void EMesh::cleanup_after_send() {
	staging_index_buffer.destroy();
	staging_vertex_buffer.destroy();

	// clears useless vertex data
	vertex_count = static_cast<uint32_t>(verticies.size());
	index_count = static_cast<uint32_t>(indicies.size());
	verticies.clear();
	indicies.clear();
}

void EMesh::destroy() {
	index_buffer.destroy();
	vertex_buffer.destroy();
}
