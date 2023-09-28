#pragma once

#include <vk_mem_alloc.h>

namespace render {

class Renderer;

struct EVertex {
	glm::vec3 position = {};
	glm::vec3 color = {};
	glm::vec3 normal = {};
	glm::vec2 texcoord = {};

	static std::vector<VkVertexInputBindingDescription> get_binding_descriptions();
	static std::vector<VkVertexInputAttributeDescription> get_attribute_descriptions();
};

struct EBuffer {
	VkBuffer buffer = VK_NULL_HANDLE;
	VmaAllocator allocator = VK_NULL_HANDLE;
	VmaAllocation allocation = VK_NULL_HANDLE;

	operator VmaAllocation() { return allocation; }
	operator VkBuffer() { return buffer; };

	VkResult allocate(VmaAllocator vma_allocator, VkBufferCreateInfo *buffer_info, VmaAllocationCreateInfo *create_info, VmaAllocationInfo *allocation_info = nullptr);
	void destroy();
};

struct EImage {
	VkImage image = VK_NULL_HANDLE;
	VkImageView view = VK_NULL_HANDLE;
	VmaAllocation allocation = VK_NULL_HANDLE;

	VkResult create(int width, int height);
	void destroy();
};

struct ETexture {
	EImage image = {};
	EBuffer buffer = {};

	void load_ktx(VmaAllocator vma_allocator, const char *path, Renderer &renderer);
	void destroy();
};

// uniforms
struct EGlobalData {
	glm::mat4 view = {};
	glm::mat4 projection = {};
};

struct EObjectData {
	glm::mat4 model = {};
	int texture_index = 0;
};

}