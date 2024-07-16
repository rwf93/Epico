struct Vertex {
	glm::vec3 position;
	glm::vec3 color;
};

struct SceneData {
	glm::mat4 projection;
	glm::mat4 view;
};

int main(int argc, char *argv[]) {
	UNUSED(argc);
	UNUSED(argv);

	AppContext context;
	context.width = 1280;
	context.height = 762;

	if(SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		spdlog::error("Couldn't init SDL: {}", SDL_GetError());
		return 0;
	};

	auto filesystem = get_factory<AbstractFilesystem*>("filesystem_std");
	if(!filesystem.good) {
		spdlog::error("Couldn't load VFS");
		return 0;
	}

	filesystem->mount("assets/", "../assets/");
	filesystem->mount("assets/models/", "../assets/models/");
	filesystem->mount("assets/fonts/", "../assets/fonts/");
	filesystem->mount("assets/textures/", "../assets/textures/");
	filesystem->mount("assets/shaders/", "./assets/shaders/");

	auto renderer = get_factory<AbstractRenderer*>("renderer_vk", &context);
	if(!renderer.good) {
		spdlog::error("Couldn't load renderer");
		return 0;
	}

	auto forward_image = renderer->create_image();
	renderer->image_data(
		forward_image,
		ImageDimensions::IMAGE_2D,
		ImageSamples::SAMPLE_COUNT_1_BIT,
		ImageFormat::R16G16B16A16_SFLOAT,
		ImageFlags::IMAGE_COLOR_ATTACHMENT,
		nullptr,
		context.width, context.height, 1
	);

	auto depth_image = renderer->create_image();
	renderer->image_data(
		depth_image,
		ImageDimensions::IMAGE_2D,
		ImageSamples::SAMPLE_COUNT_1_BIT,
		ImageFormat::D32_SFLOAT,
		ImageFlags::IMAGE_DEPTH_ATTACHMENT,
		nullptr,
		context.width, context.height, 1
	);

	renderer->on_resize([&](AbstractRenderer* renderer) {
		renderer->image_data(
			forward_image,
			ImageDimensions::IMAGE_2D,
			ImageSamples::SAMPLE_COUNT_1_BIT,
			ImageFormat::R16G16B16A16_SFLOAT,
			ImageFlags::IMAGE_COLOR_ATTACHMENT,
			nullptr,
			context.width, context.height, 1
		);

		renderer->image_data(
			depth_image,
			ImageDimensions::IMAGE_2D,
			ImageSamples::SAMPLE_COUNT_1_BIT,
			ImageFormat::D32_SFLOAT,
			ImageFlags::IMAGE_DEPTH_ATTACHMENT,
			nullptr,
			context.width, context.height, 1
		);
	});

	auto vertex_shader_code = filesystem->read_file<char>("assets/shaders/triangle.vert.spv", true);
	auto fragment_shader_code = filesystem->read_file<char>("assets/shaders/triangle.frag.spv", true);

	auto shader = renderer->create_graphic_shader()
		->add_attachment(ImageFormat::R16G16B16A16_SFLOAT)
		->set_depth_format(ImageFormat::D32_SFLOAT)
		->add_binding(0, sizeof(Vertex), BindingRate::RATE_VERTEX)
		->add_attribute(0, 0, offsetof(Vertex, position), AttributeType::VEC3D_SIGNED)
		->add_attribute(1, 0, offsetof(Vertex, color), AttributeType::VEC3D_SIGNED)
		->add_stage(ShaderStage::STAGE_VERTEX, vertex_shader_code.data(), vertex_shader_code.size())
		->add_stage(ShaderStage::STAGE_FRAGMENT, fragment_shader_code.data(), fragment_shader_code.size())
		->add_uniform(0, 0, UniformType::UNIFORM_BUFFER, ShaderStage::STAGE_VERTEX)
		->build();

	std::vector<Vertex> triangle = {
		{ {-0.5, -0.5, 0}, {0.3, 1.0, 1.0} },
    	{ { 0.5, -0.5, 0}, {0.5, 1.0, 1.0} },
    	{ { 0.5, 0.5, 0}, {1.0, 0.5, 0.2 } },
		{ {-0.5, 0.5, 0}, {1.0, 1.0, 1.0} },
	};

	auto vbo_handle = renderer->create_buffer();
	renderer->buffer_data(vbo_handle, BufferType::BUFFER_VERTEX, triangle.size() * sizeof(Vertex), triangle.data());

	std::vector<uint32_t> indicies = {
		0, 1, 2, 2, 3, 0
	};

	auto ibo_handle = renderer->create_buffer();
	renderer->buffer_data(ibo_handle, BufferType::BUFFER_INSTANCE, indicies.size() * sizeof(uint32_t), indicies.data());

	auto scene_data_handle = renderer->create_buffer();
	renderer->buffer_data(scene_data_handle, BufferType::BUFFER_UNIFORM, sizeof(SceneData), nullptr);

	auto scene_data_uniform_handle = renderer->create_uniform_buffer();
	renderer->init_uniform_buffer(scene_data_uniform_handle, scene_data_handle);

	static bool quit = false;
	static bool minimized = false;
	while(!quit) {
		SDL_Event event;
		while(SDL_PollEvent(&event)) {
			if(event.type == SDL_QUIT) quit = true;

			if(event.window.event == SDL_WINDOWEVENT_MINIMIZED)
				minimized = true;

			if(event.window.event == SDL_WINDOWEVENT_RESTORED)
				minimized = false;

			renderer->ui()->process_event(&event);
		}

		if(minimized) {
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			continue;
		}

		static SceneData scene_data = {};
		renderer->buffer_sub_data(scene_data_handle, 0, sizeof(SceneData), &scene_data);

		renderer->begin();
		{
			renderer->clear(0, 0, 0, 0);

			{
				static std::vector<SubpassAttachment> attachments = {
					{ .resource = forward_image, .type = AttachmentType::COLOR, .clear = { 0, 0, 0, 1 } },
					{ .resource = depth_image,   .type = AttachmentType::DEPTH, .clear = { 0, 0, 0, 0, 1.0f, 0 } }
				};

				static SubpassDependencyInfo forward_dependency = {
					.attachments = attachments.data(),
					.count = static_cast<uint32_t>(attachments.size())
				};

				renderer->begin_pass(&forward_dependency);
					renderer->viewport(static_cast<float>(context.width), static_cast<float>(context.height));
					renderer->scissor(context.width, context.height);
					renderer->bind_graphic_shader(shader);
					renderer->bind_buffer(vbo_handle, BindBufferType::BIND_VERTEX);
					renderer->bind_buffer(ibo_handle, BindBufferType::BIND_INSTANCE);
					renderer->draw_instanced(static_cast<uint32_t>(indicies.size()), 1, 0);
				renderer->end_pass(&forward_dependency);
			}

			renderer->show_image(forward_image);

			static AbstractUI *ui = renderer->ui();
			ui->begin_ui();
				ui->show_demo_window();
				ui->begin("Shader Picker");
				ui->end();
			ui->end_ui();
		}
		renderer->end();
		renderer->present();
	}

	renderer.release();
	filesystem.release();
	SDL_Quit();

	return 0;
}
