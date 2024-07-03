#include "vkinstance.h"
#include "vksurface.h"
#include "vkdevice.h"
#include "vkswapchain.h"
#include "vkcommandpool.h"
#include "vkresourcemanager.h"
#include "vkshadermanager.h"
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
	shader_manager.init(cleanup_queue, &device);
	ui_imgui.init(cleanup_queue, app_context, &instance, &device, &swapchain, &command_pool);
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
		swapchain.get_swapchain_image(),
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_GENERAL
	);
}

void VulkanRenderer::end() {
	command_pool.transition_image(
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
}

void VulkanRenderer::begin_pass(SubpassDependency *dependencies) {
	std::vector<VkRenderingAttachmentInfo> color_attachments;
	std::vector<VkRenderingAttachmentInfo> depth_attachments;

	for(uint32_t i = 0; i < dependencies->count; i++) {
		auto resource = resource_manager.get_image(dependencies->attachments[i]);
		switch(dependencies->types[i]) {
			case AttachmentType::COLOR:
				color_attachments.push_back(info::attachment_info(resource->get_view(), nullptr, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL));
				command_pool.transition_image(resource->get_image(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
				break;
			case AttachmentType::DEPTH:
				depth_attachments.push_back(info::attachment_info(resource->get_view(), nullptr, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL));
				command_pool.transition_image(resource->get_image(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);
				break;
			default: break;
		}
	}

	assert(depth_attachments.size() <= 1);

	auto rendering_info = info::rendering_info(
		swapchain.get_swapchain().extent,
		color_attachments.data(), depth_attachments.data(),
		static_cast<uint32_t>(color_attachments.size())
	);

	vkCmdBeginRendering(command_pool.get_command(), &rendering_info);
}

void VulkanRenderer::end_pass(SubpassDependency *dependencies) {
	vkCmdEndRendering(command_pool.get_command());

	for(uint32_t i = 0; i < dependencies->count; i++) {
		auto resource = resource_manager.get_image(dependencies->attachments[i]);
		switch(dependencies->types[i]) {
			case AttachmentType::COLOR:
				command_pool.transition_image(resource->get_image(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL);
				break;
			case AttachmentType::DEPTH:
				command_pool.transition_image(resource->get_image(), VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL);
				break;
			default: break;
		}
	}
}

void VulkanRenderer::present() {
	VkPresentInfoKHR present_info = {};
	present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present_info.pSwapchains = &swapchain.get_swapchain().swapchain;
	present_info.swapchainCount = 1;
	present_info.pWaitSemaphores = &command_pool.get_finished_semaphore();
	present_info.waitSemaphoreCount = 1;
	present_info.pImageIndices = &swapchain.get_image_index();

	VkResult present_result = vkQueuePresentKHR(device.get_graphics_queue(), &present_info);
	if(present_result == VK_ERROR_OUT_OF_DATE_KHR || present_result == VK_SUBOPTIMAL_KHR)
		rebuild();

	command_pool.advance();
}

void VulkanRenderer::clear(float r, float g, float b, float a) {
	command_pool.clear_image(swapchain.get_swapchain_image(), r, g, b, a);
}

void VulkanRenderer::viewport(float width, float height, float x, float y) {
	auto viewport = info::viewport(width, height, x, y);
	vkCmdSetViewport(command_pool.get_command(), 0, 1, &viewport);
}

void VulkanRenderer::scissor(uint32_t width, uint32_t height, int32_t x, int32_t y) {
	VkRect2D scissor = {};
	scissor.extent.width = width;
	scissor.extent.height = height;
	scissor.offset.x = x;
	scissor.offset.y = y;

	vkCmdSetScissor(command_pool.get_command(), 0, 1, &scissor);
}

ResourceHandle VulkanRenderer::create_image() {
	return resource_manager.create_image();
}

ResourceHandle VulkanRenderer::create_buffer() {
	return resource_manager.create_buffer();
}

AbstractGraphicShader *VulkanRenderer::create_graphic_shader() {
	return shader_manager.create_graphic_shader();
}

AbstractComputeShader *VulkanRenderer::create_compute_shader() {
	return shader_manager.create_compute_shader();
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
	if(resize_event) resize_event(this);
	swapchain.rebuild();
	command_pool.rebuild();
}

static VulkanRenderer *singleton;

extern "C" EAPI AbstractRenderer *create_factory(void *user_data) {
	if(!singleton)
		singleton = new VulkanRenderer(reinterpret_cast<AppContext*>(user_data));
	return singleton;
}