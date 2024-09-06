
#if defined(SHADER_CODE)
#define C(C, SHADER) SHADER

#define UNIFORM_BLOCK(LOCATION, STRUCTURE) \
    layout(binding = LOCATION) uniform STRUCTURE##Block { STRUCTURE data; } STRUCTURE##_block; STRUCTURE

#define STORAGE_BLOCK(LOCATION, STRUCTURE, NAME) \
    layout(binding = LOCATION) readonly buffer STRUCTURE##Block { STRUCTURE data[]; } NAME##_block;

#else
	#pragma once
	#define C(C, SHADER) C
#endif

struct Vertex {
	C(glm::vec3, vec3) position 	C(= glm::vec3(0,0,0),);
	C(glm::vec3, vec3) normal 		C(= glm::vec3(0,0,0),);
	C(glm::vec3, vec3) tangent 		C(= glm::vec3(0,0,0),);
	C(glm::vec2, vec2) uv 			C(= glm::vec3(0,0,0),);
};

struct SceneData {
	C(glm::mat4, mat4) view;
	C(glm::mat4, mat4) projection;
	C(glm::vec4, vec4) camera_position;
	C(glm::vec2, vec2) resolution;
	C(int32_t, int) gbuffer_selection;
	float time;
	float time_delta;
};

struct StorageData {
	C(glm::mat4, mat4) model;
	C(static const uint32_t MAX_OBJECTS = 1024;,)
};

struct LightData {
	C(glm::vec4, vec4) position;
	C(glm::vec3, vec3) color;
	float radius;
	C(static const uint32_t MAX_LIGHTS = 4;,)
};