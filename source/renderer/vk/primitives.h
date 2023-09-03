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
};

struct EImage {
	VkImage image;
	VkImageView view;
	VmaAllocation allocation;

	operator VkImage() { return image; };
	operator VkImageView() { return view; };
};

struct EMesh {
	void allocate(VmaAllocator allocator);
	void destroy(VmaAllocator allocator);

	void send_to_gpu(VmaAllocator allocation, VkCommandBuffer command);

	void cleanup_after_send(VmaAllocator allocator);

	std::vector<EVertex> verticies = {};
	std::vector<uint32_t> indicies = {};

	EBuffer staging_vertex_buffer = {};
	EBuffer staging_index_buffer = {};

	EBuffer vertex_buffer = {};
	EBuffer index_buffer = {};
};
// uniforms

struct ECameraData {
	glm::mat4 view;
	glm::mat4 projection;
};

struct EObjectData {
	glm::mat4 model;
};

}