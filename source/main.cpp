struct gameGlobals {
	vkb::Instance instance;
	VkSurfaceKHR surface;
	vkb::Device device;
	vkb::Swapchain swapchain;

	VkQueue graphics_queue;
	VkQueue present_queue;

	std::vector<VkImage> swapchain_images;
	std::vector<VkImageView> swapchain_image_views;
	std::vector<VkFramebuffer> framebuffers;

	VkRenderPass render_pass;
	VkPipelineLayout pipeline_layout;
	VkPipeline graphics_pipeline;

	VkCommandPool command_pool;
	std::vector<VkCommandBuffer> command_buffers;

	std::vector<VkSemaphore> available_semaphores;
	std::vector<VkSemaphore> finished_semaphore;
	std::vector<VkFence> in_flight_fences;
	std::vector<VkFence> image_in_flight;
	size_t current_frame = 0;

	SDL_Window *window;
};

bool create_window(gameGlobals *game) {
	game->window = SDL_CreateWindow("Epico", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_SHOWN | SDL_WINDOW_VULKAN );
	if(game->window == NULL) {
		fmt::println("Failed to create SDL Window {}", SDL_GetError());
		return 0;
	}

	return 1;
}

bool create_vk_instance(gameGlobals *game) {
	vkb::InstanceBuilder instance_builder;
	auto instance_builder_ret = instance_builder
									.set_app_name("Epico")
									.set_engine_name("Epico Engine")
									.require_api_version(1,0,0)
									.build();

	if(!instance_builder_ret) {
		fmt::println("Failed to create Vulkan instance {}", instance_builder_ret.error().message());
		return 0;
	}

	game->instance = instance_builder_ret.value();

	return 1;
}

bool create_surface(gameGlobals *game) {
	if (!SDL_Vulkan_CreateSurface(game->window, game->instance.instance, &game->surface)) {
		fmt::println("Failed to create SDL Surface {}", SDL_GetError());
		return 0;
	}

	return 1;
}

bool create_device(gameGlobals *game) {
	vkb::PhysicalDeviceSelector device_selector(game->instance);
	auto device_selector_ret = device_selector
								.set_surface(game->surface)
								.select();

	if(!device_selector_ret) {
		fmt::println("Failed to create Vulkan Device Selector {}", device_selector_ret.error().message());
		return 0;
	}

	vkb::DeviceBuilder device_builder{ device_selector_ret.value() };
	auto device_builder_ret = device_builder.build();

	if(!device_builder_ret) {
		fmt::println("Failed to create Vulkan Device {}", device_builder_ret.error().message());
		return 0;
	}

	game->device = device_builder_ret.value(); // ze bluechowth dewice has connectedey suchessfulley

	return 1;
}

bool create_swapchain(gameGlobals *game) {
	vkb::SwapchainBuilder swapchain_builder{ game->device };
	auto swapchain_builder_ret = swapchain_builder.build();

	if(!swapchain_builder_ret) {
		fmt::println("Failed to create Vulkan Swapchain {}", swapchain_builder_ret.error().message());
		return 0;
	}

	game->swapchain = swapchain_builder_ret.value();

	return 1;
}

bool get_queues(gameGlobals *game) {
	auto gq = game->device.get_queue(vkb::QueueType::graphics);
	auto pq = game->device.get_queue(vkb::QueueType::present);

	if (!gq.has_value()) {
		fmt::println("Failed to create Get Vulkan Graphics Queue");
		return 0;
	}

	if (!pq.has_value()) {
		fmt::println("Failed to create Get Vulkan Present Queue");
		return 0;
	}

	game->graphics_queue = gq.value();
	game->present_queue = pq.value();

	return 1;
}

bool create_render_pass(gameGlobals *game) {
	VkAttachmentDescription color_attachment = {};
	color_attachment.format = game->swapchain.image_format;
	color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference color_attachment_ref = {};
	color_attachment_ref.attachment = 0;
	color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &color_attachment_ref;

	VkSubpassDependency dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo render_pass_info = {};
	render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	render_pass_info.attachmentCount = 1;
	render_pass_info.pAttachments = &color_attachment;
	render_pass_info.subpassCount = 1;
	render_pass_info.pSubpasses = &subpass;
	render_pass_info.dependencyCount = 1;
	render_pass_info.pDependencies = &dependency;

	if(vkCreateRenderPass(game->device, &render_pass_info, nullptr, &game->render_pass) != VK_SUCCESS) {
		fmt::println("Failed to create render pass");
		return 0;
	}

	return 1;
}

int main(int argc, char *args[]) {
	gameGlobals game;

	if(SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		fmt::println("SDL Failed to init with error: {}", SDL_GetError());
		return 0;
	}

	if(!create_window(&game)) 		return 0;

	if(!create_vk_instance(&game)) 	return 0;
	if(!create_surface(&game)) 		return 0;
	if(!create_device(&game)) 		return 0;
	if(!create_swapchain(&game)) 	return 0;
	if(!get_queues(&game))			return 0;
	if(!create_render_pass(&game))	return 0;

	fmt::println("Epico instance: {}", 	(void*)game.instance.instance);
	fmt::println("Epico dewice: {}", 	(void*)game.device.device);
	fmt::println("Epico swapchain: {}", (void*)game.swapchain.swapchain);

	static bool quit = false;
	while(!quit) {
		SDL_Event e;
		while(SDL_PollEvent(&e)) {
			if(e.type == SDL_QUIT)
				quit = true;
		}
	}

	vkb::destroy_swapchain(game.swapchain);
	vkb::destroy_device(game.device);
	vkb::destroy_instance(game.instance);

	SDL_DestroyWindow(game.window);
	SDL_Quit();

	return 0;
}