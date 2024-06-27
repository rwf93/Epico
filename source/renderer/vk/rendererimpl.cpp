#include "vkinstance.h"
#include "vksurface.h"
#include "vkdevice.h"
#include "vkswapchain.h"
#include "vkcommandpool.h"
#include "vkresourcemanager.h"
#include "vkimgui.h"

#include "rendererimpl.h"

VulkanRenderer::VulkanRenderer(AppContext *app_context) {
	this->app_context = app_context;

	VK_CHECK(volkInitialize());

	instance.init(cleanup_queue);
	surface.init(cleanup_queue, app_context, &instance);
	device.init(cleanup_queue, &instance, &surface);
	swapchain.init(cleanup_queue, &device);
	command_pool.init(cleanup_queue, &device, &swapchain);
	resource_manager.init(cleanup_queue, &instance, &device, &command_pool);
	ui_imgui.init(cleanup_queue, &command_pool);

	draw_image = create_image();
	image_data(
		draw_image,
		ImageDimensions::IMAGE_2D,
		ImageSamples::SAMPLE_COUNT_1_BIT,
		ImageFormat::R16G16B16A16_SFLOAT,
		nullptr,
		false,
		app_context->width, app_context->height, 1
	);
}

VulkanRenderer::~VulkanRenderer() {
	device.wait();
	cleanup_queue.destroy();
}

void VulkanRenderer::begin() {
	command_pool.wait_fences();
	command_pool.reset_fences();

	if(swapchain.aquire_next_image(&command_pool))
		rebuild();

	command_pool.begin_recording();

	command_pool.transition_image(
		resource_manager.get_image(draw_image)->get_image(),
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_GENERAL
	);
}

void VulkanRenderer::end() {
	auto image = resource_manager.get_image(draw_image)->get_image();

	command_pool.transition_image(
		image,
		VK_IMAGE_LAYOUT_GENERAL,
		VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
	);

	command_pool.transition_image(
		swapchain.get_swapchain_image(),
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
	);

	command_pool.copy_image(
		image,
		swapchain.get_swapchain_image(),
		VkExtent2D{ .width = app_context->width, .height = app_context->height },
		VkExtent2D{ .width = app_context->width, .height = app_context->height }
	);

	command_pool.transition_image(
		swapchain.get_swapchain_image(),
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
	);

	command_pool.end_recording();

	auto command_info = info::command_buffer_submit_info(command_pool.get_command());
	auto wait_info = info::semaphore_submit_info(
		VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR,
		command_pool.get_available_semaphore()
	);
	auto signal_info = info::semaphore_submit_info(
		VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT,
		command_pool.get_finished_semaphore()
	);

	auto submit_info = info::submit_info(&command_info, &signal_info, &wait_info);

	VK_CHECK(vkQueueSubmit2(device.get_graphics_queue(), 1, &submit_info, command_pool.get_fence()));

	VkPresentInfoKHR present_info = {};
	present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present_info.pSwapchains = &swapchain.get_swapchain().swapchain;
	present_info.swapchainCount = 1;
	present_info.pWaitSemaphores = &command_pool.get_finished_semaphore();
	present_info.waitSemaphoreCount = 1;
	present_info.pImageIndices = &swapchain.get_image_index();

	VkResult present_result = vkQueuePresentKHR(device.get_graphics_queue(), &present_info);
	if(present_result == VK_ERROR_OUT_OF_DATE_KHR || present_result == VK_SUBOPTIMAL_KHR) {
		rebuild();
	}

	command_pool.advance();
}

void VulkanRenderer::clear(float r, float g, float b, float a) {
	VkClearColorValue clear_value = { { r, g, b, a } };

	auto clear_range = info::image_subresource_range(VK_IMAGE_ASPECT_COLOR_BIT);
	vkCmdClearColorImage(
		command_pool.get_command(),
		resource_manager.get_image(draw_image)->get_image(),
		VK_IMAGE_LAYOUT_GENERAL,
		&clear_value,
		1,
		&clear_range
	);
}

void VulkanRenderer::clear_image(ResourceHandle handle) {
	auto image = resource_manager.get_image(handle);
	auto draw_resource = resource_manager.get_image(draw_image);

	command_pool.transition_image(
		image->get_image(),
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_GENERAL
	);

	command_pool.transition_image(
		image->get_image(),
		VK_IMAGE_LAYOUT_GENERAL,
		VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
	);

	command_pool.transition_image(
		draw_resource->get_image(),
		VK_IMAGE_LAYOUT_GENERAL,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
	);

	command_pool.copy_image(
		image->get_image(),
		draw_resource->get_image(),
		VkExtent2D{ .width = image->get_extent().width, .height = image->get_extent().height },
		VkExtent2D{ .width = app_context->width, .height = app_context->height }
	);

	command_pool.transition_image(
		draw_resource->get_image(),
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_LAYOUT_GENERAL
	);
}

ResourceHandle VulkanRenderer::create_image() {
	return resource_manager.create_image();
}

ResourceHandle VulkanRenderer::create_buffer() {
	return resource_manager.create_buffer();
}

void VulkanRenderer::buffer_data(ResourceHandle handle, BufferType type, size_t size, void *data) {
	resource_manager.buffer_data(handle, convert::convert_buffer_type(type), data, size);
}

void VulkanRenderer::buffer_sub_data(ResourceHandle handle, size_t offset, size_t size, void *data) {
	resource_manager.buffer_sub_data(handle, offset, data, size);
}

void VulkanRenderer::image_data(
	ResourceHandle handle,
	ImageDimensions dimensions,
	ImageSamples samples,
	ImageFormat format,
	void *data,
	bool mipmapped,
	int width, int height, int depth = 1
) {
	auto image_info = info::image_create_info(width, height, depth);
	image_info.imageType = convert::convert_image_dimensions(dimensions);
	image_info.samples = convert::convert_sample_bits(samples);
	image_info.format = convert::convert_image_format(format);
	image_info.tiling = VK_IMAGE_TILING_OPTIMAL;

	image_info.usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	image_info.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	image_info.usage |= VK_IMAGE_USAGE_STORAGE_BIT;
	image_info.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	image_info.usage |= VK_IMAGE_USAGE_SAMPLED_BIT;

	if(mipmapped)
		image_info.mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1;

	resource_manager.image_data(handle, image_info, data);
}

void VulkanRenderer::rebuild() {
	int width, height;
	SDL_GetWindowSize(app_context->window, &width, &height);
	app_context->width = width;
	app_context->height = height;

	device.wait();

	image_data(
		draw_image,
		ImageDimensions::IMAGE_2D,
		ImageSamples::SAMPLE_COUNT_1_BIT,
		ImageFormat::R16G16B16A16_SFLOAT,
		nullptr,
		false,
		app_context->width, app_context->height, 1
	);

	swapchain.rebuild();
	command_pool.rebuild();
}

static VulkanRenderer *singleton;

extern "C" EAPI AbstractRenderer *create_factory(void *user_data) {
	if(!singleton)
		singleton = new VulkanRenderer(reinterpret_cast<AppContext*>(user_data));
	return singleton;
}