#pragma once

#include <vk_mem_alloc.h>

#include "primitives.h"
#include "buffer.h"

namespace render {

class Renderer;

struct EMesh {
	void load_mesh(Renderer *renderer, const char *path);

	void allocate(Renderer *renderer);
	void destroy();

	void cleanup_after_send(); // used to free after transfers

	std::vector<EVertex> verticies = {};
	std::vector<uint32_t> indicies = {};

	uint32_t vertex_count = 0;
	uint32_t index_count = 0;

	EBuffer staging_vertex_buffer = {};
	EBuffer staging_index_buffer = {};

	EBuffer vertex_buffer = {};
	EBuffer index_buffer = {};

	VkBuffer &get_vertex_buffer() { return vertex_buffer.get_buffer(); }
	VkBuffer &get_index_buffer() { return index_buffer.get_buffer(); }

	Renderer *context = nullptr;
};

}