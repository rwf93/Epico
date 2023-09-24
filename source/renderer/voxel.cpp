#include "globals.h"

#include <vk_mem_alloc.h>

#include "primitives.h"
#include "voxel.h"

using namespace render;

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
// this code sucks, you genuinely cannot blame me.
void Chunk::build_mesh() {
	/*
	for(int d = 0; d < 3; d++) {
		int i = 0, j = 0, k = 0;
		int l = 0, w = 0, h = 0;

		int u = (d+1) % 3;
		int v = (d+2) % 3;

		int x[3] = { 0, 0, 0 }; // chunk/block mask
		int q[3] = { 0, 0, 0 }; // axis mask

		BMask mask[MAX_WIDTH * MAX_HEIGHT] = {};
		q[d] = 1;

		// computes the mask.
		for(x[d] = -1; x[d] < MAX_WIDTH;) {
			int n = 0;
			for(x[v] = 0; x[v] < MAX_HEIGHT; x[v]++) {
				for(x[u] = 0; x[u] < MAX_WIDTH; x[u]++) {
					VoxelType current = get_type(x[0], x[1], x[2]);
					VoxelType compare = get_type(x[0] + q[0], x[1] + q[1], x[2] + q[2]);

					// if a Voxel is active. If a voxel isn't rendered (destroyed), then it is inactive
					// Voxel data won't be stored in the Voxel class, it's too much to handle for any given system.
					bool current_active = is_active(x[0], x[1], x[2]);
					bool compare_active = is_active(x[0] + q[0], x[1] + q[1], x[2] + q[2]);
					// TODO: handle other opaque blocks such as water and glass; once we get vulkan to render the alpha channel.
					bool current_opaque = (current != VoxelType::VOXEL_AIR) && current_active;
					bool compare_opaque = (compare != VoxelType::VOXEL_AIR) && compare_active;

					if(current_opaque == compare_opaque) {
						mask[n++] = { VoxelType::VOXEL_NULL, 0 };
					} else if(current_opaque) {
						mask[n++] = { current, 1 };
					} else {
						mask[n++] = { compare, -1 };
					}
				}
			}

			x[d]++;
			n = 0;
			// does some black magic wizardy with the mask.
			for(j = 0; j < MAX_HEIGHT; j++) {
				for(i = 0; i < MAX_WIDTH;) {
					if(mask[n].index != 0) {
						BMask &current_mask = mask[n];
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

						uint32_t count = static_cast<uint32_t>(chunk_mesh.verticies.size());

						// black magic wizardy insues
						chunk_mesh.verticies.push_back( { { x[0], x[1], x[2] }, glm::vec3(1, 0, 0) } );
						chunk_mesh.verticies.push_back( { { x[0]+du[0], x[1]+du[1], x[2]+du[2] }, glm::vec3(0, 1, 0) } );
						chunk_mesh.verticies.push_back( { { x[0]+du[0]+dv[0], x[1]+du[1]+dv[1], x[2]+du[2]+dv[2] }, glm::vec3(0, 0, 1) } );
						chunk_mesh.verticies.push_back( { { x[0]+dv[0], x[1]+dv[1], x[2]+dv[2] }, glm::vec3(1, 1, 1) } );

						chunk_mesh.indicies.push_back(count);
						chunk_mesh.indicies.push_back(count + 1);
						chunk_mesh.indicies.push_back(count + 2);
						chunk_mesh.indicies.push_back(count + 2);
						chunk_mesh.indicies.push_back(count + 3);
						chunk_mesh.indicies.push_back(count);

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
	}*/

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

	std::vector<glm::vec3> BOTTOM_FACE = {
		{ -0.5f,  0.5f, -0.5f }, {  0.5f,  0.5f, -0.5f }, {  0.5f,  0.5f,  0.5f }, { -0.5f,  0.5f,  0.5f }
	};

	std::vector<glm::vec3> TOP_FACE = {
		{ -0.5f, -0.5f,  0.5f }, {  0.5f, -0.5f,  0.5f }, {  0.5f, -0.5f, -0.5f }, { -0.5f, -0.5f, -0.5f }
	};

    for(uint32_t x = 0; x < MAX_WIDTH; x++) {
        for(uint32_t y = 0; y < MAX_HEIGHT; y++) {
            for(uint32_t z = 0; z < MAX_WIDTH; z++) {
                Voxel &voxel = voxels[x][y][z];
				if(!voxel.active) continue;
				if(is_void(voxel)) continue;

				glm::vec3 translation(x, y, z);

				static std::map<VoxelType, glm::vec3> type_colors = {
					{VOXEL_DIRT, glm::vec3(1, 1, 1)},
					{VOXEL_STONE, glm::vec3(0, 0, 0)},
				};

				add_quad(translation, FRONT_FACE, type_colors[voxel.type]);
				add_quad(translation, BACK_FACE, type_colors[voxel.type]);
				add_quad(translation, TOP_FACE, type_colors[voxel.type]);
				add_quad(translation, BOTTOM_FACE, type_colors[voxel.type]);
				add_quad(translation, LEFT_FACE, type_colors[voxel.type]);
				add_quad(translation, RIGHT_FACE, type_colors[voxel.type]);
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

void Chunk::add_quad(glm::vec3 translation, std::vector<glm::vec3> &face, glm::vec3 color) {
		uint32_t count = static_cast<uint32_t>(chunk_mesh.verticies.size());

		chunk_mesh.verticies.push_back({ face[0] + translation, color, {1, 1, 1}});
		chunk_mesh.verticies.push_back({ face[1] + translation, color, {1, 1, 1}});
		chunk_mesh.verticies.push_back({ face[2] + translation, color, {1, 1, 1}});
		chunk_mesh.verticies.push_back({ face[3] + translation, color, {1, 1, 1}});

		chunk_mesh.indicies.push_back(count);
		chunk_mesh.indicies.push_back(count + 1);
		chunk_mesh.indicies.push_back(count + 2);
		chunk_mesh.indicies.push_back(count + 2);
		chunk_mesh.indicies.push_back(count + 3);
		chunk_mesh.indicies.push_back(count);
}