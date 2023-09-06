#include "globals.h"

#include <vk_mem_alloc.h>

#include "primitives.h"
#include "voxel.h"

using namespace render;

void Voxel::load_model() {
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	std::string warn;
	std::string err;

    tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, "./assets/block.obj", nullptr);

    for(const auto& shape: shapes) {
		for (const auto& index : shape.mesh.indices) {
			EVertex vertex = {};

			vertex.pos = {
				attrib.vertices[3 * index.vertex_index + 0],
				attrib.vertices[3 * index.vertex_index + 1],
				attrib.vertices[3 * index.vertex_index + 2]
			};

			vertex.color = {
				attrib.normals[3 * index.normal_index + 0],
				attrib.normals[3 * index.normal_index + 1],
				attrib.normals[3 * index.normal_index + 2]
			};

            block_model.push_back(vertex);
		}
	}
}

Chunk::Chunk(int x, int y) {
	this->chunk_x = x;
	this->chunk_y = y;
}

void Chunk::build_mesh(VmaAllocator allocator, VkCommandBuffer command) {
	UNUSED(allocator);
	UNUSED(command);

	std::vector<glm::vec3> FRONT_FACE = {
		{ -0.5f, -0.5f, -0.5f }, {  0.5f, -0.5f, -0.5f }, {  0.5f,  0.5f, -0.5f }, { -0.5f,  0.5f, -0.5f }
	};

	std::vector<glm::vec3> LEFT_FACE = {
		{ -0.5f, -0.5f,  0.5f }, { -0.5f, -0.5f, -0.5f }, { -0.5f,  0.5f, -0.5f }, { -0.5f,  0.5f,  0.5f }
	};

	std::vector<glm::vec3> RIGHT_FACE = {
		{  0.5f, -0.5f, -0.5f }, {  0.5f, -0.5f,  0.5f }, { 0.5f, 0.5f, 0.5f }, { 0.5f, 0.5f, -0.5f }
	};

	std::vector<glm::vec3> BACK_FACE = {
		{  0.5f, -0.5f,  0.5f }, { -0.5f, -0.5f,  0.5f }, { -0.5f,  0.5f,  0.5f }, {  0.5f,  0.5f,  0.5f }
	};

	std::vector<glm::vec3> TOP_FACE = {
		{ -0.5f,  0.5f, -0.5f }, {  0.5f,  0.5f, -0.5f }, {  0.5f,  0.5f,  0.5f }, { -0.5f,  0.5f,  0.5f }
	};

	std::vector<glm::vec3> BOTTOM_FACE = {
		{ -0.5f, -0.5f,  0.5f }, {  0.5f, -0.5f,  0.5f }, {  0.5f, -0.5f, -0.5f }, { -0.5f, -0.5f, -0.5f }
	};

    for(uint32_t y = 0; y < MAX_HEIGHT; y++) {
        for(uint32_t x = 0; x < MAX_WIDTH; x++) {
            for(uint32_t z = 0; z < MAX_WIDTH; z++) {
                Voxel &voxel = voxels[y][x][z];
				if(!voxel.active) continue;

				glm::vec3 translation(x, y, z);

				static std::map<VoxelType, glm::vec3> type_colors = {
					{VOXEL_AIR, glm::vec3(1, 1, 0)},
					{VOXEL_DIRT, glm::vec3(0, 1, 1)},
					{VOXEL_STONE, glm::vec3(1, 0, 1)},
					{VOXEL_MAXTYPE, glm::vec3(1, 1, 1)},
				};

				add_plane(translation, FRONT_FACE, type_colors[voxel.type]);
				add_plane(translation, LEFT_FACE, type_colors[voxel.type]);
				add_plane(translation, RIGHT_FACE, type_colors[voxel.type]);
				add_plane(translation, BACK_FACE, type_colors[voxel.type]);
				add_plane(translation, TOP_FACE, type_colors[voxel.type]);
				add_plane(translation, BOTTOM_FACE, type_colors[voxel.type]);
            }
        }
    }
}

void Chunk::add_plane(glm::vec3 translation, std::vector<glm::vec3> &face, glm::vec3 color) {
		chunk_mesh.verticies.push_back({ face[0] + translation, color });
		chunk_mesh.verticies.push_back({ face[1] + translation, color });
		chunk_mesh.verticies.push_back({ face[2] + translation, color });
		chunk_mesh.verticies.push_back({ face[3] + translation, color });

		chunk_mesh.indicies.push_back((uint32_t)chunk_mesh.verticies.size());
		chunk_mesh.indicies.push_back((uint32_t)chunk_mesh.verticies.size() + 1);
		chunk_mesh.indicies.push_back((uint32_t)chunk_mesh.verticies.size() + 2);
		chunk_mesh.indicies.push_back((uint32_t)chunk_mesh.verticies.size() + 2);
		chunk_mesh.indicies.push_back((uint32_t)chunk_mesh.verticies.size() + 3);
		chunk_mesh.indicies.push_back((uint32_t)chunk_mesh.verticies.size());
}