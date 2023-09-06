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

	int idx = 0;
	std::vector<glm::vec3> FRONT_FACE = {
		Voxel::block_model[idx++].pos, Voxel::block_model[idx++].pos,
		Voxel::block_model[idx++].pos, Voxel::block_model[idx++].pos
	};

	std::vector<glm::vec3> LEFT_FACE = {
		Voxel::block_model[idx++].pos, Voxel::block_model[idx++].pos,
		Voxel::block_model[idx++].pos, Voxel::block_model[idx++].pos
	};

	std::vector<glm::vec3> RIGHT_FACE = {
		Voxel::block_model[idx++].pos, Voxel::block_model[idx++].pos,
		Voxel::block_model[idx++].pos, Voxel::block_model[idx++].pos
	};

	std::vector<glm::vec3> BACK_FACE = {
		Voxel::block_model[idx++].pos, Voxel::block_model[idx++].pos,
		Voxel::block_model[idx++].pos, Voxel::block_model[idx++].pos
	};

	std::vector<glm::vec3> TOP_FACE = {
		Voxel::block_model[idx++].pos, Voxel::block_model[idx++].pos,
		Voxel::block_model[idx++].pos, Voxel::block_model[idx++].pos
	};

	std::vector<glm::vec3> BOTTOM_FACE = {
		Voxel::block_model[idx++].pos, Voxel::block_model[idx++].pos,
		Voxel::block_model[idx++].pos, Voxel::block_model[idx++].pos
	};

    for(uint32_t y = 0; y < MAX_HEIGHT; y++) {
        for(uint32_t x = 0; x < MAX_WIDTH; x++) {
            for(uint32_t z = 0; z < MAX_WIDTH; z++) {
                Voxel &voxel = voxels[y][x][z];
				if(!voxel.active) continue;

				glm::vec3 translation(x + Voxel::MODEL_OFFSET, y + Voxel::MODEL_OFFSET, z + Voxel::MODEL_OFFSET);

				chunk_mesh.verticies.push_back({ FRONT_FACE[0] + translation, glm::vec3(1, 1, 1) });
				chunk_mesh.verticies.push_back({ FRONT_FACE[1] + translation, glm::vec3(1, 1, 1) });
				chunk_mesh.verticies.push_back({ FRONT_FACE[2] + translation, glm::vec3(1, 1, 1) });
				chunk_mesh.verticies.push_back({ FRONT_FACE[3] + translation, glm::vec3(1, 1, 1) });

				chunk_mesh.verticies.push_back({ LEFT_FACE[0] + translation, glm::vec3(1, 1, 1) });
				chunk_mesh.verticies.push_back({ LEFT_FACE[1] + translation, glm::vec3(1, 1, 1) });
				chunk_mesh.verticies.push_back({ LEFT_FACE[2] + translation, glm::vec3(1, 1, 1) });
				chunk_mesh.verticies.push_back({ LEFT_FACE[3] + translation, glm::vec3(1, 1, 1) });

				chunk_mesh.verticies.push_back({ RIGHT_FACE[0] + translation, glm::vec3(1, 1, 1) });
				chunk_mesh.verticies.push_back({ RIGHT_FACE[1] + translation, glm::vec3(1, 1, 1) });
				chunk_mesh.verticies.push_back({ RIGHT_FACE[2] + translation, glm::vec3(1, 1, 1) });
				chunk_mesh.verticies.push_back({ RIGHT_FACE[3] + translation, glm::vec3(1, 1, 1) });

				chunk_mesh.verticies.push_back({ BACK_FACE[0] + translation, glm::vec3(1, 1, 1) });
				chunk_mesh.verticies.push_back({ BACK_FACE[1] + translation, glm::vec3(1, 1, 1) });
				chunk_mesh.verticies.push_back({ BACK_FACE[2] + translation, glm::vec3(1, 1, 1) });
				chunk_mesh.verticies.push_back({ BACK_FACE[3] + translation, glm::vec3(1, 1, 1) });

				chunk_mesh.verticies.push_back({ TOP_FACE[0] + translation, glm::vec3(1, 1, 1) });
				chunk_mesh.verticies.push_back({ TOP_FACE[1] + translation, glm::vec3(1, 1, 1) });
				chunk_mesh.verticies.push_back({ TOP_FACE[2] + translation, glm::vec3(1, 1, 1) });
				chunk_mesh.verticies.push_back({ TOP_FACE[3] + translation, glm::vec3(1, 1, 1) });

				chunk_mesh.verticies.push_back({ BOTTOM_FACE[0] + translation, glm::vec3(1, 1, 1) });
				chunk_mesh.verticies.push_back({ BOTTOM_FACE[1] + translation, glm::vec3(1, 1, 1) });
				chunk_mesh.verticies.push_back({ BOTTOM_FACE[2] + translation, glm::vec3(1, 1, 1) });
				chunk_mesh.verticies.push_back({ BOTTOM_FACE[3] + translation, glm::vec3(1, 1, 1) });

				chunk_mesh.indicies.push_back((uint32_t)chunk_mesh.indicies.size());
            }
        }
    }
}