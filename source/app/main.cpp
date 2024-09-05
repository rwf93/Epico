#include "renderdefs.h"
#include "camera.h"
#include "mesh.h"
#include "texture.h"

glm::mat4 calculate_model_matrix(glm::vec3 translation, glm::vec3 rotation, glm::vec3 scale) {
	glm::mat4 translation_matrix = glm::translate(glm::mat4(1.0f), translation);
	glm::mat4 rotation_matrix = glm::toMat4(glm::quat(rotation));
	glm::mat4 scale_matrix = glm::scale(glm::mat4(1.0f), scale);

	return translation_matrix * rotation_matrix * scale_matrix;
}

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

struct RenderPassResources {
	TextureHandle position = TextureHandle::Invalid;
	TextureHandle normal = TextureHandle::Invalid;
	TextureHandle albedo = TextureHandle::Invalid;
	TextureHandle depth = TextureHandle::Invalid;
	TextureHandle composition = TextureHandle::Invalid;

	TextureViewHandle position_view = TextureViewHandle::Invalid;
	TextureViewHandle albedo_view = TextureViewHandle::Invalid;
	TextureViewHandle normal_view = TextureViewHandle::Invalid;
	TextureViewHandle depth_view = TextureViewHandle::Invalid;
	TextureViewHandle composition_view = TextureViewHandle::Invalid;
};

struct RenderResources {
	RenderPassResources *pass_handles;
	SamplerHandle position_sampler = SamplerHandle::Invalid;
	SamplerHandle normal_sampler = SamplerHandle::Invalid;
	SamplerHandle albedo_sampler = SamplerHandle::Invalid;

	BufferHandle scene_buffer = BufferHandle::Invalid;
	BufferHandle storage_buffer = BufferHandle::Invalid;
	BufferHandle composition_buffer = BufferHandle::Invalid;
	BufferHandle light_buffer = BufferHandle::Invalid;
};


void setup_resources(RenderAPI *api, RenderResources *resources);
void setup_pass_resources(AppContext *context, RenderAPI *renderer, RenderPassResources *resources);

// refactor
struct TempTexture {
	TextureHandle texture;
	TextureViewHandle view;
	SamplerHandle sampler;
};

void create_texture(RenderAPI *render_api, void *data, int width, int height, TempTexture *texture, ImageFormat format = ImageFormat::R8G8B8A8_UNORM) {
	texture->texture = render_api->create_texture();
	texture->view = render_api->create_texture_view();
	texture->sampler = render_api->create_sampler();

	render_api->texture(
		texture->texture,
		ImageDimensions::IMAGE_2D,
		ImageSamples::SAMPLE_COUNT_1_BIT,
		format,
		ImageFlags::SAMPLED,
		data, width, height
	);

	render_api->texture_view(
		texture->view,
		texture->texture,
		ImageViewDimensions::IMAGE_2D,
		format, 0, 0
	);

	render_api->sampler(
		texture->sampler,
		SamplerAddressMode::REPEAT,
		SamplerAddressMode::REPEAT,
		SamplerAddressMode::REPEAT
	);
}

class Model {
public:
	Model(RenderAPI *api, Filesystem *filesystem):
		api(api),
		filesystem(filesystem) {}

	void load(std::filesystem::path path) {
		Assimp::Importer importer;
		const aiScene *scene = importer.ReadFile(
			filesystem->resolve_physical_dir(path).string(),
			aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_CalcTangentSpace
		);

		process_node(scene->mRootNode, scene, calculate_model_matrix(glm::vec3(0), glm::vec3(0), glm::vec3(1.0)));
	}

	void process_node(aiNode *node, const aiScene *scene, glm::mat4 parent_transform) {
		transforms.push_back(parent_transform);
		glm::mat4 transform = convert_matrix(node->mTransformation) * parent_transform;

		for(unsigned int i = 0; i < node->mNumMeshes; i++) {
			aiMesh *assimp_mesh = scene->mMeshes[node->mMeshes[i]];

			Mesh mesh(filesystem, api);
			mesh.process(assimp_mesh);

			meshes.push_back(mesh);
		}

		for(unsigned int i = 0; i < node->mNumChildren; i++)
			process_node(node->mChildren[i], scene, transform);
	}

	glm::mat4 convert_matrix(const aiMatrix4x4 &matrix) {
		return {
			matrix.a1, matrix.b1, matrix.c1, matrix.d1,
			matrix.a2, matrix.b2, matrix.c2, matrix.d2,
			matrix.a3, matrix.b3, matrix.c3, matrix.d3,
			matrix.a4, matrix.b4, matrix.c4, matrix.d4
		};
	}

	void draw(std::vector<StorageData> &render_objects, uint32_t &render_index) {
		UNUSED(render_objects);
		UNUSED(render_index);

		for(auto &mesh: meshes)
			mesh.draw(1, 0);
	}
private:
	std::vector<Mesh> meshes;
	std::vector<glm::mat4> transforms;

	RenderAPI *api;
	Filesystem *filesystem;
};

int main(int argc, char *argv[]) {
	UNUSED(argc);
	UNUSED(argv);

	AppContext context;
	context.argc = argc;
	context.argv = argv;
	context.width = 1280;
	context.height = 762;

	Camera camera(&context);

	if(SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		spdlog::error("Couldn't init SDL: {}", SDL_GetError());
		return 0;
	};

	auto filesystem = get_factory<Filesystem*>("filesystem_std", &context, std::filesystem::weakly_canonical(argv[0]).parent_path().append("./"));
	auto render_api = get_factory<RenderAPI*>("api_vk", &context, std::filesystem::weakly_canonical(argv[0]).parent_path().append("./"));

	if(!filesystem.good) {
		spdlog::error("Couldn't load VFS");
		return 0;
	}

	if(!render_api.good) {
		spdlog::error("Couldn't load renderer");
		return 0;
	}

	filesystem->mount("assets/", "../../assets/");
	filesystem->mount("assets/models/", "../../assets/models/");
	filesystem->mount("assets/fonts/", "../assets/fonts/");
	filesystem->mount("assets/textures/", "../../assets/textures/");
	filesystem->mount("assets/shaders/", "../assets/shaders/");

	uint32_t black = glm::packUnorm4x8(glm::vec4(0, 0, 0, 0));
	uint32_t magenta = glm::packUnorm4x8(glm::vec4(1, 0, 1, 1));
	std::array<uint32_t, 16 *16> missing_texture_data;
	for (int x = 0; x < 16; x++) {
		for (int y = 0; y < 16; y++) {
			missing_texture_data[y*16 + x] = ((x % 2) ^ (y % 2)) ? magenta : black;
		}
	}

	TempTexture missing_texture;
	create_texture(render_api.interface, missing_texture_data.data(), 16, 16, &missing_texture);

	Texture armor_albedo_texture(render_api.interface);
	ImageTexture armor_albedo_image(&armor_albedo_texture, filesystem.interface);
	armor_albedo_image.upload_from_ktx("assets/textures/colormap_rgba.ktx");

	Texture armor_normal_texture(render_api.interface);
	ImageTexture armor_normal_image(&armor_normal_texture, filesystem.interface);
	armor_normal_image.upload_from_ktx("assets/textures/normalmap_rgba.ktx");

	Texture skybox_texture(render_api.interface);
	ImageTexture skybox_image(&skybox_texture, filesystem.interface);
	skybox_image.upload_from_ktx("assets/textures/skysphere_rgba.ktx");

	Texture monkey_normals(render_api.interface);
	ImageTexture monkey_normals_image(&monkey_normals, filesystem.interface);
	monkey_normals_image.upload_from_ktx("assets/textures/suzanne_normal.ktx");

	RenderPassResources renderpass_resources = {};
	RenderResources resources = {
		.pass_handles = &renderpass_resources
	};

	setup_resources(render_api.interface, &resources);
	setup_pass_resources(&context, render_api.interface, &renderpass_resources);
	render_api->on_resize([&](RenderAPI* renderer) {
		setup_pass_resources(&context, renderer, &renderpass_resources);
	});

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

	auto deferred_vertex_code = filesystem->read_file<char>("assets/shaders/deferred.vert.spv", true);
	auto deferred_fragment_code = filesystem->read_file<char>("assets/shaders/deferred.frag.spv", true);
	auto &deferred_builder = render_api->create_graphics_program()
		.add_attachment(ImageFormat::R16G16B16A16_SFLOAT) // Position
		.add_attachment(ImageFormat::R16G16B16A16_SFLOAT) // Normals
		.add_attachment(ImageFormat::R8G8B8A8_UNORM) 	  // Albedo
		.set_depth_format(ImageFormat::D32_SFLOAT)
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
		.add_stage(ShaderStage::VERTEX, deferred_vertex_code)
		.add_stage(ShaderStage::FRAGMENT, deferred_fragment_code)
		.build();

	auto deferred_test_program = deferred_builder
		.clear_stages()
		.add_stage(ShaderStage::VERTEX, deferred_vertex_code)
		.add_stage(ShaderStage::FRAGMENT, deferred_fragment_code)
		.build();

	auto composition_vertex_code = filesystem->read_file<char>("assets/shaders/composition.vert.spv", true);
	auto composition_fragment_code = filesystem->read_file<char>("assets/shaders/composition.frag.spv", true);
	auto composition_shader = render_api->create_graphics_program()
		.add_attachment(ImageFormat::R16G16B16A16_SFLOAT)
		.set_depth_format(ImageFormat::D32_SFLOAT)
		.add_stage(ShaderStage::VERTEX, composition_vertex_code)
		.add_stage(ShaderStage::FRAGMENT, composition_fragment_code)
		.set_layout(composition_layout)
		.build();

	auto skybox_vertex_code = filesystem->read_file<char>("assets/shaders/skybox.vert.spv", true);
	auto skybox_fragment_code = filesystem->read_file<char>("assets/shaders/skybox.frag.spv", true);
	auto skybox_shader = render_api->create_graphics_program()
		.add_attachment(ImageFormat::R16G16B16A16_SFLOAT)
		.set_depth_format(ImageFormat::D32_SFLOAT)
		.set_cull_face(CullFace::FRONT)
		.set_front_face(FrontFace::COUNTER_CLOCKWISE)
		.set_depth_test(true, false, CompareOp::EQUAL)
		.add_binding(sizeof(Vertex), BindingRate::VERTEX)
		.add_attribute(offsetof(Vertex, position), AttributeType::VEC3F_SIGNED)
		.add_attribute(offsetof(Vertex, normal), AttributeType::VEC3F_SIGNED)
		.add_attribute(offsetof(Vertex, tangent), AttributeType::VEC3F_SIGNED)
		.add_attribute(offsetof(Vertex, uv), AttributeType::VEC2F_SIGNED)
		.add_stage(ShaderStage::VERTEX, skybox_vertex_code)
		.add_stage(ShaderStage::FRAGMENT, skybox_fragment_code)
		.set_layout(skybox_layout)
		.build();

	Mesh armor_mesh(filesystem.interface, render_api.interface);
	armor_mesh.load_from_file("assets/models/armor.gltf");

	Mesh monkey_mesh(filesystem.interface, render_api.interface);
	monkey_mesh.load_from_file("assets/models/monkey.glb");

	Mesh sphere_mesh(filesystem.interface, render_api.interface);
	sphere_mesh.load_from_file("assets/models/sphere.glb");

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

		static SceneData scene_data = {};
		scene_data.view = camera.get_view_matrix();
		scene_data.projection = glm::perspective(glm::radians(70.f), static_cast<float>(context.width) / static_cast<float>(context.height), 0.1f, 1000.0f);
		scene_data.projection[1][1] *= -1;
		scene_data.resolution = glm::vec2(context.width, context.height);
		scene_data.time = context.time;
		scene_data.time_delta = context.time_delta;
		render_api->buffer_sub(resources.scene_buffer, 0, sizeof(SceneData), &scene_data);

		static std::vector<StorageData> storage_data(StorageData::MAX_OBJECTS);
		storage_data.at(0).model = calculate_model_matrix(glm::vec3(0), glm::vec3(0), glm::vec3(1.0));
		render_api->buffer_sub(resources.storage_buffer, 0, storage_data.size(), storage_data.data());
		//uint32_t render_index = 0;

		static CompositionData composition_data = {};
		composition_data.camera_position = glm::vec4(camera.get_position(), 0.0f) * glm::vec4(-1.0f, 1.0f, -1.0f, 1.0f);
		render_api->buffer_sub(resources.composition_buffer, 0, sizeof(CompositionData), &composition_data);

		static LightData light_data[LightData::MAX_LIGHTS] = {};
		render_api->buffer_sub(resources.light_buffer, 0, sizeof(LightData) * LightData::MAX_LIGHTS, &light_data);

		render_api->begin();
		{
			render_api->clear(0, 0, 0, 1);

			// Offscreen/Deferred rendering (first pass)
			static std::vector<SubpassAttachment> deferred_attachments = {
				{
					.texture = renderpass_resources.position,
					.view = renderpass_resources.position_view,
					.type = AttachmentType::COLOR,
					.clear = (SubpassAttachment::ClearValue{ .rgba = { 0.0f, 0.0f, 0.0f, 0.0f } })
				},
				{
					.texture = renderpass_resources.normal,
					.view = renderpass_resources.normal_view,
					.type = AttachmentType::COLOR,
					.clear = (SubpassAttachment::ClearValue{ .rgba = { 0.0f, 0.0f, 0.0f, 0.0f } })
				},
				{
					.texture = renderpass_resources.albedo,
					.view = renderpass_resources.albedo_view,
					.type = AttachmentType::COLOR,
					.clear = (SubpassAttachment::ClearValue{ .rgba = { 0.0f, 0.0f, 0.0f, 0.0f } })
				},
				{
					.texture = renderpass_resources.depth,
					.view = renderpass_resources.depth_view,
					.type = AttachmentType::DEPTH,
					.clear = (SubpassAttachment::ClearValue{ .rgba = { 1.0f }, .depth = 0.0f, .stencil = 0 })
				}
			};

			static std::vector<UniformBind> deferred_binds = {
				{
					.buffer = {
						.buffer_handle = resources.scene_buffer,
						.range = sizeof(SceneData)
					},
					.type = UniformType::BUFFER,
				},
				{
					.buffer = {
						.buffer_handle = resources.storage_buffer,
						.range = sizeof(StorageData) * StorageData::MAX_OBJECTS
					},
					.type = UniformType::STORAGE,
				},
			};

			static bool testing = false;

			render_api->begin_pass(deferred_attachments);
				render_api->viewport(static_cast<float>(context.width), static_cast<float>(context.height));
				render_api->scissor(context.width, context.height);

				render_api->bind_program(testing ? deferred_test_program : deferred_program);

				deferred_binds.push_back(armor_albedo_texture.as_bind());
				deferred_binds.push_back(armor_normal_texture.as_bind());
				render_api->bind_uniform(deferred_layout, deferred_binds);
				deferred_binds.pop_back();
				deferred_binds.pop_back();

				monkey_mesh.draw(1);

			render_api->end_pass(deferred_attachments);

			static std::vector<SubpassAttachment> composition_attachments = {
				{
					.texture = renderpass_resources.composition,
					.view = renderpass_resources.composition_view,
					.type = AttachmentType::COLOR,
					.clear = (SubpassAttachment::ClearValue{ .rgba = { 0.0f, 0.0f, 0.0f, 0.0f } })
				},
				{
					.texture = renderpass_resources.position,
					.view = renderpass_resources.position_view,
					.type = AttachmentType::SHADER
				},
				{
					.texture = renderpass_resources.albedo,
					.view = renderpass_resources.albedo_view,
					.type = AttachmentType::SHADER
				},
				{
					.texture = renderpass_resources.normal,
					.view = renderpass_resources.normal_view,
					.type = AttachmentType::SHADER
				},
			};

			static std::vector<UniformBind> composition_binds = {
				{
					.buffer = {
						.buffer_handle = resources.composition_buffer,
						.offset = 0,
						.range = sizeof(CompositionData)
					},
					.type = UniformType::BUFFER
				},
				{
					.buffer = {
						.buffer_handle = resources.light_buffer,
						.offset = 0,
						.range = sizeof(LightData) * LightData::MAX_LIGHTS
					},
					.type = UniformType::STORAGE
				},
				{
					.texture = {
						.texture_view_handle = renderpass_resources.position_view,
						.sampler_handle = resources.position_sampler
					},
					.type = UniformType::TEXTURE,
				},
				{
					.texture = {
						.texture_view_handle = renderpass_resources.normal_view,
						.sampler_handle = resources.normal_sampler
					},
					.type = UniformType::TEXTURE,
				},
				{
					.texture = {
						.texture_view_handle = renderpass_resources.albedo_view,
						.sampler_handle = resources.albedo_sampler
					},
					.type = UniformType::TEXTURE,
				},
			};
			// Lighting/Composition pass
			render_api->begin_pass(composition_attachments);
				render_api->viewport(static_cast<float>(context.width), static_cast<float>(context.height));
				render_api->scissor(context.width, context.height);
				render_api->bind_uniform(composition_layout, composition_binds);
				render_api->bind_program(composition_shader);
				render_api->draw(3, 1);
			render_api->end_pass(composition_attachments);

			static std::vector<SubpassAttachment> skybox_attachments = {
				{
					.texture = renderpass_resources.composition,
					.view = renderpass_resources.composition_view,
					.type = AttachmentType::COLOR,
				},
				{
					.texture = renderpass_resources.depth,
					.view = renderpass_resources.depth_view,
					.type = AttachmentType::DEPTH,
				}
			};

			static std::vector<UniformBind> skybox_uniforms = {
				{
					.buffer = {
						.buffer_handle = resources.scene_buffer,
						.range = sizeof(SceneData)
					},
					.type = UniformType::BUFFER,
				},
				skybox_texture.as_bind()
			};

			// Skybox pass
			render_api->begin_pass(skybox_attachments);
				render_api->bind_uniform(skybox_layout, skybox_uniforms);
				render_api->bind_program(skybox_shader);
				sphere_mesh.draw();
			render_api->end_pass(skybox_attachments);

			render_api->show_image(resources.pass_handles->composition);

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
				ImGui::Combo("G-Buffer", &composition_data.gbuffer_selection, items, IM_ARRAYSIZE(items));

				for(uint32_t i = 0; i < LightData::MAX_LIGHTS; i++) {
					ImGui::SliderFloat3(fmt::format("Light {} Position", i).c_str(), glm::value_ptr(light_data[i].position), -100, 100);
					ImGui::ColorEdit3(fmt::format("Light {} Color", i).c_str(), glm::value_ptr(light_data[i].color));
					ImGui::SliderFloat(fmt::format("Light {} Radius", i).c_str(), &light_data[i].radius, 0.5, 100);
					ImGui::Separator();
				}

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
		}
		render_api->end();
		render_api->present();

		start_time = current_time;
	}

	render_api.release();
	filesystem.release();
	SDL_Quit();

	return 0;
}

void setup_resources(RenderAPI *render_api, RenderResources *resources) {
	resources->pass_handles->position = render_api->create_texture();
	resources->pass_handles->normal = render_api->create_texture();
	resources->pass_handles->albedo = render_api->create_texture();
	resources->pass_handles->depth = render_api->create_texture();
	resources->pass_handles->composition = render_api->create_texture();
	resources->pass_handles->position_view = render_api->create_texture_view();
	resources->pass_handles->normal_view = render_api->create_texture_view();
	resources->pass_handles->albedo_view = render_api->create_texture_view();
	resources->pass_handles->depth_view = render_api->create_texture_view();
	resources->pass_handles->composition_view = render_api->create_texture_view();
	resources->position_sampler = render_api->create_sampler();
	resources->normal_sampler = render_api->create_sampler();
	resources->albedo_sampler = render_api->create_sampler();
	resources->scene_buffer = render_api->create_buffer();
	resources->storage_buffer = render_api->create_buffer();
	resources->composition_buffer = render_api->create_buffer();
	resources->light_buffer = render_api->create_buffer();

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

	render_api->buffer(
		resources->storage_buffer,
		BufferType::STORAGE,
		sizeof(StorageData) * StorageData::MAX_OBJECTS,
		nullptr
	);

	render_api->buffer(
		resources->composition_buffer,
		BufferType::UNIFORM,
		sizeof(CompositionData),
		nullptr
	);

	render_api->buffer(
		resources->light_buffer,
		BufferType::STORAGE,
		sizeof(LightData) * LightData::MAX_LIGHTS,
		nullptr
	);
}

void setup_pass_resources(AppContext *context, RenderAPI *renderer, RenderPassResources *resources) {
	renderer->texture(
		resources->position,
		ImageDimensions::IMAGE_2D,
		ImageSamples::SAMPLE_COUNT_1_BIT,
		ImageFormat::R16G16B16A16_SFLOAT,
		ImageFlags::COLOR_ATTACHMENT | ImageFlags::SAMPLED,
		nullptr,
		context->width, context->height
	);

	renderer->texture(
		resources->albedo,
		ImageDimensions::IMAGE_2D,
		ImageSamples::SAMPLE_COUNT_1_BIT,
		ImageFormat::R8G8B8A8_UNORM,
		ImageFlags::COLOR_ATTACHMENT | ImageFlags::SAMPLED,
		nullptr,
		context->width, context->height
	);

	renderer->texture(
		resources->normal,
		ImageDimensions::IMAGE_2D,
		ImageSamples::SAMPLE_COUNT_1_BIT,
		ImageFormat::R16G16B16A16_SFLOAT,
		ImageFlags::COLOR_ATTACHMENT | ImageFlags::SAMPLED,
		nullptr,
		context->width, context->height
	);

	renderer->texture(
		resources->depth,
		ImageDimensions::IMAGE_2D,
		ImageSamples::SAMPLE_COUNT_1_BIT,
		ImageFormat::D32_SFLOAT,
		ImageFlags::DEPTH_ATTACHMENT,
		nullptr,
		context->width, context->height
	);

	renderer->texture(
		resources->composition,
		ImageDimensions::IMAGE_2D,
		ImageSamples::SAMPLE_COUNT_1_BIT,
		ImageFormat::R16G16B16A16_SFLOAT,
		ImageFlags::COLOR_ATTACHMENT,
		nullptr,
		context->width, context->height
	);

	renderer->texture_view(
		resources->position_view,
		resources->position,
		ImageViewDimensions::IMAGE_2D,
		ImageFormat::R16G16B16A16_SFLOAT,
		0, 0
	);

	renderer->texture_view(
		resources->albedo_view,
		resources->albedo,
		ImageViewDimensions::IMAGE_2D,
		ImageFormat::R8G8B8A8_UNORM,
		0, 0
	);

	renderer->texture_view(
		resources->normal_view,
		resources->normal,
		ImageViewDimensions::IMAGE_2D,
		ImageFormat::R16G16B16A16_SFLOAT,
		0, 0
	);

	renderer->texture_view(
		resources->depth_view,
		resources->depth,
		ImageViewDimensions::IMAGE_2D,
		ImageFormat::D32_SFLOAT,
		0, 0
	);

	renderer->texture_view(
		resources->composition_view,
		resources->composition,
		ImageViewDimensions::IMAGE_2D,
		ImageFormat::R16G16B16A16_SFLOAT,
		0, 0
	);
}
