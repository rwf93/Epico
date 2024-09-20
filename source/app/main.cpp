#include "renderdefs.h"
#include "camera.h"
#include "texture.h"

glm::mat4 calculate_model_matrix(glm::vec3 translation, glm::vec3 rotation, glm::vec3 scale) {
	glm::mat4 translation_matrix = glm::translate(glm::mat4(1.0f), translation);
	glm::mat4 rotation_matrix = glm::toMat4(glm::quat(rotation));
	glm::mat4 scale_matrix = glm::scale(glm::mat4(1.0f), scale);

	return translation_matrix * rotation_matrix * scale_matrix;
}

glm::vec2 calculate_billboard(glm::vec3 translation, glm::mat4 projection, glm::mat4 view, int w, int h) {
	glm::vec2 xy = {};

	glm::vec4 world_space = glm::vec4(translation, 1.0f);
	glm::vec4 screen_space = projection * view * world_space;
	if(screen_space.w < 0.01) { return glm::vec2(-1000, -1000); }
	screen_space /= screen_space.w;

	xy.x = (screen_space.x + 1.0f) * 0.5f * w;
	xy.y = (screen_space.y + 1.0f) * 0.5f * h;

	return xy;
}

class NewTexture {
	struct M {
		RenderAPI *api;
		TextureHandle texture;
		TextureViewHandle view;
		ImageFormat format;
		ImageFlags flags;
	} m;

	explicit NewTexture(M m) : m(std::move(m)) {}
public:
	static NewTexture create(
		RenderAPI *api,
		int width, int height,
		ImageFormat format = ImageFormat::R8G8B8A8_UNORM,
		ImageFlags flags = ImageFlags::SAMPLED,
		void *data = nullptr
	) {
		auto texture = api->create_texture();
		auto view = api->create_texture_view();

		api->texture(
			texture,
			ImageDimensions::IMAGE_2D,
			ImageSamples::SAMPLE_COUNT_1_BIT,
			format,
			flags,
			data,
			width, height
		);

		api->texture_view(
			view,
			texture,
			ImageViewDimensions::IMAGE_2D,
			format,
			0, 0
		);

		return NewTexture(M{
			.api = api,
			.texture = texture,
			.view = view,
			.format = format,
			.flags = flags
		});
	}

	void resize(int width, int height, void *data = nullptr) {
		m.api->texture(
			m.texture,
			ImageDimensions::IMAGE_2D,
			ImageSamples::SAMPLE_COUNT_1_BIT,
			m.format,
			m.flags,
			data,
			width, height
		);

		m.api->texture_view(
			m.view,
			m.texture,
			ImageViewDimensions::IMAGE_2D,
			m.format,
			0, 0
		);
	}

	operator TextureHandle() { return m.texture; }
	operator TextureViewHandle() { return m.view; }
};

class Mesh {
	struct M {
		RenderAPI *api;
		BufferHandle vbo;
		BufferHandle ibo;
		uint32_t index_count;
	} m;

	explicit Mesh(M m) : m(std::move(m)) {}
public:
	static Mesh create(
		RenderAPI *api,
		const aiMesh *mesh
	) {
		auto vbo = api->create_buffer();
		auto ibo = api->create_buffer();

		std::vector<Vertex> verticies = {};
		std::vector<uint32_t> indicies = {};

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
				indicies.push_back(static_cast<uint32_t>(indicies.size()));
			}
		}

		api->buffer(
			vbo,
			BufferType::VERTEX,
			sizeof(Vertex) * verticies.size(),
			verticies.data()
		);

		api->buffer(
			ibo,
			BufferType::INSTANCE,
			sizeof(uint32_t) * indicies.size(),
			indicies.data()
		);

		return Mesh(M{
			.api = api,
			.vbo = vbo,
			.ibo = ibo,
			.index_count = static_cast<uint32_t>(indicies.size()),
		});
	}

	void draw(uint32_t instance_count = 1, uint32_t first_instance = 0) {
		m.api->bind_buffer(m.vbo, BindBufferType::VERTEX);
		m.api->bind_buffer(m.ibo, BindBufferType::INSTANCE);
		m.api->draw_instanced(m.index_count, instance_count, first_instance);
	}
};

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

template<typename T, size_t Size>
class StorageBuffer {
	struct M {
		RenderAPI *api;
		BufferHandle handle;
		T *data;
	} m;

	explicit StorageBuffer(M m) : m(std::move(m)) {}
public:
	~StorageBuffer() {
	}

	static StorageBuffer create(RenderAPI *api) {
		auto handle = api->create_buffer();
		api->buffer(handle, BufferType::STORAGE, sizeof(T) * Size, nullptr);

		return StorageBuffer(M{
			.api = api,
			.handle = handle,
		});
	}

	void lock() {
		m.data = static_cast<T*>(m.api->map(m.handle));
	}

	void unlock() {
		m.api->unmap(m.handle);
		m.data = nullptr;
	}

	void flush() {
		m.api->flush(m.handle, 0, sizeof(T) * Size);
	}

	inline T *data() { return m.data; }
	inline T &at(size_t index) { return m.data[index]; }

	operator UniformBind() {
		return {
			.buffer = {
				.buffer = m.handle,
				.offset = 0,
				.range = sizeof(T) * Size
			},
			.type = UniformType::STORAGE
		};
	}
};

class Model {
	struct PerMeshData {
		Mesh mesh;
		glm::mat4 transform;
		unsigned int texture_id;
	};

	struct PerTextureData {
		NewTexture albedo;
		NewTexture normal;
	};

	struct M {
		RenderAPI *api;
		Filesystem *fs;
		std::vector<PerMeshData> meshes;
		std::vector<PerTextureData> textures;
		SamplerHandle texture_sampler;
	} m;

	explicit Model(M m) : m(std::move(m)) {}

	static glm::mat4 convert_matrix(const aiMatrix4x4 &matrix) {
		return {
			matrix.a1, matrix.b1, matrix.c1, matrix.d1,
			matrix.a2, matrix.b2, matrix.c2, matrix.d2,
			matrix.a3, matrix.b3, matrix.c3, matrix.d3,
			matrix.a4, matrix.b4, matrix.c4, matrix.d4
		};
	}

	static NewTexture load_texture(RenderAPI *api, Filesystem *fs, aiMaterial *material, aiTextureType texture_type) {
		aiString texture_file;
		material->GetTexture(texture_type, 0, &texture_file);

		auto path = fs->resolve_physical_dir(fmt::format("assets/textures/{}", texture_file.C_Str()));
		int width, height, nrchannels;
		unsigned char *image_data = stbi_load(path.string().c_str(), &width, &height, &nrchannels, 4);

		auto texture = image_data == nullptr
				? NewTexture::create(
					api, 1024, 1024
				)
				: NewTexture::create(
					api, width, height, ImageFormat::R8G8B8A8_UNORM, ImageFlags::SAMPLED, image_data
				);
		return texture;
	}

	static void process_node(
		RenderAPI *api,
		Filesystem *fs,
		const aiScene *scene,
		aiNode *node,
		std::vector<PerMeshData> &meshes,
		std::vector<PerTextureData> &textures,
		glm::mat4 parent_transform
	) {
		UNUSED(fs);
		UNUSED(textures);

		std::stack<std::pair<aiNode*, glm::mat4>> stack;
		stack.push({node, parent_transform});

		while(!stack.empty()) {
			auto [current_node, current_transform] = stack.top();
			stack.pop();

			auto local_transform = current_transform * convert_matrix(current_node->mTransformation);

			for (unsigned int i = 0; i < current_node->mNumMeshes; i++) {
				const aiMesh* mesh = scene->mMeshes[current_node->mMeshes[i]];

				meshes.push_back(PerMeshData{
					.mesh = Mesh::create(api, mesh),
					.transform = local_transform,
					.texture_id = mesh->mMaterialIndex,
				});
			}

			for (unsigned int i = 0; i < current_node->mNumChildren; i++) {
				stack.push({current_node->mChildren[i], local_transform});
			}
		}
	}

public:
	static Model create(
		RenderAPI *api,
		Filesystem *fs,
		std::filesystem::path model_file
	) {
		Assimp::Importer importer;
		const aiScene *scene = importer.ReadFile(
			fs->resolve_physical_dir(model_file).string(),
			aiProcess_Triangulate | aiProcess_FlipUVs
		);

		std::vector<PerMeshData> meshes = {};
		std::vector<PerTextureData> textures = {};

		auto sampler = api->create_sampler();
		api->sampler(
			sampler,
			SamplerAddressMode::CLAMP_BORDER,
			SamplerAddressMode::CLAMP_BORDER,
			SamplerAddressMode::CLAMP_BORDER
		);

		process_node(
			api,
			fs,
			scene,
			scene->mRootNode,
			meshes,
			textures,
			calculate_model_matrix(glm::vec3(0), glm::vec3(0), glm::vec3(1.0f))
		);

		return Model(M{
			.api = api,
			.fs = fs,
			.meshes = std::move(meshes),
			.textures = std::move(textures),
			.texture_sampler = sampler
		});
	}

	void draw(
		StorageBuffer<StorageData, StorageData::MAX_OBJECTS> &render_objects,
		uint32_t &object_index,
		std::vector<UniformBind> &binds,
		LayoutHandle geometry_layout
	) {
		UNUSED(binds);
		UNUSED(geometry_layout);

		m.api->begin_label(DebugLabel { .name = fmt::format("Model ({})", (void*)this).c_str(), .rgba = { 0.2f, 0.761f, 0.71f, 1.0f } });
		render_objects.lock();
		for(auto &data: m.meshes) {
			data.mesh.draw(1, object_index > 0 ? object_index : 0);
			render_objects.data()[object_index++].model = data.transform;
		}
		render_objects.unlock();
		m.api->end_label();
	}

	void draw() {
		for(auto &data: m.meshes)
			data.mesh.draw();
	}
};

struct RenderResources {
	SamplerHandle position_sampler = SamplerHandle::Invalid;
	SamplerHandle normal_sampler = SamplerHandle::Invalid;
	SamplerHandle albedo_sampler = SamplerHandle::Invalid;

	BufferHandle scene_buffer = BufferHandle::Invalid;

	NewTexture position;
	NewTexture normal;
	NewTexture albedo;
	NewTexture depth;
	NewTexture composition;

	SceneData scene;
	StorageBuffer<StorageData, StorageData::MAX_OBJECTS> storage;
	StorageBuffer<LightData, LightData::MAX_LIGHTS> lights;
};

void setup_resources(RenderAPI *api, RenderResources *resources);
void update_shader_buffers(RenderAPI *api, RenderResources *resources);

int main(int argc, char *argv[]) {
	UNUSED(argc);
	UNUSED(argv);

	AppContext context;
	context.argc = argc;
	context.argv = argv;
	context.width = 1280;
	context.height = 762;

	Camera camera = Camera::create(&context);

	if(SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		spdlog::error("Couldn't init SDL: {}", SDL_GetError());
		return 0;
	};

	auto filesystem = FactoryHandle<Filesystem, AppContext*>::create(
		"filesystem_std",
		&context,
		std::filesystem::weakly_canonical(argv[0]).parent_path().append("./")
	);
	auto render_api = FactoryHandle<RenderAPI, AppContext*>::create(
		"api_vk",
		&context,
		std::filesystem::weakly_canonical(argv[0]).parent_path().append("./")
	);

	filesystem->mount("assets/", "../../assets/");
	filesystem->mount("assets/models/", "../../assets/models/");
	filesystem->mount("assets/fonts/", "../assets/fonts/");
	filesystem->mount("assets/textures/", "../../assets/textures/");
	filesystem->mount("assets/shaders/", "../assets/shaders/");

	Texture armor_albedo_texture(render_api);
	ImageTexture armor_albedo_image(&armor_albedo_texture, filesystem);
	armor_albedo_image.upload_from_ktx("assets/textures/colormap_rgba.ktx");

	Texture armor_normal_texture(render_api);
	ImageTexture armor_normal_image(&armor_normal_texture, filesystem);
	armor_normal_image.upload_from_ktx("assets/textures/normalmap_rgba.ktx");

	Texture skybox_texture(render_api);
	ImageTexture skybox_image(&skybox_texture, filesystem);
	skybox_image.upload_from_ktx("assets/textures/skysphere_rgba.ktx");

	Texture monkey_normals(render_api);
	ImageTexture monkey_normals_image(&monkey_normals, filesystem);
	monkey_normals_image.upload_from_ktx("assets/textures/suzanne_normal.ktx");

	RenderResources resources = {
		.position = NewTexture::create(
			render_api,
			context.width,
			context.height,
			ImageFormat::R16G16B16A16_SFLOAT,
			ImageFlags::COLOR_ATTACHMENT | ImageFlags::SAMPLED
		),
		.normal = NewTexture::create(
			render_api,
			context.width,
			context.height,
			ImageFormat::R16G16B16A16_SFLOAT,
			ImageFlags::COLOR_ATTACHMENT | ImageFlags::SAMPLED
		),
		.albedo = NewTexture::create(
			render_api,
			context.width,
			context.height,
			ImageFormat::R8G8B8A8_UNORM,
			ImageFlags::COLOR_ATTACHMENT | ImageFlags::SAMPLED
		),
		.depth = NewTexture::create(
			render_api,
			context.width,
			context.height,
			ImageFormat::D32_SFLOAT,
			ImageFlags::DEPTH_ATTACHMENT
		),
		.composition = NewTexture::create(
			render_api,
			context.width,
			context.height,
			ImageFormat::R16G16B16A16_SFLOAT,
			ImageFlags::COLOR_ATTACHMENT
		),
		.storage = StorageBuffer<StorageData, StorageData::MAX_OBJECTS>::create(render_api),
		.lights = StorageBuffer<LightData, LightData::MAX_LIGHTS>::create(render_api),
	};
	setup_resources(render_api, &resources);
	render_api->on_resize([&](RenderAPI* renderer) {
		UNUSED(renderer);
		resources.position.resize(context.width, context.height);
		resources.normal.resize(context.width, context.height);
		resources.albedo.resize(context.width, context.height);
		resources.depth.resize(context.width, context.height);
		resources.composition.resize(context.width, context.height);
	});

	auto deferred_vertex = render_api->create_shader();
	auto deferred_fragment = render_api->create_shader();
	auto composition_vertex = render_api->create_shader();
	auto composition_fragment = render_api->create_shader();
	auto skybox_vertex = render_api->create_shader();
	auto skybox_fragment = render_api->create_shader();
	auto skybox_weird_fragment = render_api->create_shader();

	render_api->shader(
		deferred_vertex,
		ShaderStage::VERTEX,
		filesystem->read_file<char>("assets/shaders/deferred.vert.spv", true)
	);

	render_api->shader(
		deferred_fragment,
		ShaderStage::FRAGMENT,
		filesystem->read_file<char>("assets/shaders/deferred.frag.spv", true)
	);

	render_api->shader(
		composition_vertex,
		ShaderStage::VERTEX,
		filesystem->read_file<char>("assets/shaders/composition.vert.spv", true)
	);

	render_api->shader(
		composition_fragment,
		ShaderStage::FRAGMENT,
		filesystem->read_file<char>("assets/shaders/composition.frag.spv", true)
	);

	render_api->shader(
		skybox_vertex,
		ShaderStage::VERTEX,
		filesystem->read_file<char>("assets/shaders/skybox.vert.spv", true)
	);

	render_api->shader(
		skybox_fragment,
		ShaderStage::FRAGMENT,
		filesystem->read_file<char>("assets/shaders/skybox.frag.spv", true)
	);

	render_api->shader(
		skybox_weird_fragment,
		ShaderStage::FRAGMENT,
		filesystem->read_file<char>("assets/shaders/skybox_weird.frag.spv", true)
	);

	auto deferred_layout = render_api->create_layout()
		.add_uniform(ShaderStage::VERTEX, UniformType::BUFFER)
		.add_uniform(ShaderStage::VERTEX, UniformType::STORAGE)
		.add_uniform(ShaderStage::FRAGMENT, UniformType::TEXTURE)
		.add_uniform(ShaderStage::FRAGMENT, UniformType::TEXTURE)
		.build();

	auto composition_layout = render_api->create_layout()
		.add_uniform(ShaderStage::FRAGMENT, UniformType::BUFFER)
		.add_uniform(ShaderStage::FRAGMENT, UniformType::STORAGE)
		.add_uniform(ShaderStage::FRAGMENT, UniformType::TEXTURE)
		.add_uniform(ShaderStage::FRAGMENT, UniformType::TEXTURE)
		.add_uniform(ShaderStage::FRAGMENT, UniformType::TEXTURE)
		.build();

	auto skybox_layout = render_api->create_layout()
		.add_uniform(ShaderStage::VERTEX | ShaderStage::FRAGMENT, UniformType::BUFFER)
		.add_uniform(ShaderStage::FRAGMENT, UniformType::TEXTURE)
		.build();

	auto &deferred_builder = render_api->create_graphics_program()
		.add_attachment(ImageFormat::R16G16B16A16_SFLOAT) // Position
		.add_attachment(ImageFormat::R16G16B16A16_SFLOAT) // Normals
		.add_attachment(ImageFormat::R8G8B8A8_UNORM) 	  // Albedo
		.set_cull_face(CullFace::BACK)
		.set_front_face(FrontFace::COUNTER_CLOCKWISE)
		.set_depth_test(true, true, CompareOp::LESS_OR_EQUAL)
		.add_binding(sizeof(Vertex), BindingRate::VERTEX, 0)
		.add_attribute(offsetof(Vertex, position), AttributeType::VEC3F_SIGNED)
		.add_attribute(offsetof(Vertex, normal), AttributeType::VEC3F_SIGNED)
		.add_attribute(offsetof(Vertex, tangent), AttributeType::VEC3F_SIGNED)
		.add_attribute(offsetof(Vertex, uv), AttributeType::VEC2F_SIGNED)
		.set_layout(deferred_layout);

	auto deferred_program = deferred_builder
		.clear_stages()
		.add_stage(deferred_vertex)
		.add_stage(deferred_fragment)
		.build();

	auto deferred_test_program = deferred_builder
		.set_cull_face(CullFace::NONE)
		.set_polygon_mode(PolygonMode::LINE)
		.clear_stages()
		.add_stage(deferred_vertex)
		.add_stage(deferred_fragment)
		.build();

	auto composition_shader = render_api->create_graphics_program()
		.add_attachment(ImageFormat::R16G16B16A16_SFLOAT)
		.add_stage(composition_vertex)
		.add_stage(composition_fragment)
		.set_layout(composition_layout)
		.build();

	auto &skybox_builder = render_api->create_graphics_program()
		.add_attachment(ImageFormat::R16G16B16A16_SFLOAT)
		.set_cull_face(CullFace::FRONT)
		.set_front_face(FrontFace::COUNTER_CLOCKWISE)
		.set_depth_test(true, false, CompareOp::EQUAL)
		.add_binding(sizeof(Vertex), BindingRate::VERTEX)
		.add_attribute(offsetof(Vertex, position), AttributeType::VEC3F_SIGNED)
		.add_attribute(offsetof(Vertex, uv), AttributeType::VEC2F_SIGNED)
		.set_layout(skybox_layout);

	auto skybox_shader = skybox_builder
		.clear_stages()
		.add_stage(skybox_vertex)
		.add_stage(skybox_fragment)
		.build();

	auto skybox_weird_program = skybox_builder
		.clear_stages()
		.add_stage(skybox_vertex)
		.add_stage(skybox_weird_fragment)
		.build();

	auto sphere_model = Model::create(
		render_api,
		filesystem,
		"assets/models/sphere.glb"
	);

	auto sponza_model = Model::create(
		render_api,
		filesystem,
		"assets/models/sponza.glb"
	);

	auto armor_model = Model::create(
		render_api,
		filesystem,
		"assets/models/armor.gltf"
	);

	ImGui::SetCurrentContext(static_cast<ImGuiContext*>(render_api->ui()->get_context()));

	clock_t start_time = std::clock();

	static bool quit = false;
	static bool minimized = false;
	while(!quit) {
		clock_t current_time = std::clock();
		context.time_delta = static_cast<float>(current_time - start_time) / CLOCKS_PER_SEC;
		context.time = static_cast<float>(current_time) / CLOCKS_PER_SEC;

		SDL_Event event;
		while(SDL_PollEvent(&event)) {
			if(event.type == SDL_QUIT) quit = true;

			if(event.window.event == SDL_WINDOWEVENT_MINIMIZED)
				minimized = true;

			if(event.window.event == SDL_WINDOWEVENT_RESTORED)
				minimized = false;

			render_api->ui()->process_event(&event);
			camera.process_event(&event);
		}

		if(minimized) {
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			continue;
		}

		camera.update();

		resources.scene.view = camera.get_view_matrix();
		resources.scene.projection = glm::perspective(glm::radians(70.f), static_cast<float>(context.width) / static_cast<float>(context.height), 0.1f, 1000.0f);
		resources.scene.projection[1][1] *= -1;
		resources.scene.resolution = glm::vec2(context.width, context.height);
		resources.scene.time = context.time;
		resources.scene.time_delta = context.time_delta;
		resources.scene.camera_position = glm::vec4(camera.get_position(), 0.0f) * glm::vec4(-1.0f, 1.0f, -1.0f, 1.0f);

		render_api->begin();
		{
			update_shader_buffers(render_api, &resources);


			render_api->clear(0, 0, 0, 1);
			render_api->viewport(static_cast<float>(context.width), static_cast<float>(context.height));
			render_api->scissor(context.width, context.height);

			// Offscreen/Deferred rendering (first pass)
			static std::vector<SubpassAttachment> deferred_attachments = {
				{
					.texture = resources.position,
					.view = resources.position,
					.type = AttachmentType::COLOR,
					.clear = (SubpassAttachment::ClearValue{ .rgba = { 0.0f, 0.0f, 0.0f, 0.0f } })
				},
				{
					.texture = resources.normal,
					.view = resources.normal,
					.type = AttachmentType::COLOR,
					.clear = (SubpassAttachment::ClearValue{ .rgba = { 0.0f, 0.0f, 0.0f, 0.0f } })
				},
				{
					.texture = resources.albedo,
					.view = resources.albedo,
					.type = AttachmentType::COLOR,
					.clear = (SubpassAttachment::ClearValue{ .rgba = { 0.0f, 0.0f, 0.0f, 0.0f } })
				},
				{
					.texture = resources.depth,
					.view = resources.depth,
					.type = AttachmentType::DEPTH,
					.clear = (SubpassAttachment::ClearValue{ .rgba = { 1.0f }, .depth = 0.0f, .stencil = 0 })
				}
			};

			static std::vector<UniformBind> deferred_binds = {
				{
					.buffer = {
						.buffer = resources.scene_buffer,
						.range = sizeof(SceneData)
					},
					.type = UniformType::BUFFER,
				},
				resources.storage,
				armor_albedo_texture,
				armor_normal_texture
			};

			static bool testing = false;
			render_api->begin_label(DebugLabel { .name = "Geometry Pass", .rgba = { 0.22f, 0.761f, 0.294f, 1.0f } });
			render_api->begin_pass(deferred_attachments);
				render_api->bind_program(testing ? deferred_test_program : deferred_program);

				render_api->bind_uniform(deferred_layout, deferred_binds);

				uint32_t storage_index = 0;
				armor_model.draw(resources.storage, storage_index, deferred_binds, deferred_layout);
			render_api->end_pass();
			render_api->end_label();

			static std::vector<SubpassAttachment> composition_attachments = {
				{
					.texture = resources.composition,
					.view = resources.composition,
					.type = AttachmentType::COLOR,
					.clear = (SubpassAttachment::ClearValue{ .rgba = { 0.0f, 0.0f, 0.0f, 0.0f } })
				},
				{
					.texture = resources.position,
					.view = resources.position,
					.type = AttachmentType::SHADER
				},
				{
					.texture = resources.albedo,
					.view = resources.albedo,
					.type = AttachmentType::SHADER
				},
				{
					.texture = resources.normal,
					.view = resources.normal,
					.type = AttachmentType::SHADER
				},
			};

			static std::vector<UniformBind> composition_binds = {
				{
					.buffer = {
						.buffer = resources.scene_buffer,
						.offset = 0,
						.range = sizeof(SceneData)
					},
					.type = UniformType::BUFFER
				},
				resources.lights,
				{
					.texture = {
						.view = resources.position,
						.sampler = resources.position_sampler
					},
					.type = UniformType::TEXTURE,
				},
				{
					.texture = {
						.view = resources.normal,
						.sampler = resources.normal_sampler
					},
					.type = UniformType::TEXTURE,
				},
				{
					.texture = {
						.view = resources.albedo,
						.sampler = resources.albedo_sampler
					},
					.type = UniformType::TEXTURE,
				},
			};

			render_api->begin_label(DebugLabel { .name = "Composition Pass", .rgba = { 0.82f, 0.82f, 0.278f, 1.0f } });
			render_api->begin_pass(composition_attachments);
				render_api->bind_uniform(composition_layout, composition_binds);
				render_api->bind_program(composition_shader);
				render_api->draw(3, 1);
			render_api->end_pass();
			render_api->end_label();

			static std::vector<SubpassAttachment> skybox_attachments = {
				{
					.texture = resources.composition,
					.view = resources.composition,
					.type = AttachmentType::COLOR,
				},
				{
					.texture = resources.depth,
					.view = resources.depth,
					.type = AttachmentType::DEPTH,
				}
			};

			static std::vector<UniformBind> skybox_uniforms = {
				{
					.buffer = {
						.buffer = resources.scene_buffer,
						.range = sizeof(SceneData)
					},
					.type = UniformType::BUFFER,
				},
				skybox_texture
			};

			// Skybox pass
			render_api->begin_label(DebugLabel { .name = "Skybox Pass", .rgba = { 0.224f, 0.804f, 0.902f, 1.0f } });
			render_api->begin_pass(skybox_attachments);
				render_api->bind_uniform(skybox_layout, skybox_uniforms);
				render_api->bind_program(testing ? skybox_weird_program : skybox_shader);
				sphere_model.draw();
			render_api->end_pass();
			render_api->end_label();

			render_api->show_image(resources.composition);

			render_api->begin_label(DebugLabel { .name = "ImGUI", .rgba = { 0.988f, 0.443f, 0.553f, 1.0f } });
			render_api->ui()->begin();
				ImGui::NewFrame();

				ImGui::Begin("Selection");
				const char *items[] = {
					"Position",
					"Normals",
					"Albedo",
					"Specular",
					"Composition"
				};
				ImGui::Combo("G-Buffer", &resources.scene.gbuffer_selection, items, IM_ARRAYSIZE(items));
/*
				resources.lights.lock();
				for(uint32_t i = 0; i < 4; i++) {
					LightData &light = resources.lights.at(i);
					ImGui::SliderFloat3(fmt::format("Light {} Position", i).c_str(), glm::value_ptr(light.position), -100, 100);
					ImGui::ColorEdit3(fmt::format("Light {} Color", i).c_str(), glm::value_ptr(light.color));
					ImGui::SliderFloat(fmt::format("Light {} Radius", i).c_str(), &light.radius, 0.5, 100);
					ImGui::Separator();

					auto dist = glm::length(camera.get_position() - light.position.xyz);

					auto pos = calculate_billboard(
						light.position,
						resources.scene.projection,
						resources.scene.view,
						context.width,
						context.height
					);
					ImGui::GetBackgroundDrawList()->AddCircleFilled(
						ImVec2(pos.x, pos.y),
						light.radius * 4 / dist,
						ImColor(
							light.color.x,
							light.color.y,
							light.color.z
						)
					);
				}
				resources.lights.unlock();
*/

				ImGui::Checkbox("Enable Testing Shader", &testing);

				ImGui::End();

				ImGuiIO &io = ImGui::GetIO();
				ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 10.0f);
				{
					ImGui::SetNextWindowPos(ImVec2(1.5f, 1.5f));
					ImGui::Begin("Statistics", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoSavedSettings);
					{
						ImGui::Text("Statistics");
						ImGui::Separator();
						ImGui::Text("Frames Per Second: %.1f (%.3fms/frame)", io.Framerate, 1000.0f / io.Framerate);
						ImGui::Text("Surface Size: %ix%i", context.width, context.height);

						if(ImGui::BeginPopupContextWindow()) {
							if(ImGui::MenuItem("Top-Left", NULL)) {};
							if(ImGui::MenuItem("Top-Right", NULL)) {};
							ImGui::EndPopup();
						}
					}
					ImGui::End();
				}
				ImGui::PopStyleVar();

				ImGui::Render();
			render_api->ui()->end();
			render_api->end_label();
		}
		render_api->end();
		render_api->present();

		start_time = current_time;
	}

	SDL_Quit();

	return 0;
}

void setup_resources(RenderAPI *render_api, RenderResources *resources) {
	resources->position_sampler 	= render_api->create_sampler();
	resources->normal_sampler 		= render_api->create_sampler();
	resources->albedo_sampler 		= render_api->create_sampler();
	resources->scene_buffer 		= render_api->create_buffer();

	render_api->sampler(
		resources->position_sampler,
		SamplerAddressMode::REPEAT,
		SamplerAddressMode::REPEAT,
		SamplerAddressMode::REPEAT
	);

	render_api->sampler(
		resources->albedo_sampler,
		SamplerAddressMode::REPEAT,
		SamplerAddressMode::REPEAT,
		SamplerAddressMode::REPEAT
	);

	render_api->sampler(
		resources->normal_sampler,
		SamplerAddressMode::REPEAT,
		SamplerAddressMode::REPEAT,
		SamplerAddressMode::REPEAT
	);

	render_api->buffer(
		resources->scene_buffer,
		BufferType::UNIFORM,
		sizeof(SceneData),
		nullptr
	);
}

void update_shader_buffers(RenderAPI *api, RenderResources *resources) {
	api->buffer_sub(resources->scene_buffer, 0, sizeof(SceneData), &resources->scene);
}