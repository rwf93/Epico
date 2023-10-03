#include "info.h"
#include "mesh.h"

#include "renderer.h"

using namespace render;

void EMesh::load_mesh(Renderer *renderer, const char *path) {
	Assimp::Importer importer;
	const aiScene *scene = importer.ReadFile(path, 0);

	for(unsigned int i = 0; i < scene->mNumMeshes; i++) {
		aiMesh *mesh = scene->mMeshes[i];
		for(unsigned int j = 0; j < mesh->mNumFaces; j++) {
			aiFace &face = mesh->mFaces[j];
			for(int k = 0; k < 3; k++) {
				EVertex vertex = {};

				aiVector3D position = mesh->mVertices[face.mIndices[k]];
				aiVector3D normal = mesh->HasNormals() ? mesh->mNormals[face.mIndices[k]] : aiVector3D(1.0f);
				aiVector3D texcoord = mesh->mTextureCoords[0][face.mIndices[k]];

				vertex.position = { position.x, position.y, position.z };
				vertex.normal = { normal.x, normal.y, normal.z };
				vertex.texcoord = { texcoord.x, texcoord.y };
				vertex.color = { 0.912, 0.475, 0.289 };

				verticies.push_back(vertex);
				indicies.push_back((uint32_t)indicies.size());
			}
		}
	}

	allocate(renderer);
	vertex_buffer.stage(&staging_vertex_buffer, verticies.size() * sizeof(EVertex));
	index_buffer.stage(&staging_index_buffer, indicies.size() * sizeof(uint32_t));
	cleanup_after_send();
}

void EMesh::allocate(Renderer *renderer) {
	assert(renderer != VK_NULL_HANDLE);
	this->context = renderer;

	auto staging_allocate_info = info::allocation_create_info(VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT, 0, VMA_MEMORY_USAGE_CPU_ONLY);
	auto staging_buffer_info = info::buffer_create_info(verticies.size() * sizeof(EVertex), VK_BUFFER_USAGE_TRANSFER_SRC_BIT);

	auto allocate_info = info::allocation_create_info(0, 0, VMA_MEMORY_USAGE_GPU_ONLY);

	staging_vertex_buffer.allocate(context, &staging_buffer_info, &staging_allocate_info);
	memcpy(staging_vertex_buffer.get_info().pMappedData, verticies.data(), verticies.size() * sizeof(EVertex));

  	staging_index_buffer.allocate(context, &staging_buffer_info, &staging_allocate_info);
	memcpy(staging_index_buffer.get_info().pMappedData, indicies.data(), indicies.size() * sizeof(uint32_t));

	{
		auto buffer_info = info::buffer_create_info(verticies.size() * sizeof(EVertex), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
		vertex_buffer.allocate(context, &buffer_info, &allocate_info);
	}

	{
		auto buffer_info = info::buffer_create_info(indicies.size() * sizeof(uint32_t), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
		index_buffer.allocate(context, &buffer_info, &allocate_info);
	}
}

void EMesh::cleanup_after_send() {
	staging_index_buffer.destroy();
	staging_vertex_buffer.destroy();

	// clears useless vertex data
	vertex_count = static_cast<uint32_t>(verticies.size());
	index_count = static_cast<uint32_t>(indicies.size());
	verticies.clear();
	indicies.clear();
}

void EMesh::destroy() {
	index_buffer.destroy();
	vertex_buffer.destroy();
}
