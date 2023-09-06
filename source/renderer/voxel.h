#pragma once

#include "primitives.h"

namespace render {

enum VoxelType {
    VOXEL_AIR = 0,
    VOXEL_DIRT,
    VOXEL_STONE,

    VOXEL_MAXTYPE,
};

class Voxel {
public:
    bool active;
    VoxelType type;

    // yes, this is indeed abuse.
    static void load_model();
    static const uint32_t MODEL_OFFSET = 2;
    inline static std::vector<EVertex> block_model;
};

class Chunk {
public:
    Chunk(int x, int y);
    void build_mesh(VmaAllocator allocator, VkCommandBuffer command);

public:
    static const uint32_t MAX_HEIGHT = 128;
    static const uint32_t MAX_WIDTH = 16;

    template <unsigned I, unsigned J, unsigned K>
    using VoxelArray = std::array<std::array<std::array<Voxel, K>, J>, I>;
    VoxelArray<MAX_HEIGHT, MAX_WIDTH, MAX_WIDTH> voxels = {};

    EMesh chunk_mesh = {};

    int chunk_x = 0; int chunk_y = 0;
};

}