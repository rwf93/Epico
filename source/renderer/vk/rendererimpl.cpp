#include "vkinstance.h"
#include "vksurface.h"
#include "vkdevice.h"
#include "vkswapchain.h"
#include "vkcommandpool.h"
#include "vkresourcemanager.h"
#include "vkimgui.h"

#include "rendererimpl.h"

#include <public/filesystem/abstractvfs.h>

VulkanRenderer::VulkanRenderer(AppContext *app_context) {
	this->app_context = app_context;

	VK_CHECK(volkInitialize());

	instance.init(cleanup_queue);
	surface.init(cleanup_queue, app_context, &instance);
	device.init(cleanup_queue, &instance, &surface);
	swapchain.init(cleanup_queue, &device);
	command_pool.init(cleanup_queue, &device, &swapchain);
	resource_manager.init(cleanup_queue, &instance, &device, &command_pool);
}

VulkanRenderer::~VulkanRenderer() {
	cleanup_queue.destroy();
}

void VulkanRenderer::begin() {
	command_pool.wait_fences();
	command_pool.reset_fences();

	if(swapchain.aquire_next_image(&command_pool))
		rebuild();

	command_pool.begin_recording();

	swapchain.transition_image(
		command_pool.get_command(),
		swapchain.get_swapchain_image(),
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_GENERAL
	);
}

void VulkanRenderer::end() {
	swapchain.transition_image(
		command_pool.get_command(),
		swapchain.get_swapchain_image(),
		VK_IMAGE_LAYOUT_GENERAL,
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
		swapchain.get_swapchain_image(),
		VK_IMAGE_LAYOUT_GENERAL,
		&clear_value,
		1,
		&clear_range
	);
}

ResourceHandle VulkanRenderer::create_image(
	int width,
	int height,
	int depth,
	ImageDimensions dimensions,
	ImageFormat format,
	ImageSample samples
) {
	UNUSED(dimensions)

	auto image_info = info::image_create_info(width, height, depth);
	image_info.format = convert::convert_image_format(format);
	image_info.samples = convert::convert_sample_bits(samples);
	image_info.mipLevels = 1;
	image_info.arrayLayers = 1;
	image_info.tiling = VK_IMAGE_TILING_OPTIMAL;

	return resource_manager.create_image(image_info);
}

ResourceHandle VulkanRenderer::create_buffer(
	void *data,
	size_t size,
	BufferType type
) {
	return resource_manager.create_buffer(data, size, convert::convert_buffer_type(type));
}

void VulkanRenderer::rebuild() {
	device.wait();
	swapchain.rebuild();
	command_pool.rebuild();
}

static VulkanRenderer *singleton;

extern "C" EAPI AbstractRenderer *create_factory(void *user_data) {
	if(!singleton)
		singleton = new VulkanRenderer(reinterpret_cast<AppContext*>(user_data));
	return singleton;
}