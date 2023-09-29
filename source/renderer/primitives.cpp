#include <vk_mem_alloc.h>

#include "info.h"
#include "tools.h"
#include "primitives.h"

#include "renderer.h"

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
		.offset = offsetof(EVertex, position)
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

	attribute_descriptions.push_back({
		.location = 3,
		.binding = 0,
		.format = VK_FORMAT_R32G32_SFLOAT,
		.offset = offsetof(EVertex, texcoord)
	});

	return attribute_descriptions;
}