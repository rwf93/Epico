#pragma once

namespace render {

struct EVertex {
	glm::vec3 pos;
	glm::vec3 color;
	glm::vec3 normal;

	static std::vector<VkVertexInputBindingDescription> get_binding_descriptions();
	static std::vector<VkVertexInputAttributeDescription> get_attribute_descriptions();
};

struct EBuffer {
	VkBuffer buffer = VK_NULL_HANDLE;
	VmaAllocation allocation = VK_NULL_HANDLE;

	operator VmaAllocation() { return allocation; }
	operator VkBuffer() { return buffer; };
};

struct EImage {
	VkImage image = VK_NULL_HANDLE;
	VmaAllocation allocation = VK_NULL_HANDLE;

	operator VmaAllocation() { return allocation; }
	operator VkImage() { return image; };
};

struct ETexture {
	EImage image = {};
	VkImageView view = VK_NULL_HANDLE;

	operator VmaAllocation() { return image; }
	operator VkImage() { return image; };
	operator VkImageView() { return view; };
};

struct EMesh {
	void allocate(VmaAllocator allocator);
	void destroy(VmaAllocator allocator);

	void send_to_gpu(VmaAllocator allocator, VkCommandBuffer command);
	void cleanup_after_send(VmaAllocator allocator); // used to free after transfers

	std::vector<EVertex> verticies = {};
	std::vector<uint32_t> indicies = {};

	uint32_t vertex_count = 0;
	uint32_t index_count = 0;

	EBuffer staging_vertex_buffer = {};
	EBuffer staging_index_buffer = {};

	EBuffer vertex_buffer = {};
	EBuffer index_buffer = {};
};

// uniforms
struct EGlobalData {
	glm::mat4 view = {};
	glm::mat4 projection = {};
};

struct EObjectData {
	glm::mat4 model = {};
};

}