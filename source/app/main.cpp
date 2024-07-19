struct Vertex {
	glm::vec3 position;
	glm::vec3 color;
};

struct SceneData {
	glm::mat4 projection;
	glm::mat4 view;
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

	auto filesystem = get_factory<Filesystem*>("filesystem_std");
	if(!filesystem.good) {
		spdlog::error("Couldn't load VFS");
		return 0;
	}

	filesystem->mount("assets/", "../assets/");
	filesystem->mount("assets/models/", "../assets/models/");
	filesystem->mount("assets/fonts/", "../assets/fonts/");
	filesystem->mount("assets/textures/", "../assets/textures/");
	filesystem->mount("assets/shaders/", "./assets/shaders/");

	auto renderer = get_factory<RenderAPI*>("api_vk", &context);
	if(!renderer.good) {
		spdlog::error("Couldn't load renderer");
		return 0;
	}

	auto position_image = renderer->create_texture();
	auto albedo_image = renderer->create_texture();
	auto depth_image = renderer->create_texture();
	auto composition_image = renderer->create_texture();

	auto position_image_view = renderer->create_texture_view();
	auto albedo_image_view = renderer->create_texture_view();
	auto depth_image_view = renderer->create_texture_view();
	auto composition_image_view = renderer->create_texture_view();

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
	renderer_setup_pass_resources(&context, renderer.interface, &resize_handles);
	renderer->on_resize([&](RenderAPI* renderer) {
		renderer_setup_pass_resources(&context, renderer, &resize_handles);
	});

	auto global_layout = renderer->create_layout()
		->add_uniform(ShaderStage::VERTEX)
		->add_uniform(ShaderStage::VERTEX)
		->build();

	auto composition_layout = renderer->create_layout()
		->add_uniform(ShaderStage::FRAGMENT) // Position
		->add_uniform(ShaderStage::FRAGMENT) // Albedo
		->add_uniform(ShaderStage::FRAGMENT) // Light Data
		->build();

	UNUSED(global_layout);
	UNUSED(composition_layout)

	auto triangle_vertex_code = filesystem->read_file<char>("assets/shaders/triangle.vert.spv", true);
	auto triangle_fragment_code = filesystem->read_file<char>("assets/shaders/triangle.frag.spv", true);
	auto triangle_shader = renderer->create_graphic_shader()
		->add_attachment(ImageFormat::R16G16B16A16_SFLOAT)
		->set_depth_format(ImageFormat::D32_SFLOAT)
		->add_binding(0, sizeof(Vertex), BindingRate::VERTEX)
		->add_attribute(0, 0, offsetof(Vertex, position), AttributeType::VEC3D_SIGNED)
		->add_attribute(1, 0, offsetof(Vertex, color), AttributeType::VEC3D_SIGNED)
		->add_stage(ShaderStage::VERTEX, triangle_vertex_code.data(), triangle_vertex_code.size())
		->add_stage(ShaderStage::FRAGMENT, triangle_fragment_code.data(), triangle_fragment_code.size())
		->build();

	auto deferred_vertex_code = filesystem->read_file<char>("assets/shaders/deferred.vert.spv", true);
	auto deferred_fragment_code = filesystem->read_file<char>("assets/shaders/deferred.frag.spv", true);
	auto deferred_shader = renderer->create_graphic_shader()
		->add_attachment(ImageFormat::R16G16B16A16_SFLOAT)
		->add_attachment(ImageFormat::R8G8B8A8_UNORM)
		->set_depth_format(ImageFormat::D32_SFLOAT)
		->set_depth_test(true, ShaderCompareOp::GREATER_OR_EQUAL)
		->add_binding(0, sizeof(Vertex), BindingRate::VERTEX)
		->add_attribute(0, 0, offsetof(Vertex, position), AttributeType::VEC3D_SIGNED)
		->add_attribute(1, 0, offsetof(Vertex, color), AttributeType::VEC3D_SIGNED)
		->add_stage(ShaderStage::VERTEX, deferred_vertex_code.data(), deferred_vertex_code.size())
		->add_stage(ShaderStage::FRAGMENT, deferred_fragment_code.data(), deferred_fragment_code.size())
		->add_layout(global_layout)
		->build();

	auto composition_vertex_code = filesystem->read_file<char>("assets/shaders/composition.vert.spv", true);
	auto composition_fragment_code = filesystem->read_file<char>("assets/shaders/composition.frag.spv", true);
	auto composition_shader = renderer->create_graphic_shader()
		->add_attachment(ImageFormat::R16G16B16A16_SFLOAT)
		->add_stage(ShaderStage::VERTEX, composition_vertex_code.data(), composition_vertex_code.size())
		->add_stage(ShaderStage::FRAGMENT, composition_fragment_code.data(), composition_fragment_code.size())
		->add_layout(composition_layout)
		->build();

	UNUSED(triangle_shader);
	UNUSED(deferred_shader);
	UNUSED(composition_shader);

	std::vector<Vertex> triangle = {
		{ {-0.5, -0.5, 0}, {0.3, 1.0, 1.0} },
    	{ { 0.5, -0.5, 0}, {0.5, 1.0, 1.0} },
    	{ { 0.5, 0.5, 0}, {1.0, 0.5, 0.2 } },
		{ {-0.5, 0.5, 0}, {1.0, 1.0, 1.0} },
	};

	auto vbo_handle = renderer->create_buffer();
	renderer->buffer_data(vbo_handle, BufferType::VERTEX, triangle.size() * sizeof(Vertex), triangle.data());

	std::vector<uint32_t> indicies = {
		0, 1, 2, 2, 3, 0
	};

	auto ibo_handle = renderer->create_buffer();
	renderer->buffer_data(ibo_handle, BufferType::INSTANCE, indicies.size() * sizeof(uint32_t), indicies.data());

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

		renderer->begin();
		{
			renderer->clear(0, 0, 0, 0);

			// Offscreen/Deferred rendering (first pass)
			std::vector<SubpassAttachment> deferred_attachments = {
				{ .texture = position_image, .view = position_image_view, .type = AttachmentType::COLOR, .clear = { 0, 0, 0, 1 } },
				{ .texture = albedo_image, .view = albedo_image_view,  .type = AttachmentType::COLOR, .clear = { 0, 0, 0, 1 } },
				{ .texture = depth_image, .view = depth_image_view,  .type = AttachmentType::DEPTH, .clear = { 0, 0, 0, 0, 1.0f, 0 } }
			};

			SubpassDependencyInfo deferred_info = {
				.attachments = deferred_attachments.data(),
				.count = static_cast<uint32_t>(deferred_attachments.size())
			};

			renderer->begin_pass(&deferred_info);
				renderer->viewport(static_cast<float>(context.width), static_cast<float>(context.height));
				renderer->scissor(context.width, context.height);
				renderer->bind_graphic_shader(deferred_shader);
				renderer->bind_buffer(vbo_handle, BindBufferType::VERTEX);
				renderer->bind_buffer(ibo_handle, BindBufferType::INSTANCE);
				renderer->draw_instanced(static_cast<uint32_t>(indicies.size()), 1, 0);
			renderer->end_pass(&deferred_info);

			// Composed pass (second pass)
			std::vector<SubpassAttachment> composition_attachments = {
				{ .texture = composition_image, .view = composition_image_view, .type = AttachmentType::COLOR, .clear = { 0, 0, 0, 1 } },
			};

			SubpassDependencyInfo composition_info = {
				.attachments = composition_attachments.data(),
				.count = static_cast<uint32_t>(composition_attachments.size())
			};

			renderer->begin_pass(&composition_info);
				renderer->viewport(static_cast<float>(context.width), static_cast<float>(context.height));
				renderer->scissor(context.width, context.height);
				renderer->bind_graphic_shader(composition_shader);
				renderer->draw(3, 1);
			renderer->end_pass(&composition_info);

			renderer->show_image(composition_image);

			static RenderUI *ui = renderer->ui();
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