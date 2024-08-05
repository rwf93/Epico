#include "vkinstance.h"
#include "vksurface.h"
#include "vkdevice.h"
#include "vkswapchain.h"
#include "vkcommandpool.h"
#include "vkresourcemanager.h"
#include "vklayoutmanager.h"
#include "vkprogrammanager.h"

#include "vkimgui.h"

#include "rendererimpl.h"

CREATE_FACTORY(VulkanAPI);

void VulkanAPI::init(AppContext *app_context) {
	this->context = app_context;

	auto tracy_log_sink = std::make_shared<spdlog::sinks::callback_sink_mt>([](const spdlog::details::log_msg &msg) {
		UNUSED(msg); // Disabling tracy causes issues.
		TracyMessage(msg.payload.data(), msg.payload.size());
	});

	auto console = spdlog::stdout_color_mt("api_vulkan");
	console->sinks().push_back(tracy_log_sink);

	VK_CHECK(volkInitialize());
	instance.init(cleanup_queue);
	surface.init(cleanup_queue, context, &instance);
	device.init(cleanup_queue, &instance, &surface);
	swapchain.init(cleanup_queue, &device);
	command_pool.init(cleanup_queue, &device, &swapchain);
	resource_manager.init(cleanup_queue, &instance, &device, &command_pool);
	layout_manager.init(cleanup_queue, &instance, &device, &command_pool);
	shader_manager.init(cleanup_queue, &device, &layout_manager);
	ui_imgui.init(cleanup_queue, context, &instance, &device, &swapchain, &command_pool);
}

VulkanAPI::~VulkanAPI() {
	device.wait();
	cleanup_queue.destroy_backward();
}

void VulkanAPI::begin() {
	ZoneScoped;

	command_pool.wait_fences();
	command_pool.reset_fences();

	if(swapchain.aquire_next_image(&command_pool))
		rebuild();

	command_pool.begin_recording();

	TracyVkZone(command_pool.get_frame_context().trace_context, command_pool.get_command()->get_command(), "API Begin");

	command_pool.transition_image(
		swapchain.get_swapchain_image(),
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_GENERAL
	);
}

void VulkanAPI::end() {
	command_pool.transition_image(
		swapchain.get_swapchain_image(),
		VK_IMAGE_LAYOUT_GENERAL,
		VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
	);

	TracyVkCollect(command_pool.get_frame_context().trace_context, command_pool.get_command()->get_command());

	command_pool.end_recording();

	auto command_info = info::command_buffer_submit_info(command_pool.get_command()->get_command());
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

void VulkanAPI::begin_pass(std::span<SubpassAttachment> dependencies) {
	ZoneScoped;

	std::vector<VkRenderingAttachmentInfo> color_attachments;
	std::vector<VkRenderingAttachmentInfo> depth_attachment;

	for(auto &dependency: dependencies) {
		auto texture = resource_manager.try_get_texture(dependency.texture).value();
		auto texture_view = resource_manager.try_get_texture_view(dependency.view).value();

		VkClearValue *clear_value = reinterpret_cast<VkClearValue*>(&dependency.clear);
		switch(dependency.type) {
			case AttachmentType::COLOR:
				color_attachments.push_back(
					info::attachment_info(
						texture_view->get_view(),
						clear_value,
						VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
					)
				);

				command_pool.transition_image(
					texture->get_image(),
					VK_IMAGE_LAYOUT_UNDEFINED,
					VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
				);
				break;
			case AttachmentType::DEPTH:
				depth_attachment.push_back(
					info::attachment_info(
						texture_view->get_view(),
						clear_value,
						VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL
					)
				);

				command_pool.transition_image(
					texture->get_image(),
					VK_IMAGE_LAYOUT_UNDEFINED,
					VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL
				);
				break;
			case AttachmentType::SHADER:
				command_pool.transition_image(
					texture->get_image(),
					VK_IMAGE_LAYOUT_GENERAL,
					VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
				);
				break;
			default: break;
		}
	}

	auto rendering_info = info::rendering_info(
		swapchain.get_swapchain().extent,
		color_attachments.data(), depth_attachment.data(),
		static_cast<uint32_t>(color_attachments.size())
	);

	command_pool.get_command()->begin_rendering(&rendering_info);
}

void VulkanAPI::end_pass(std::span<SubpassAttachment> dependencies) {
	command_pool.get_command()->end_rendering();

	for(auto &dependency: dependencies) {
		auto resource = resource_manager.try_get_texture(dependency.texture).value();
		switch(dependency.type) {
			case AttachmentType::COLOR:
				command_pool.transition_image(
					resource->get_image(),
					VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
					VK_IMAGE_LAYOUT_GENERAL
				);
				break;
			case AttachmentType::DEPTH:
				command_pool.transition_image(
					resource->get_image(),
					VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
					VK_IMAGE_LAYOUT_GENERAL
				);
				break;
			case AttachmentType::SHADER:
				command_pool.transition_image(
					resource->get_image(),
					VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
					VK_IMAGE_LAYOUT_GENERAL
				);
				break;
			default: break;
		}
	}
}

void VulkanAPI::present() {
	VkPresentInfoKHR present_info = {};
	present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present_info.pSwapchains = &swapchain.get_swapchain().swapchain;
	present_info.swapchainCount = 1;
	present_info.pWaitSemaphores = &command_pool.get_finished_semaphore();
	present_info.waitSemaphoreCount = 1;
	present_info.pImageIndices = &swapchain.get_image_index();

	VkResult present_result = vkQueuePresentKHR(device.get_present_queue(), &present_info);
	if(present_result == VK_ERROR_OUT_OF_DATE_KHR || present_result == VK_SUBOPTIMAL_KHR)
		rebuild();

	command_pool.advance();
	FrameMark;
}

void VulkanAPI::clear(float r, float g, float b, float a) {
	command_pool.clear_image(swapchain.get_swapchain_image(), r, g, b, a);
}

void VulkanAPI::clear(TextureHandle handle, float r, float g, float b, float a) {
	auto image = resource_manager.try_get_texture(handle).value();
	command_pool.clear_image(image->get_image(), r, g, b, a);
}

void VulkanAPI::viewport(float width, float height, float x, float y) {
	std::vector<VkViewport> viewports = {
		info::viewport(width, height, x, y)
	};

	command_pool.get_command()->viewport(0, viewports);
}

void VulkanAPI::scissor(uint32_t width, uint32_t height, int32_t x, int32_t y) {
	VkRect2D scissor = {};
	scissor.extent.width = width;
	scissor.extent.height = height;
	scissor.offset.x = x;
	scissor.offset.y = y;

	std::vector<VkRect2D> scissors = {
		scissor
	};
	command_pool.get_command()->scissor(0, scissors);
}

void VulkanAPI::show_image(TextureHandle handle) {
	auto resource = resource_manager.try_get_texture(handle).value();

	command_pool.transition_image(swapchain.get_swapchain_image(), VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	command_pool.transition_image(resource->get_image(), VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

	command_pool.copy_image(resource->get_image(), swapchain.get_swapchain_image(), resource->get_info()->extent, VkExtent3D{ swapchain.get_swapchain().extent.width, swapchain.get_swapchain().extent.height, 1 });

	command_pool.transition_image(resource->get_image(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL);
	command_pool.transition_image(swapchain.get_swapchain_image(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL);
}

void VulkanAPI::bind_buffer(BufferHandle handle, BindBufferType type) {
	auto resource = resource_manager.try_get_buffer(handle).value();
	VkDeviceSize offset[] = { 0 };
	switch(type) {
		case BindBufferType::VERTEX:
			command_pool.get_command()->bind_vertex_buffer(0, 1, &resource->get_buffer(), offset);
			break;
		case BindBufferType::INSTANCE:
			command_pool.get_command()->bind_index_buffer(resource->get_buffer(), 0, VK_INDEX_TYPE_UINT32);
			break;
		default: break;
	}
}

void VulkanAPI::bind_shader(GraphicsProgramHandle handle) {
	auto shader = shader_manager.try_get_graphics_program(handle).value();
	assert(shader);
	command_pool.get_command()->bind_pipeline(VK_PIPELINE_BIND_POINT_GRAPHICS, shader->get_pipeline());
};

// MSVC is for some optimizing this function oddly casuing weird behaviour with vk.
void VulkanAPI::bind_uniform(LayoutHandle layout_handle, std::span<UniformBind> binds) {
	auto layout = layout_manager.try_get_layout(layout_handle).value();
	assert(layout);
	std::vector<VkWriteDescriptorSet> write_sets = {};

	for(uint32_t i = 0; i < binds.size(); i++) {
		UniformBind &bind = binds[i];

		switch(bind.type) {
			case UniformType::BUFFER: {
				auto buffer_resource = resource_manager.try_get_buffer(bind.buffer.buffer_handle).value();

				VkDescriptorBufferInfo buffer_info = {};
				buffer_info.buffer = buffer_resource->get_buffer();
				buffer_info.offset = bind.buffer.offset;
				buffer_info.range = bind.buffer.range;

				VkWriteDescriptorSet descriptor_write = {};
				descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptor_write.dstSet = 0;
				descriptor_write.dstBinding = i;
				descriptor_write.dstArrayElement = 0;
				descriptor_write.descriptorCount = 1;
				descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				descriptor_write.pBufferInfo = &buffer_info;

				write_sets.push_back(descriptor_write);
				break;
			}
			case UniformType::STORAGE: {
				auto buffer_resource = resource_manager.try_get_buffer(bind.buffer.buffer_handle).value();

				VkDescriptorBufferInfo buffer_info = {};
				buffer_info.buffer = buffer_resource->get_buffer();
				buffer_info.offset = bind.buffer.offset;
				buffer_info.range = bind.buffer.range;

				VkWriteDescriptorSet descriptor_write = {};
				descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptor_write.dstSet = 0;
				descriptor_write.dstBinding = i;
				descriptor_write.dstArrayElement = 0;
				descriptor_write.descriptorCount = 1;
				descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
				descriptor_write.pBufferInfo = &buffer_info;

				write_sets.push_back(descriptor_write);
				break;
			}
			case UniformType::TEXTURE: {
				auto texture_view_resource = resource_manager.try_get_texture_view(bind.texture.texture_view_handle).value();
				auto sampler_resource = resource_manager.try_get_sampler_resource(bind.texture.sampler_handle).value();

				VkDescriptorImageInfo image_info = {};
				image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				image_info.imageView = texture_view_resource->get_view();
				image_info.sampler = sampler_resource->get_sampler();

				VkWriteDescriptorSet descriptor_write = {};
				descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptor_write.dstSet = 0;
				descriptor_write.dstBinding = i;
				descriptor_write.dstArrayElement = 0;
				descriptor_write.descriptorCount = 1;
				descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				descriptor_write.pImageInfo = &image_info;

				write_sets.push_back(descriptor_write);
				break;
			}
			default: break;
		}
	}

	vkCmdPushDescriptorSetKHR(
		command_pool.get_command()->get_command(),
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		layout->get_pipeline_layout(),
		0,
		static_cast<uint32_t>(write_sets.size()), write_sets.data()
	);
}

void VulkanAPI::draw(uint32_t vertex_count, uint32_t instance_count) {
	command_pool.get_command()->draw(vertex_count, instance_count, 0, 0);
};

void VulkanAPI::draw_instanced(uint32_t index_count, uint32_t instance_count, uint32_t index) {
	command_pool.get_command()->draw_instanced(index_count, instance_count, 0, 0, index);
}

TextureHandle VulkanAPI::create_texture() {
	return resource_manager.create_texture();
}

SamplerHandle VulkanAPI::create_sampler() {
	return resource_manager.create_sampler();
}

TextureViewHandle VulkanAPI::create_texture_view() {
	return resource_manager.create_texture_view();
}

BufferHandle VulkanAPI::create_buffer() {
	return resource_manager.create_buffer();
}

RenderLayoutBuilder *VulkanAPI::create_layout() {
	return layout_manager.create_layout();
}

RenderGraphicProgramBuilder *VulkanAPI::create_graphic_program() {
	return shader_manager.create_graphic_program();
}

void VulkanAPI::buffer_data(BufferHandle handle, BufferType type, size_t size, void *data) {
	resource_manager.buffer_data(handle, convert::convert_buffer_type(type), data, size);
}

void VulkanAPI::buffer_sub_data(BufferHandle handle, size_t offset, size_t size, void *data) {
	resource_manager.buffer_sub_data(handle, offset, data, size);
}

void VulkanAPI::texture_data(
	TextureHandle handle,
	ImageDimensions dimensions,
	ImageSamples samples,
	ImageFormat format,
	ImageFlags flags,
	void *data,
	int width, int height, int depth = 1
) {
	auto image_info = info::image_create_info(width, height, depth);
	image_info.imageType = convert::convert_image_dimensions(dimensions);
	image_info.samples = convert::convert_sample_bits(samples);
	image_info.format = convert::convert_image_format(format);
	image_info.tiling = VK_IMAGE_TILING_OPTIMAL;

	image_info.usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	image_info.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;

	if(flags & ImageFlags::COLOR_ATTACHMENT)
		image_info.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	if(flags & ImageFlags::DEPTH_ATTACHMENT)
		image_info.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

	if(flags & ImageFlags::SAMPLED)
		image_info.usage |= VK_IMAGE_USAGE_SAMPLED_BIT;

	if(flags & ImageFlags::MIPMAPPED)
		image_info.mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1;

	resource_manager.texture_data(handle, image_info, data);
}

void VulkanAPI::texture_view(
	TextureViewHandle view_handle,
	TextureHandle image_handle,
	ImageViewDimensions dimensions,
	ImageFormat format,
	int min_level,
	int min_layer,
	int num_levels,
	int num_layers
) {
	VkImageViewCreateInfo view_info = {};
	view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	view_info.viewType = convert::convert_image_view_dimensions(dimensions);
	view_info.format = convert::convert_image_format(format);
	view_info.subresourceRange.baseMipLevel = min_level;
	view_info.subresourceRange.baseArrayLayer = min_layer;
	view_info.subresourceRange.levelCount = num_levels;
	view_info.subresourceRange.layerCount = num_layers;

	resource_manager.texture_view(view_handle, image_handle, view_info);
}

void VulkanAPI::sampler(
		SamplerHandle handle,
		SamplerAddressMode u,
		SamplerAddressMode v,
		SamplerAddressMode w
) {
	VkSamplerCreateInfo sampler_info = {};
	sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	sampler_info.magFilter = VK_FILTER_NEAREST;
	sampler_info.minFilter = VK_FILTER_NEAREST;

	sampler_info.addressModeU = convert::convert_address_mode(u);
	sampler_info.addressModeV = convert::convert_address_mode(v);
	sampler_info.addressModeW = convert::convert_address_mode(w);

	resource_manager.sampler(handle, sampler_info);
}

void VulkanAPI::rebuild() {
	int width, height;
	SDL_GetWindowSize(context->window, &width, &height);
	context->width = width;
	context->height = height;

	device.wait();
	swapchain.rebuild();

	if(resize_event)
		resize_event(this);
}