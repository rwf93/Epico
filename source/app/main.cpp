struct Vertex {
	glm::vec3 position;
	glm::vec3 color;
};

struct SceneData {
	glm::mat4 view;
	glm::mat4 projection;
	glm::vec3 color;
};

struct StorageData {
	glm::mat4 model;
};

struct RendererPassHandles {
	TextureHandle position;
	TextureHandle albedo;
	TextureHandle depth;
	TextureHandle composition;

	TextureViewHandle position_view;
	TextureViewHandle albedo_view;
	TextureViewHandle depth_view;
	TextureViewHandle composition_view;
};

glm::mat4 calculate_model_matrix(glm::vec3 translation, glm::vec3 rotation, glm::vec3 scale) {
	glm::mat4 translation_matrix = glm::translate(glm::mat4(1.0f), translation);
	glm::mat4 rotation_matrix = glm::toMat4(glm::quat(rotation));
	glm::mat4 scale_matrix = glm::scale(glm::mat4(1.0f), scale);

	return translation_matrix * rotation_matrix * scale_matrix;
}

void renderer_setup_pass_resources(AppContext *context, RenderAPI *renderer, RendererPassHandles *resources);

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

	auto position_image = render_api->create_texture();
	auto albedo_image = render_api->create_texture();
	auto depth_image = render_api->create_texture();
	auto composition_image = render_api->create_texture();

	auto position_image_view = render_api->create_texture_view();
	auto albedo_image_view = render_api->create_texture_view();
	auto depth_image_view = render_api->create_texture_view();
	auto composition_image_view = render_api->create_texture_view();

	RendererPassHandles resize_handles = {
		.position = position_image,
		.albedo = albedo_image,
		.depth = depth_image,
		.composition = composition_image,
		.position_view = position_image_view,
		.albedo_view = albedo_image_view,
		.depth_view = depth_image_view,
		.composition_view = composition_image_view
	};

	// Initalizes the subpass dependencies, resizing recreates those dependencies.
	renderer_setup_pass_resources(&context, render_api.interface, &resize_handles);
	render_api->on_resize([&](RenderAPI* renderer) {
		renderer_setup_pass_resources(&context, renderer, &resize_handles);
	});

	auto global_layout = render_api->create_layout()
		->add_uniform(ShaderStage::VERTEX, UniformType::BUFFER)
		->add_uniform(ShaderStage::VERTEX, UniformType::STORAGE)
		->build();

	UNUSED(global_layout);

	auto deferred_vertex_code = filesystem->read_file<char>("assets/shaders/deferred.vert.spv", true);
	auto deferred_fragment_code = filesystem->read_file<char>("assets/shaders/deferred.frag.spv", true);
	auto deferred_shader = render_api->create_graphic_program()
		->add_attachment(ImageFormat::R16G16B16A16_SFLOAT)
		->add_attachment(ImageFormat::R8G8B8A8_UNORM)
		->set_depth_format(ImageFormat::D32_SFLOAT)
		->set_depth_test(true, ShaderCompareOp::GREATER_OR_EQUAL)
		->add_binding(0, sizeof(Vertex), BindingRate::VERTEX)
		->add_attribute(0, 0, offsetof(Vertex, position), AttributeType::VEC3D_SIGNED)
		->add_attribute(1, 0, offsetof(Vertex, color), AttributeType::VEC3D_SIGNED)
		->add_stage(ShaderStage::VERTEX, deferred_vertex_code.data(), deferred_vertex_code.size())
		->add_stage(ShaderStage::FRAGMENT, deferred_fragment_code.data(), deferred_fragment_code.size())
		->set_layout(global_layout)
		->build();

	auto composition_vertex_code = filesystem->read_file<char>("assets/shaders/composition.vert.spv", true);
	auto composition_fragment_code = filesystem->read_file<char>("assets/shaders/composition.frag.spv", true);
	auto composition_shader = render_api->create_graphic_program()
		->add_attachment(ImageFormat::R16G16B16A16_SFLOAT)
		->add_stage(ShaderStage::VERTEX, composition_vertex_code.data(), composition_vertex_code.size())
		->add_stage(ShaderStage::FRAGMENT, composition_fragment_code.data(), composition_fragment_code.size())
		->build();

	UNUSED(deferred_shader);
	UNUSED(composition_shader);

	std::vector<Vertex> triangle = {
		{ {-0.5, -0.5, 0}, {0.3, 1.0, 1.0} },
    	{ { 0.5, -0.5, 0}, {0.5, 1.0, 1.0} },
    	{ { 0.5, 0.5, 0}, {1.0, 0.5, 0.2 } },
		{ {-0.5, 0.5, 0}, {1.0, 1.0, 1.0} },
	};

	std::vector<uint32_t> indicies = {
		0, 1, 2, 2, 3, 0
	};

	auto vbo_handle = render_api->create_buffer();
	auto ibo_handle = render_api->create_buffer();

	render_api->buffer_data(vbo_handle, BufferType::VERTEX, triangle.size() * sizeof(Vertex), triangle.data());
	render_api->buffer_data(ibo_handle, BufferType::INSTANCE, indicies.size() * sizeof(uint32_t), indicies.data());

	auto scene_data_handle = render_api->create_buffer();
	render_api->buffer_data(scene_data_handle, BufferType::UNIFORM, sizeof(SceneData), nullptr);

	auto storage_data_handle = render_api->create_buffer();
	render_api->buffer_data(storage_data_handle, BufferType::STORAGE, sizeof(StorageData) * 1024, nullptr);

	ImGui::SetCurrentContext(static_cast<ImGuiContext*>(render_api->ui()->get_context()));

	clock_t start_time = std::clock();
	float time_delta = 0;
	float time = 0;

	static bool quit = false;
	static bool minimized = false;
	while(!quit) {
		clock_t current_time = std::clock();
		time_delta = static_cast<float>(current_time - start_time) / CLOCKS_PER_SEC;
		time = static_cast<float>(current_time) / CLOCKS_PER_SEC;

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

		glm::vec3 camPos = { 0.f,0.f,-2.f };

		static float frame = 0;
		frame += 0.01f;

		static glm::vec3 camera_position = glm::vec3(0.0f, 0.0f, 0.0f);
		static glm::vec3 camera_front = glm::vec3(0.0f, 0.0f, -1.0f);
		static glm::vec3 camera_up = glm::vec3(0.0f, 1.0f, 0.0f);
		static glm::vec3 camera_right = glm::normalize(glm::cross(camera_front, camera_up));

		{
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

			static glm::vec3 camera_direction = {};

			camera_direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
			camera_direction.y = sin(glm::radians(pitch));
			camera_direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

			camera_front = glm::normalize(camera_direction);
			camera_right = glm::normalize(glm::cross(camera_direction, camera_up));

			float camera_speed = 0.3f;

			if(key_state[SDL_SCANCODE_LSHIFT])
				camera_speed *= 6.0f;

			if(key_state[SDL_SCANCODE_W])
				camera_position += camera_front * (time_delta * camera_speed);

			if(key_state[SDL_SCANCODE_S])
				camera_position -= camera_front * (time_delta * camera_speed);

			if(key_state[SDL_SCANCODE_D])
				camera_position += camera_right * (time_delta * camera_speed);

			if(key_state[SDL_SCANCODE_A])
				camera_position -= camera_right * (time_delta * camera_speed);
		}

		SceneData scene_data = {};
		scene_data.view = glm::lookAt(camera_position, camera_position + camera_front, camera_up);
		scene_data.projection = glm::perspective(glm::radians(70.f), 1700.f / 900.f, 0.1f, 200.0f);
		scene_data.projection[1][1] *= -1;
		scene_data.color = glm::vec3(1);

		render_api->buffer_sub_data(scene_data_handle, 0, sizeof(SceneData), &scene_data);

		static StorageData storage_data[1024] = {};
		static bool init = false;
		if(!init) {
			for(int i = 0; i < 1024; i++) {
				storage_data[i].model = calculate_model_matrix(glm::ballRand(100.0f), glm::vec3(0, 0, 0), glm::vec3(1));
			}
			init = true;
		}

		render_api->buffer_sub_data(storage_data_handle, 0, sizeof(StorageData) * 1024, &storage_data);

		render_api->begin();
		{
			render_api->clear(0, 0, 0, 1);

			// Offscreen/Deferred rendering (first pass)
			std::vector<SubpassAttachment> deferred_attachments = {
				{
					.texture = position_image,
					.view = position_image_view,
					.type = AttachmentType::COLOR,
				},
				{
					.texture = albedo_image,
					.view = albedo_image_view,
					.type = AttachmentType::COLOR,
					.clear = { {0, 0, 0, 1} }
				},
				{
					.texture = depth_image,
					.view = depth_image_view,
					.type = AttachmentType::DEPTH,
					.clear = { .depth = 0.0f, .stencil = 0 }
				}
			};

			std::vector<UniformBind> deferred_binds = {
				{
					.buffer = scene_data_handle,
					.type = UniformType::BUFFER,
					.offset = 0,
					.range = sizeof(SceneData)
				},
				{
					.buffer = storage_data_handle,
					.type = UniformType::STORAGE,
					.offset = 0,
					.range = sizeof(StorageData) * 1024
				},
			};

			render_api->begin_pass(deferred_attachments);
				render_api->viewport(static_cast<float>(context.width), static_cast<float>(context.height));
				render_api->scissor(context.width, context.height);
				render_api->bind_uniform(global_layout, deferred_binds);
				render_api->bind_shader(deferred_shader);
				render_api->bind_buffer(vbo_handle, BindBufferType::VERTEX);
				render_api->bind_buffer(ibo_handle, BindBufferType::INSTANCE);
				for(int i = 0; i < 1024; i++) {
					render_api->draw_instanced(static_cast<uint32_t>(indicies.size()), 1, i);
				}
			render_api->end_pass(deferred_attachments);

			render_api->show_image(position_image);

			render_api->ui()->begin();
				ImGui::NewFrame();

				ImGui::Begin("fart");
				ImGui::End();
				ImGui::ShowDemoWindow();

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

void renderer_setup_pass_resources(AppContext *context, RenderAPI *renderer, RendererPassHandles *resources) {
	renderer->texture_data(
		resources->position,
		ImageDimensions::IMAGE_2D,
		ImageSamples::SAMPLE_COUNT_1_BIT,
		ImageFormat::R16G16B16A16_SFLOAT,
		ImageFlags::COLOR_ATTACHMENT,
		nullptr,
		context->width, context->height, 1
	);

	renderer->texture_data(
		resources->albedo,
		ImageDimensions::IMAGE_2D,
		ImageSamples::SAMPLE_COUNT_1_BIT,
		ImageFormat::R8G8B8A8_UNORM,
		ImageFlags::COLOR_ATTACHMENT,
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