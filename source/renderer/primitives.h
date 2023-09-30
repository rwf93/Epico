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

struct EImage {
	VkImage image = VK_NULL_HANDLE;
	VkImageView view = VK_NULL_HANDLE;
	VmaAllocation allocation = VK_NULL_HANDLE;
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