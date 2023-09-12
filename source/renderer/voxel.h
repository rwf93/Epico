#pragma once

#include "primitives.h"

namespace render {

enum VoxelType {
    VOXEL_NULL = 0,
    VOXEL_AIR,
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
    void build_mesh();
    bool is_active(int x, int y, int z);
    VoxelType get_type(int x, int y, int z);

public:
    static const int MAX_HEIGHT = 128;
    static const int MAX_WIDTH = 16;

    template <unsigned I, unsigned J, unsigned K>
    using VoxelArray = std::array<std::array<std::array<Voxel, I>, J>, K>;
    VoxelArray<MAX_WIDTH, MAX_HEIGHT, MAX_WIDTH> voxels = {};

    EMesh chunk_mesh = {};

    int chunk_x = 0; int chunk_y = 0;

    struct BMask {
        VoxelType type;
        int index;
    };

private:
    void add_quad(std::vector<glm::vec3> &face, glm::vec3 color);
    bool compare_mask(BMask m1, BMask m2) { return m1.type == m2.type && m1.index == m2.index; }
};

}

template<>
struct fmt::formatter<render::VoxelType> : fmt::formatter<std::string>
{
	auto format(render::VoxelType my, format_context &ctx) const -> decltype(ctx.out())
	{
		return fmt::format_to(ctx.out(), "{}", magic_enum::enum_name(my));
	}
};

template<>
struct fmt::formatter<render::Chunk::BMask> : fmt::formatter<std::string>
{
	auto format(render::Chunk::BMask my, format_context &ctx) const -> decltype(ctx.out())
	{
		return fmt::format_to(ctx.out(), "{{ {}, {} }}", my.type, my.index);
	}
};