#pragma once

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
	VkPipelineLayout graphics_pipeline_layout;
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

#define FUNC_CREATE_WINDOW gameGlobals *game
#define FUNC_READ_FILE const std::string &filename, bool binary
#define FUNC_CLEANUP gameGlobals *game

bool create_window(FUNC_CREATE_WINDOW);
std::vector<char> read_file(FUNC_READ_FILE = false);
void cleanup(FUNC_CLEANUP);