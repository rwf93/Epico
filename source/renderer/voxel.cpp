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

	for(int vx = 0; vx < MAX_WIDTH; vx++) {
		for(int vy = 0; vy < MAX_HEIGHT; vy++) {
			for(int vz = 0; vz < MAX_WIDTH; vz++) {
				voxels[vx][vy][vz].active = true;
				voxels[vx][vy][vz].type = VOXEL_AIR;
			}
		}
	}
}

void Chunk::build_mesh() {
	uint32_t vertex_count = 0;

	for(int d = 0; d < 3; d++) {
		int i = 0, j = 0, k = 0;
		int l = 0, w = 0, h = 0;

		int u = (d+1) % 3;
		int v = (d+2) % 3;

		int x[3] = { 0, 0, 0 };
		int q[3] = { 0, 0, 0 };

		BMask mask[MAX_WIDTH * MAX_HEIGHT] = {};
		q[d] = 1;

		// computes the mask.
		for(x[d] = -1; x[d] < MAX_WIDTH;) {
			int n = 0;
			for(x[v] = 0; x[v] < MAX_HEIGHT; x[v]++) {
				for(x[u] = 0; x[u] < MAX_WIDTH; x[u]++) {
					VoxelType current = get_type(x[0], x[1], x[2]);
					VoxelType compare = get_type(x[0] + q[0], x[1] + q[1], x[2] + q[2]);

					bool current_opaque = current != VoxelType::VOXEL_AIR;
					bool compare_opaque = compare != VoxelType::VOXEL_AIR;

					if(current_opaque == compare_opaque) {
						mask[n++] = { VoxelType::VOXEL_NULL, 0 };
					} else if(current_opaque) {
						mask[n++] = { current, 1 };
					} else {
						mask[n++] = { compare, -1 };
					}
					//mask[n++] = { current_opaque == compare_opaque ? compare : current, current_opaque != compare_opaque };
				}
			}

			x[d]++;
			n = 0;

			for(j = 0; j < MAX_HEIGHT; j++) {
				for(i = 0; i < MAX_WIDTH;) {
					if(mask[n].index != 0) {
						BMask current_mask = mask[n];
						for(w = 0; i + w < MAX_WIDTH && compare_mask(mask[n + w], current_mask); w++) {}

						bool done = false;
						for(h = 1; j + h < MAX_HEIGHT; h++) {
							for(k = 0; k < w; k++) {
								if(!compare_mask(mask[n + k + h * MAX_WIDTH], current_mask)) {
									done = true;
									break;
								}
							}

							if(done) break;
						}

						x[u] = i;
						x[v] = j;

						int du[3] = { 0, 0, 0 };
						du[u] = w;

						int dv[3] = { 0, 0, 0 };
						dv[v] = h;

						chunk_mesh.verticies.push_back( { { x[0], x[1], x[2] }, glm::vec3(1, 0, 0) } );
						chunk_mesh.verticies.push_back( { { x[0]+du[0], x[1]+du[1], x[2]+du[2] }, glm::vec3(0, 1, 0) } );
						chunk_mesh.verticies.push_back( { { x[0]+du[0]+dv[0], x[1]+du[1]+dv[1], x[2]+du[2]+dv[2] }, glm::vec3(0, 0, 1) } );
						chunk_mesh.verticies.push_back( { { x[0]+dv[0], x[1]+dv[1], x[2]+dv[2] }, glm::vec3(1, 1, 1) } );

						chunk_mesh.indicies.push_back(vertex_count);
						chunk_mesh.indicies.push_back(vertex_count + 1);
						chunk_mesh.indicies.push_back(vertex_count + 2);
						chunk_mesh.indicies.push_back(vertex_count + 2);
						chunk_mesh.indicies.push_back(vertex_count + 3);
						chunk_mesh.indicies.push_back(vertex_count);

						vertex_count += 4;

						for(l = 0; l < h; l++)
							for(k = 0; k < w; k++)
								mask[n + k + l * MAX_WIDTH] = { VoxelType::VOXEL_NULL, 0 };

						i += w;
						n += w;
					} else {
						i++;
						n++;
					}
				}
			}
		}
	}
}

bool Chunk::is_active(int x, int y, int z) {
	if(x >= MAX_WIDTH  || x < 0) return false;
	if(y >= MAX_HEIGHT || y < 0) return false;
	if(z >= MAX_WIDTH  || z < 0) return false;

	return voxels.at(x).at(y).at(z).active;
}

VoxelType Chunk::get_type(int x, int y, int z) {
	if(x >= MAX_WIDTH  || x < 0) return VoxelType::VOXEL_AIR;
	if(y >= MAX_HEIGHT || y < 0) return VoxelType::VOXEL_AIR;
	if(z >= MAX_WIDTH  || z < 0) return VoxelType::VOXEL_AIR;

	return voxels.at(x).at(y).at(z).type;
}

void Chunk::add_quad(std::vector<glm::vec3> &face, glm::vec3 color) {
		chunk_mesh.verticies.push_back({ face[0], color });
		chunk_mesh.verticies.push_back({ face[1], color });
		chunk_mesh.verticies.push_back({ face[2], color });
		chunk_mesh.verticies.push_back({ face[3], color });
}