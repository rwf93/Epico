#include "renderdefs.h"
#include "mesh.h"

void Mesh::load_from_file(std::filesystem::path path) {
    std::vector<Vertex> verticies;
    std::vector<uint32_t> indicies;

    Assimp::Importer importer;
	const aiScene *scene = importer.ReadFile(
		filesystem->resolve_physical_dir(path).string().c_str(),
		aiProcess_FlipUVs
	);

	for(unsigned int i = 0; i < scene->mNumMeshes; i++) {
		aiMesh *mesh = scene->mMeshes[i];
		for(unsigned int j = 0; j < mesh->mNumFaces; j++) {
			aiFace &face = mesh->mFaces[j];
			for(unsigned int k = 0; k < face.mNumIndices; k++) {
				Vertex vertex = {};

				aiVector3D position = mesh->mVertices[face.mIndices[k]];
				aiVector3D normal = mesh->HasNormals() ? mesh->mNormals[face.mIndices[k]] : aiVector3D(1.0f);
				aiVector3d tangent = mesh->HasTangentsAndBitangents() ? mesh->mTangents[face.mIndices[k]] : aiVector3D(1.0f);
				aiVector3D texcoord = mesh->mTextureCoords[0][face.mIndices[k]];

				vertex.position = { position.x, position.y, position.z };
				vertex.normal = { normal.x, normal.y, normal.z };
				vertex.tangent = { tangent.x, tangent.y, tangent.z };
				vertex.uv = { texcoord.x, texcoord.y };

				verticies.push_back(vertex);
				indicies.push_back((uint32_t)indicies.size());
			}
		}
	}

    index_count = static_cast<uint32_t>(indicies.size());

    api->buffer_data(
        vbo,
        BufferType::VERTEX,
        sizeof(Vertex) * verticies.size(),
        verticies.data()
    );

    api->buffer_data(
        ibo,
        BufferType::INSTANCE,
        sizeof(uint32_t) * indicies.size(),
        indicies.data()
    );
}

void Mesh::bind() {
    api->bind_buffer(vbo, BindBufferType::VERTEX);
	api->bind_buffer(ibo, BindBufferType::INSTANCE);
}

void Mesh::draw(uint32_t instance) {
    bind();
    api->draw_instanced(index_count, 1, instance);
}