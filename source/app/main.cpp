struct SceneData {
	glm::mat4 view;
	glm::mat4 projection;
};

struct StorageData {
	glm::mat4 model;
	static const uint32_t MAX_OBJECTS = 1024;
};

struct CompositionData {
	glm::vec4 camera_position;
	int32_t gbuffer_selection;
};

struct LightData {
	glm::vec4 position;
	glm::vec3 color;
	float radius;
	static const uint32_t MAX_LIGHTS = 4;
};

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

	TextureHandle missing_texture = TextureHandle::Invalid;
	TextureViewHandle missing_texture_view = TextureViewHandle::Invalid;
	SamplerHandle missing_texture_sampler = SamplerHandle::Invalid;

	BufferHandle scene_buffer = BufferHandle::Invalid;
	BufferHandle storage_buffer = BufferHandle::Invalid;
	BufferHandle composition_buffer = BufferHandle::Invalid;
	BufferHandle light_buffer = BufferHandle::Invalid;
};

glm::mat4 calculate_model_matrix(glm::vec3 translation, glm::vec3 rotation, glm::vec3 scale) {
	glm::mat4 translation_matrix = glm::translate(glm::mat4(1.0f), translation);
	glm::mat4 rotation_matrix = glm::toMat4(glm::quat(rotation));
	glm::mat4 scale_matrix = glm::scale(glm::mat4(1.0f), scale);

	return translation_matrix * rotation_matrix * scale_matrix;
}

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

void setup_resources(RenderAPI *api, RenderResources *resources);
void setup_pass_resources(AppContext *context, RenderAPI *renderer, RenderPassResources *resources);

#include "renderdefs.h"
#include "camera.h"
#include "mesh.h"
#include "texture.h"

// refactor
struct TempTexture {
	TextureHandle texture;
	TextureViewHandle view;
	SamplerHandle sampler;
};

void create_texture(RenderAPI *render_api, void *data, int width, int height, TempTexture *texture) {
	texture->texture = render_api->create_texture();
	texture->view = render_api->create_texture_view();
	texture->sampler = render_api->create_sampler();

	render_api->texture(
		texture->texture,
		ImageDimensions::IMAGE_2D,
		ImageSamples::SAMPLE_COUNT_1_BIT,
		ImageFormat::R8G8B8A8_UNORM,
		ImageFlags::SAMPLED,
		data, width, height
	);

	render_api->texture_view(
		texture->view,
		texture->texture,
		ImageViewDimensions::IMAGE_2D,
		ImageFormat::R8G8B8A8_UNORM, 0, 0
	);

	render_api->sampler(
		texture->sampler,
		SamplerAddressMode::REPEAT,
		SamplerAddressMode::REPEAT,
		SamplerAddressMode::REPEAT
	);
}

int main(int argc, char *argv[]) {
	UNUSED(argc);
	UNUSED(argv);

	AppContext context;
	context.argc = argc;
	context.argv = argv;
	context.width = 1280;
	context.height = 762;

	Camera camera = { &context };

	if(SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		spdlog::error("Couldn't init SDL: {}", SDL_GetError());
		return 0;
	};

	auto filesystem = get_factory<Filesystem*>("filesystem_std");
	auto render_api = get_factory<RenderAPI*>("api_vk");

	if(!filesystem.good) {
		spdlog::error("Couldn't load VFS");
		return 0;
	}

	if(!render_api.good) {
		spdlog::error("Couldn't load renderer");
		return 0;
	}

	filesystem->init(&context);
	render_api->init(&context);

	filesystem->mount("assets/", "../../assets/");
	filesystem->mount("assets/models/", "../../assets/models/");
	filesystem->mount("assets/fonts/", "../assets/fonts/");
	filesystem->mount("assets/textures/", "../../assets/textures/");
	filesystem->mount("assets/shaders/", "../assets/shaders/");

	//int width, height, nrchannels;
	//unsigned char *data = stbi_load(filesystem->resolve_physical_dir("assets/textures/Ariral_Holds.png").string().c_str(), &width, &height, &nrchannels, 4);

	uint32_t black = glm::packUnorm4x8(glm::vec4(0, 0, 0, 0));
	uint32_t magenta = glm::packUnorm4x8(glm::vec4(1, 0, 1, 1));
	std::array<uint32_t, 16 *16> missing_texture_data;
	for (int x = 0; x < 16; x++) {
		for (int y = 0; y < 16; y++) {
			missing_texture_data[y*16 + x] = ((x % 2) ^ (y % 2)) ? magenta : black;
		}
	}

	ktxTexture *texture;
	ktxTexture_CreateFromNamedFile(
		filesystem->resolve_physical_dir("assets/textures/armor.ktx").string().c_str(),
		KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT,
		&texture
	);
	ktx_uint8_t *texture_data = ktxTexture_GetData(texture);

	TempTexture armor_albedo_texture;
	create_texture(render_api.interface, texture_data, texture->baseWidth, texture->baseHeight, &armor_albedo_texture);

	int nwidth, nheight, nnrchannels;
	unsigned char *armor_normal_data = stbi_load(filesystem->resolve_physical_dir("assets/textures/armor_default_normal.png").string().c_str(), &nwidth, &nheight, &nnrchannels, 4);

	TempTexture armor_normal_texture;
	create_texture(render_api.interface, armor_normal_data, nwidth, nheight, &armor_normal_texture);

	TempTexture missing_texture;
	create_texture(render_api.interface, missing_texture_data.data(), 16, 16, &missing_texture);

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
		->add_uniform(ShaderStage::VERTEX, UniformType::BUFFER)
		->add_uniform(ShaderStage::VERTEX, UniformType::STORAGE)
		->add_uniform(ShaderStage::FRAGMENT, UniformType::TEXTURE)
		->add_uniform(ShaderStage::FRAGMENT, UniformType::TEXTURE)
		->build();

	auto composition_layout = render_api->create_layout()
		->add_uniform(ShaderStage::FRAGMENT, UniformType::BUFFER)
		->add_uniform(ShaderStage::FRAGMENT, UniformType::STORAGE)
		->add_uniform(ShaderStage::FRAGMENT, UniformType::TEXTURE)
		->add_uniform(ShaderStage::FRAGMENT, UniformType::TEXTURE)
		->add_uniform(ShaderStage::FRAGMENT, UniformType::TEXTURE)
		->build();

	auto deferred_vertex_code = filesystem->read_file<char>("assets/shaders/deferred.vert.spv", true);
	auto deferred_fragment_code = filesystem->read_file<char>("assets/shaders/deferred.frag.spv", true);
	auto deferred_shader = render_api->create_graphics_program()
		->add_attachment(ImageFormat::R16G16B16A16_SFLOAT)
		->add_attachment(ImageFormat::R16G16B16A16_SFLOAT)
		->add_attachment(ImageFormat::R8G8B8A8_UNORM)
		->set_depth_format(ImageFormat::D32_SFLOAT)
		->set_depth_test(true, true, ShaderCompareOp::LESS_OR_EQUAL)
		->add_binding(sizeof(Vertex), BindingRate::VERTEX)
		->add_attribute(offsetof(Vertex, position), AttributeType::VEC3D_SIGNED)
		->add_attribute(offsetof(Vertex, normal), AttributeType::VEC3D_SIGNED)
		->add_attribute(offsetof(Vertex, tangent), AttributeType::VEC3D_SIGNED)
		->add_attribute(offsetof(Vertex, uv), AttributeType::VEC2D_SIGNED)
		->add_stage(ShaderStage::VERTEX, deferred_vertex_code.data(), deferred_vertex_code.size())
		->add_stage(ShaderStage::FRAGMENT, deferred_fragment_code.data(), deferred_fragment_code.size())
		->set_layout(deferred_layout)
		->build();

	auto deferred_wireframe_shader = render_api->create_graphics_program()
		->add_attachment(ImageFormat::R16G16B16A16_SFLOAT)
		->add_attachment(ImageFormat::R16G16B16A16_SFLOAT)
		->add_attachment(ImageFormat::R8G8B8A8_UNORM)
		->set_depth_format(ImageFormat::D32_SFLOAT)
		->set_polygon_mode(ShaderPolygonMode::LINE)
		->set_depth_test(true, true, ShaderCompareOp::LESS_OR_EQUAL)
		->add_binding(sizeof(Vertex), BindingRate::VERTEX)
		->add_attribute(offsetof(Vertex, position), AttributeType::VEC3D_SIGNED)
		->add_attribute(offsetof(Vertex, normal), AttributeType::VEC3D_SIGNED)
		->add_attribute(offsetof(Vertex, tangent), AttributeType::VEC3D_SIGNED)
		->add_attribute(offsetof(Vertex, uv), AttributeType::VEC2D_SIGNED)
		->add_stage(ShaderStage::VERTEX, deferred_vertex_code.data(), deferred_vertex_code.size())
		->add_stage(ShaderStage::FRAGMENT, deferred_fragment_code.data(), deferred_fragment_code.size())
		->set_layout(deferred_layout)
		->build();

	auto composition_vertex_code = filesystem->read_file<char>("assets/shaders/composition.vert.spv", true);
	auto composition_fragment_code = filesystem->read_file<char>("assets/shaders/composition.frag.spv", true);
	auto composition_shader = render_api->create_graphics_program()
		->add_attachment(ImageFormat::R16G16B16A16_SFLOAT)
		->set_depth_format(ImageFormat::D32_SFLOAT)
		->add_stage(ShaderStage::VERTEX, composition_vertex_code.data(), composition_vertex_code.size())
		->add_stage(ShaderStage::FRAGMENT, composition_fragment_code.data(), composition_fragment_code.size())
		->set_layout(composition_layout)
		->build();

	auto skybox_layout = render_api->create_layout()
		->add_uniform(ShaderStage::VERTEX, UniformType::BUFFER)
		->add_uniform(ShaderStage::FRAGMENT, UniformType::TEXTURE)
		->build();

	auto skybox_vertex_code = filesystem->read_file<char>("assets/shaders/skybox.vert.spv", true);
	auto skybox_fragment_code = filesystem->read_file<char>("assets/shaders/skybox.frag.spv", true);
	auto skybox_shader = render_api->create_graphics_program()
		->add_attachment(ImageFormat::R16G16B16A16_SFLOAT)
		->set_depth_format(ImageFormat::D32_SFLOAT)
		->set_depth_test(true, false, ShaderCompareOp::LESS_OR_EQUAL)
		->add_binding(sizeof(Vertex), BindingRate::VERTEX)
		->add_attribute(offsetof(Vertex, position), AttributeType::VEC3D_SIGNED)
		->add_attribute(offsetof(Vertex, normal), AttributeType::VEC3D_SIGNED)
		->add_attribute(offsetof(Vertex, tangent), AttributeType::VEC3D_SIGNED)
		->add_attribute(offsetof(Vertex, uv), AttributeType::VEC2D_SIGNED)
		->add_stage(ShaderStage::VERTEX, skybox_vertex_code.data(), skybox_vertex_code.size())
		->add_stage(ShaderStage::FRAGMENT, skybox_fragment_code.data(), skybox_fragment_code.size())
		->set_layout(skybox_layout)
		->build();

	UNUSED(deferred_shader);
	UNUSED(composition_shader);

	Mesh armor_mesh(&context, filesystem.interface, render_api.interface);
	armor_mesh.load_from_file("assets/models/armor.gltf");

	Mesh monkey_mesh(&context, filesystem.interface, render_api.interface);
	monkey_mesh.load_from_file("assets/models/monkey.glb");

	Mesh cube_mesh(&context, filesystem.interface, render_api.interface);
	cube_mesh.load_from_file("assets/models/cube.glb");

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
		scene_data.projection = glm::perspective(glm::radians(70.f), static_cast<float>(context.width) / static_cast<float>(context.height), 0.01f, 1000.0f);
		scene_data.projection[1][1] *= -1;
		render_api->buffer_sub(resources.scene_buffer, 0, sizeof(SceneData), &scene_data);

		static StorageData storage_data[StorageData::MAX_OBJECTS] = {};
		storage_data[0].model = calculate_model_matrix(glm::vec3(0, 0, 0), glm::vec3(0, 0, 0), glm::vec3(0.01f));
		storage_data[1].model = calculate_model_matrix(glm::vec3(0, 2, 0), glm::vec3(0, 0, 0), glm::vec3(1.0f));
		render_api->buffer_sub(resources.storage_buffer, 0, sizeof(StorageData) * StorageData::MAX_OBJECTS, &storage_data);

		static CompositionData composition_data = {};
		composition_data.camera_position = glm::vec4(camera.get_position(), 0.0f) * glm::vec4(-1.0f, 1.0f, -1.0f, 1.0f);
		render_api->buffer_sub(resources.composition_buffer, 0, sizeof(CompositionData), &composition_data);

		static LightData light_data[LightData::MAX_LIGHTS] = {};
		render_api->buffer_sub(resources.light_buffer, 0, sizeof(LightData) * LightData::MAX_LIGHTS, &light_data);

		render_api->begin();
		{
			render_api->clear(0, 0, 0, 1);

			// Offscreen/Deferred rendering (first pass)
			std::vector<SubpassAttachment> deferred_attachments = {
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

			std::vector<UniformBind> deferred_binds = {
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
				{ .type = UniformType::TEXTURE },
				{ .type = UniformType::TEXTURE },
			};


			static bool testing = false;

			render_api->begin_pass(deferred_attachments);
				render_api->viewport(static_cast<float>(context.width), static_cast<float>(context.height));
				render_api->scissor(context.width, context.height);
				render_api->bind_shader(testing ? deferred_wireframe_shader : deferred_shader);
				deferred_binds[2].texture = {
					.texture_view_handle = armor_albedo_texture.view,
					.sampler_handle = armor_albedo_texture.sampler
				};
				deferred_binds[3].texture = {
					.texture_view_handle = armor_normal_texture.view,
					.sampler_handle = armor_normal_texture.sampler
				};
				render_api->bind_uniform(deferred_layout, deferred_binds);
				armor_mesh.draw(0);
				deferred_binds[2].texture = {
					.texture_view_handle = missing_texture.view,
					.sampler_handle = missing_texture.sampler
				};
				deferred_binds[3].texture = {
					.texture_view_handle = missing_texture.view,
					.sampler_handle = missing_texture.sampler
				};
				render_api->bind_uniform(deferred_layout, deferred_binds);
				monkey_mesh.draw(1);
			render_api->end_pass(deferred_attachments);

			std::vector<SubpassAttachment> composition_attachments = {
				{
					.texture = renderpass_resources.composition,
					.view = renderpass_resources.composition_view,
					.type = AttachmentType::COLOR,
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
				{
					.texture = renderpass_resources.depth,
					.view = renderpass_resources.depth_view,
					.type = AttachmentType::DEPTH,
				}
			};

			std::vector<UniformBind> composition_binds = {
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

			std::vector<UniformBind> skybox_uniforms = {
				{
					.buffer = {
						.buffer_handle = resources.scene_buffer,
						.range = sizeof(SceneData)
					},
					.type = UniformType::BUFFER,
				},
				{
					.texture = {
						.texture_view_handle = missing_texture.view,
						.sampler_handle = missing_texture.sampler
					},
					.type = UniformType::TEXTURE
				},
			};

			UNUSED(skybox_shader);

			render_api->begin_pass(composition_attachments);
				render_api->viewport(static_cast<float>(context.width), static_cast<float>(context.height));
				render_api->scissor(context.width, context.height);
				render_api->bind_uniform(composition_layout, composition_binds);
				render_api->bind_shader(composition_shader);
				render_api->draw(3, 1);
				render_api->bind_uniform(skybox_layout, skybox_uniforms);
				render_api->bind_shader(skybox_shader);
				cube_mesh.draw(0);
			render_api->end_pass(composition_attachments);

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

				ImGui::ShowDemoWindow();

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
