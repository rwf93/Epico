struct Vertex {
	glm::vec3 position;
	glm::vec3 color;
	glm::vec2 uv;
};

struct SceneData {
	glm::mat4 view;
	glm::mat4 projection;
	glm::vec3 color;
};

struct StorageData {
	glm::mat4 model;
};

struct CompositionData {
	uint32_t gbuffer_selection;
};

struct RenderPassResources {
	TextureHandle position;
	TextureHandle albedo;
	TextureHandle depth;
	TextureHandle composition;

	TextureViewHandle position_view;
	TextureViewHandle albedo_view;
	TextureViewHandle depth_view;
	TextureViewHandle composition_view;
};

struct RenderResources {
	RenderPassResources *pass_handles;
	SamplerHandle albedo_sampler;
	SamplerHandle position_sampler;

	TextureHandle missing_texture;
	TextureViewHandle missing_texture_view;
	SamplerHandle missing_texture_sampler;

	BufferHandle scene_buffer;
	BufferHandle storage_buffer;
	BufferHandle composition_buffer;
};

struct CameraData {
	glm::vec3 front;
	glm::vec3 up;
	glm::vec3 right;
	glm::vec3 direction;
	glm::vec3 position;
};

glm::mat4 calculate_model_matrix(glm::vec3 translation, glm::vec3 rotation, glm::vec3 scale) {
	glm::mat4 translation_matrix = glm::translate(glm::mat4(1.0f), translation);
	glm::mat4 rotation_matrix = glm::toMat4(glm::quat(rotation));
	glm::mat4 scale_matrix = glm::scale(glm::mat4(1.0f), scale);

	return translation_matrix * rotation_matrix * scale_matrix;
}

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

void setup_resources(AppContext *context, RenderAPI *api, RenderResources *resources);
void setup_pass_resources(AppContext *context, RenderAPI *renderer, RenderPassResources *resources);

void update_camera(CameraData *camera, AppContext *context);

int main(int argc, char *argv[]) {
	UNUSED(argc);
	UNUSED(argv);

	AppContext context;
	context.argc = argc;
	context.argv = argv;
	context.width = 1280;
	context.height = 762;

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

	uint32_t black = glm::packUnorm4x8(glm::vec4(0, 0, 0, 0));
	uint32_t magenta = glm::packUnorm4x8(glm::vec4(1, 0, 1, 1));
	std::array<uint32_t, 16 *16> missing_texture_data;
	for (int x = 0; x < 16; x++) {
		for (int y = 0; y < 16; y++) {
			missing_texture_data[y*16 + x] = ((x % 2) ^ (y % 2)) ? magenta : black;
		}
	}

	auto missing_texture_handle = render_api->create_texture();
	render_api->texture_data(
		missing_texture_handle,
		ImageDimensions::IMAGE_2D,
		ImageSamples::SAMPLE_COUNT_1_BIT,
		ImageFormat::R8G8B8A8_UNORM,
		ImageFlags::SAMPLED,
		missing_texture_data.data(), 16, 16, 1
	);

	auto missing_texture_view_handle = render_api->create_texture_view();
	render_api->texture_view(
		missing_texture_view_handle,
		missing_texture_handle,
		ImageViewDimensions::IMAGE_2D,
		ImageFormat::R8G8B8A8_UNORM, 0, 0
	);

	auto missing_texture_sampler_handle = render_api->create_sampler();
	render_api->sampler(missing_texture_sampler_handle, SamplerAddressMode::CLAMP_BORDER, SamplerAddressMode::CLAMP_BORDER, SamplerAddressMode::CLAMP_BORDER);

	RenderPassResources renderpass_resources = {};
	RenderResources resources = {
		.pass_handles = &renderpass_resources
	};

	setup_resources(&context, render_api.interface, &resources);
	setup_pass_resources(&context, render_api.interface, &renderpass_resources);
	render_api->on_resize([&](RenderAPI* renderer) {
		setup_pass_resources(&context, renderer, &renderpass_resources);
	});

	auto deferred_layout = render_api->create_layout()
		->add_uniform(ShaderStage::VERTEX, UniformType::BUFFER)
		->add_uniform(ShaderStage::VERTEX, UniformType::STORAGE)
		->add_uniform(ShaderStage::FRAGMENT, UniformType::TEXTURE)
		->build();

	auto composition_layout = render_api->create_layout()
		->add_uniform(ShaderStage::FRAGMENT, UniformType::BUFFER)
		->add_uniform(ShaderStage::FRAGMENT, UniformType::TEXTURE)
		->add_uniform(ShaderStage::FRAGMENT, UniformType::TEXTURE)
		->build();

	auto deferred_vertex_code = filesystem->read_file<char>("assets/shaders/deferred.vert.spv", true);
	auto deferred_fragment_code = filesystem->read_file<char>("assets/shaders/deferred.frag.spv", true);
	auto deferred_shader = render_api->create_graphics_program()
		->add_attachment(ImageFormat::R16G16B16A16_SFLOAT)
		->add_attachment(ImageFormat::R8G8B8A8_UNORM)
		->set_depth_format(ImageFormat::D32_SFLOAT)
		->set_depth_test(true, ShaderCompareOp::LESS_OR_EQUAL)
		->add_binding(0, sizeof(Vertex), BindingRate::VERTEX)
		->add_attribute(0, 0, offsetof(Vertex, position), AttributeType::VEC3D_SIGNED)
		->add_attribute(1, 0, offsetof(Vertex, color), AttributeType::VEC3D_SIGNED)
		->add_attribute(2, 0, offsetof(Vertex, uv), AttributeType::VEC2D_SIGNED)
		->add_stage(ShaderStage::VERTEX, deferred_vertex_code.data(), deferred_vertex_code.size())
		->add_stage(ShaderStage::FRAGMENT, deferred_fragment_code.data(), deferred_fragment_code.size())
		->set_layout(deferred_layout)
		->build();

	auto composition_vertex_code = filesystem->read_file<char>("assets/shaders/composition.vert.spv", true);
	auto composition_fragment_code = filesystem->read_file<char>("assets/shaders/composition.frag.spv", true);
	auto composition_shader = render_api->create_graphics_program()
		->add_attachment(ImageFormat::R16G16B16A16_SFLOAT)
		->add_stage(ShaderStage::VERTEX, composition_vertex_code.data(), composition_vertex_code.size())
		->add_stage(ShaderStage::FRAGMENT, composition_fragment_code.data(), composition_fragment_code.size())
		->set_layout(composition_layout)
		->build();

	UNUSED(deferred_shader);
	UNUSED(composition_shader);

	std::vector<Vertex> triangle = {
		{ {-0.5, -0.5, 0}, {0.3, 1.0, 1.0}, { 1.0, 0.0 } },
    	{ { 0.5, -0.5, 0}, {0.5, 1.0, 1.0}, { 0.0, 0.0 } },
    	{ { 0.5, 0.5, 0}, {1.0, 0.5, 0.2 }, { 0.0, 1.0 } },
		{ {-0.5, 0.5, 0}, {1.0, 1.0, 1.0}, { 1.0, 1.0 } },
	};

	std::vector<uint32_t> indicies = {
		0, 1, 2, 2, 3, 0
	};

	auto vbo_handle = render_api->create_buffer();
	auto ibo_handle = render_api->create_buffer();

	render_api->buffer_data(vbo_handle, BufferType::VERTEX, triangle.size() * sizeof(Vertex), triangle.data());
	render_api->buffer_data(ibo_handle, BufferType::INSTANCE, indicies.size() * sizeof(uint32_t), indicies.data());

	render_api->buffer_sub_data(vbo_handle, 0, triangle.size() * sizeof(Vertex), triangle.data());

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
		}

		if(minimized) {
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			continue;
		}

		static CameraData camera;
		update_camera(&camera, &context);

		static SceneData scene_data = {};
		scene_data.view = glm::lookAt(camera.position, camera.position + camera.front, camera.up);
		scene_data.projection = glm::perspective(glm::radians(70.f), static_cast<float>(context.width) / static_cast<float>(context.height), 0.1f, 1000.0f);
		scene_data.projection[1][1] *= -1;
		scene_data.color = glm::vec3(1);
		render_api->buffer_sub_data(resources.scene_buffer, 0, sizeof(SceneData), &scene_data);

		static StorageData storage_data[1024] = {};
		storage_data[0].model = calculate_model_matrix(glm::vec3(0, 0, 0), glm::vec3(0, 0, 0), glm::vec3(1));
		render_api->buffer_sub_data(resources.storage_buffer, 0, sizeof(StorageData) * 1024, &storage_data);

		static CompositionData composition_data = {};
		render_api->buffer_sub_data(resources.composition_buffer, 0, sizeof(CompositionData), &composition_data);

		render_api->begin();
		{
			render_api->clear(0, 0, 0, 1);

			// Offscreen/Deferred rendering (first pass)
			std::vector<SubpassAttachment> deferred_attachments = {
				{
					.texture = renderpass_resources.position,
					.view = renderpass_resources.position_view,
					.type = AttachmentType::COLOR,
				},
				{
					.texture = renderpass_resources.albedo,
					.view = renderpass_resources.albedo_view,
					.type = AttachmentType::COLOR,
					.clear = { {0, 0, 0, 1} }
				},
				{
					.texture = renderpass_resources.depth,
					.view = renderpass_resources.depth_view,
					.type = AttachmentType::DEPTH,
					.clear = { .rgba = { 1.0f }, .depth = 0.0f, .stencil = 0 }
				}
			};

			std::vector<UniformBind> deferred_binds = {
				{
					.buffer = {
						.buffer_handle = resources.scene_buffer,
						.offset = 0,
						.range = sizeof(SceneData)
					},
					.type = UniformType::BUFFER,
				},
				{
					.buffer = {
						.buffer_handle = resources.storage_buffer,
						.offset = 0,
						.range = sizeof(StorageData) * 1024
					},
					.type = UniformType::STORAGE,
				},
				{
					.texture = {
						.texture_view_handle = missing_texture_view_handle,
						.sampler_handle = missing_texture_sampler_handle
					},
					.type = UniformType::TEXTURE
				},
			};

			render_api->begin_pass(deferred_attachments);
				render_api->viewport(static_cast<float>(context.width), static_cast<float>(context.height));
				render_api->scissor(context.width, context.height);
				render_api->bind_uniform(deferred_layout, deferred_binds);
				render_api->bind_shader(deferred_shader);
				render_api->bind_buffer(vbo_handle, BindBufferType::VERTEX);
				render_api->bind_buffer(ibo_handle, BindBufferType::INSTANCE);
				render_api->draw_instanced(static_cast<uint32_t>(indicies.size()), 1, 0);
				render_api->draw_instanced(static_cast<uint32_t>(indicies.size()), 1, 1);
				render_api->draw_instanced(static_cast<uint32_t>(indicies.size()), 1, 2);
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
					.texture = {
						.texture_view_handle = renderpass_resources.position_view,
						.sampler_handle = resources.position_sampler
					},
					.type = UniformType::TEXTURE,
				},
				{
					.texture = {
						.texture_view_handle = renderpass_resources.albedo_view,
						.sampler_handle = resources.albedo_sampler
					},
					.type = UniformType::TEXTURE,
				}
			};

			render_api->begin_pass(composition_attachments);
				render_api->viewport(static_cast<float>(context.width), static_cast<float>(context.height));
				render_api->scissor(context.width, context.height);
				render_api->bind_uniform(composition_layout, composition_binds);
				render_api->bind_shader(composition_shader);
				render_api->draw(3, 1);
			render_api->end_pass(composition_attachments);

			render_api->show_image(resources.pass_handles->composition);

			render_api->ui()->begin();
				ImGui::NewFrame();

				ImGui::Begin("Selection");
				ImGui::SliderInt("GBuffer Selection", (int*)&composition_data.gbuffer_selection, 0, 1);
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

void setup_resources(AppContext *context, RenderAPI *render_api, RenderResources *resources) {
	UNUSED(context);
	resources->pass_handles->position = render_api->create_texture();
	resources->pass_handles->albedo = render_api->create_texture();
	resources->pass_handles->depth = render_api->create_texture();
	resources->pass_handles->composition = render_api->create_texture();
	resources->pass_handles->position_view = render_api->create_texture_view();
	resources->pass_handles->albedo_view = render_api->create_texture_view();
	resources->pass_handles->depth_view = render_api->create_texture_view();
	resources->pass_handles->composition_view = render_api->create_texture_view();
	resources->position_sampler = render_api->create_sampler();
	resources->albedo_sampler = render_api->create_sampler();
	resources->scene_buffer = render_api->create_buffer();
	resources->storage_buffer = render_api->create_buffer();
	resources->composition_buffer = render_api->create_buffer();

	render_api->sampler(
		resources->position_sampler,
		SamplerAddressMode::CLAMP_BORDER,
		SamplerAddressMode::CLAMP_BORDER,
		SamplerAddressMode::CLAMP_BORDER
	);

	render_api->sampler(
		resources->albedo_sampler,
		SamplerAddressMode::CLAMP_BORDER,
		SamplerAddressMode::CLAMP_BORDER,
		SamplerAddressMode::CLAMP_BORDER
	);

	render_api->buffer_data(
		resources->scene_buffer,
		BufferType::UNIFORM,
		sizeof(SceneData),
		nullptr
	);

	render_api->buffer_data(
		resources->storage_buffer,
		BufferType::STORAGE,
		sizeof(StorageData) * 1024,
		nullptr
	);

	render_api->buffer_data(
		resources->composition_buffer,
		BufferType::UNIFORM,
		sizeof(CompositionData),
		nullptr
	);
}

void setup_pass_resources(AppContext *context, RenderAPI *renderer, RenderPassResources *resources) {
	renderer->texture_data(
		resources->position,
		ImageDimensions::IMAGE_2D,
		ImageSamples::SAMPLE_COUNT_1_BIT,
		ImageFormat::R16G16B16A16_SFLOAT,
		ImageFlags::COLOR_ATTACHMENT | ImageFlags::SAMPLED,
		nullptr,
		context->width, context->height, 1
	);

	renderer->texture_data(
		resources->albedo,
		ImageDimensions::IMAGE_2D,
		ImageSamples::SAMPLE_COUNT_1_BIT,
		ImageFormat::R8G8B8A8_UNORM,
		ImageFlags::COLOR_ATTACHMENT | ImageFlags::SAMPLED,
		nullptr,
		context->width, context->height, 1
	);

	renderer->texture_data(
		resources->depth,
		ImageDimensions::IMAGE_2D,
		ImageSamples::SAMPLE_COUNT_1_BIT,
		ImageFormat::D32_SFLOAT,
		ImageFlags::DEPTH_ATTACHMENT,
		nullptr,
		context->width, context->height, 1
	);

	renderer->texture_data(
		resources->composition,
		ImageDimensions::IMAGE_2D,
		ImageSamples::SAMPLE_COUNT_1_BIT,
		ImageFormat::R16G16B16A16_SFLOAT,
		ImageFlags::COLOR_ATTACHMENT,
		nullptr,
		context->width, context->height, 1
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

void update_camera(CameraData *camera, AppContext *context) {
	static glm::vec3 front 		= glm::vec3(0.0f, 0.0f, -1.0f);
	static glm::vec3 up 		= glm::vec3(0.0f, 1.0f, 0.0f);
	static glm::vec3 right 		= glm::normalize(glm::cross(front, up));

	camera->front = front;
	camera->up = up;
	camera->right = right;

	int mx = 0, my = 0;

	SDL_PumpEvents();
	const Uint32 mouse_state = SDL_GetMouseState(&mx, &my);
	const Uint8* key_state = SDL_GetKeyboardState(NULL);

	static float pitch = 0.0f;
	static float yaw = -90.0f;
	const float sensitivity = 0.1f;

	static float last_mx = 400.0f, last_my = 300.0f;

	float offset_mx = (float)mx - last_mx;
	float offset_my = last_my - (float)my;

	last_mx = static_cast<float>(mx);
	last_my = static_cast<float>(my);

	if(mouse_state & SDL_BUTTON(3)) {
		offset_mx *= sensitivity;
		offset_my *= sensitivity;

		yaw += offset_mx;
		pitch += offset_my;
	}

	camera->direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	camera->direction.y = sin(glm::radians(pitch));
	camera->direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

	camera->front = glm::normalize(camera->direction);
	camera->right = glm::normalize(glm::cross(camera->direction, camera->up));

	float camera_speed = 0.3f;

	if(key_state[SDL_SCANCODE_LSHIFT])
		camera_speed *= 6.0f;

	if(key_state[SDL_SCANCODE_W])
		camera->position += camera->front * (context->time_delta * camera_speed);

	if(key_state[SDL_SCANCODE_S])
		camera->position -= camera->front * (context->time_delta * camera_speed);

	if(key_state[SDL_SCANCODE_D])
		camera->position += camera->right * (context->time_delta * camera_speed);

	if(key_state[SDL_SCANCODE_A])
		camera->position -= camera->right * (context->time_delta * camera_speed);
}