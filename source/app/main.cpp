#include <math.h>

#include <glm/glm.hpp>

struct Vertex {
	glm::vec3 position;
	union {
		glm::vec3 color;
		glm::vec3 colour;
	};
	glm::vec3 normal;
	glm::vec3 tex;
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

	auto image_handle = renderer->create_image();

	std::array<uint32_t, 16 * 16> pixels;

	uint32_t black = glm::packUnorm4x8(glm::vec4(0, 0, 0, 0));
	uint32_t magenta = glm::packUnorm4x8(glm::vec4(1, 0, 1, 1));

	for(int x = 0; x < 16; x++)
		for(int y = 0; y < 16; y++)
			pixels[ y* 16 + x ] = ((x % 2) ^ (y % 2)) ? magenta : black;

	renderer->image_data(
		image_handle,
		ImageDimensions::IMAGE_2D,
		ImageSamples::SAMPLE_COUNT_1_BIT,
		ImageFormat::R8G8B8A8_UNORM,
		pixels.data(), false,
		16, 16, 1
	);

	auto forward_image = renderer->create_image();
	renderer->image_data(
		forward_image,
		ImageDimensions::IMAGE_2D,
		ImageSamples::SAMPLE_COUNT_1_BIT,
		ImageFormat::R16G16B16A16_SFLOAT,
		nullptr, false,
		context.width, context.height, 1
	);

	renderer->on_resize([&](AbstractRenderer* renderer) {
		renderer->image_data(
			forward_image,
			ImageDimensions::IMAGE_2D,
			ImageSamples::SAMPLE_COUNT_1_BIT,
			ImageFormat::R16G16B16A16_SFLOAT,
			nullptr, false,
			context.width, context.height, 1
		);
	});

	auto vertex_shader_code = filesystem->read_file<char>("assets/shaders/triangle.vert.spv", true);
	auto fragment_shader_code = filesystem->read_file<char>("assets/shaders/triangle.frag.spv", true);

	auto shader = renderer->create_graphic_shader()
		->add_binding(0, sizeof(Vertex), BindingRate::RATE_INDEX)
		->add_attribute(0, 0, offsetof(Vertex, position), AttributeType::VEC3D_SIGNED)
		->add_attribute(1, 0, offsetof(Vertex, color), AttributeType::VEC3D_SIGNED)
		->add_attribute(2, 0, offsetof(Vertex, normal), AttributeType::VEC3D_SIGNED)
		->add_stage(ShaderStage::STAGE_VERTEX, vertex_shader_code.data(), vertex_shader_code.size())
		->add_stage(ShaderStage::STAGE_FRAGMENT, fragment_shader_code.data(), fragment_shader_code.size())
		->init();

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

		renderer->clear(0, 0, 0, 0);

		static ResourceHandle handles[] = { forward_image };
		static AttachmentType types[] = { AttachmentType::COLOR };
		static SubpassDependency forward_dependency = {
			.attachments = handles,
			.types = types,
			.count = 1
		};

		renderer->begin_pass(&forward_dependency);
		renderer->viewport(static_cast<float>(context.width), static_cast<float>(context.height));
		renderer->scissor(context.width, context.height);
		renderer->end_pass(&forward_dependency);

		renderer->ui()->begin_ui();
		renderer->ui()->show_demo_window();
		renderer->ui()->end_ui();

		renderer->end();

		renderer->present();
	}

	renderer.release();
	filesystem.release();
	SDL_Quit();

	return 0;
}
