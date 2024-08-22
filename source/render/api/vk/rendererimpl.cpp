#include "vkinstance.h"
#include "vksurface.h"
#include "vkdevice.h"
#include "vkswapchain.h"
#include "vkcommandpool.h"
#include "vkresourcemanager.h"

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
	instance.init();
	surface.init(context, &instance);
	device.init(&instance, &surface);
	swapchain.init(&device);
	command_pool.init(&device, &swapchain);
	resource_manager.init(&instance, &device, &command_pool);
	ui_imgui.init(context, &instance, &device, &swapchain, &command_pool, &resource_manager);
}

VulkanAPI::~VulkanAPI() {
	device.wait();
}

void VulkanAPI::begin() {
	ZoneScoped;

	command_pool.wait_fences();
	command_pool.reset_fences();

	if(swapchain.aquire_next_image(&command_pool))
		rebuild();

	command_pool.begin_recording();

	TracyVkZone(command_pool.get_frame_context().trace_context, command_pool.get_command()->get_command(), "API Begin");

	command_pool.get_command()->transition_image(
		swapchain.get_swapchain_image(),
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_GENERAL
	);
}

void VulkanAPI::end() {
	command_pool.get_command()->transition_image(
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
	std::optional<VkRenderingAttachmentInfo> depth_attachment;

	for(auto &dependency: dependencies) {
		auto texture = resource_manager.try_get_texture(dependency.texture).value();
		auto texture_view = resource_manager.try_get_texture_view(dependency.view).value();

		VkClearValue *clear_value = dependency.clear.has_value()
			? reinterpret_cast<VkClearValue*>(&dependency.clear.value())
			: nullptr;

		switch(dependency.type) {
			case AttachmentType::COLOR:
				color_attachments.push_back(
					info::attachment_info(
						texture_view->get_view(),
						clear_value,
						VK_IMAGE_LAYOUT_GENERAL
					)
				);

				texture->transition(VK_IMAGE_LAYOUT_GENERAL);
				break;
			case AttachmentType::DEPTH:
				depth_attachment = (
					info::attachment_info(
						texture_view->get_view(),
						clear_value,
						VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL
					)
				);

				texture->transition(VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);
				break;
			case AttachmentType::SHADER:
				texture->transition(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
				break;
			default: break;
		}
	}

	auto rendering_info = info::rendering_info(
		swapchain.get_swapchain().extent,
		color_attachments.data(), depth_attachment.has_value() ? &depth_attachment.value() : nullptr,
		static_cast<uint32_t>(color_attachments.size())
	);

	command_pool.get_command()->begin_rendering(&rendering_info);
}

void VulkanAPI::end_pass(std::span<SubpassAttachment> dependencies) {
	command_pool.get_command()->end_rendering();

	for(auto &dependency: dependencies) {
		auto resource = resource_manager.try_get_texture(dependency.texture).value();
		UNUSED(resource);
		switch(dependency.type) {
			case AttachmentType::COLOR:
			case AttachmentType::DEPTH:
			case AttachmentType::SHADER:
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
	VkClearColorValue clear_value = { { r, g, b, a } };
	static std::vector<VkImageSubresourceRange> clear_ranges = {
		info::image_subresource_range(VK_IMAGE_ASPECT_COLOR_BIT)
	};

	command_pool.get_command()->clear_image(swapchain.get_swapchain_image(), VK_IMAGE_LAYOUT_GENERAL, &clear_value, clear_ranges);
}

void VulkanAPI::clear(TextureHandle handle, float r, float g, float b, float a) {
	auto image = resource_manager.try_get_texture(handle).value();

	VkClearColorValue clear_value = { { r, g, b, a } };
	static std::vector<VkImageSubresourceRange> clear_ranges = {
		info::image_subresource_range(VK_IMAGE_ASPECT_COLOR_BIT)
	};

	command_pool.get_command()->clear_image(image->get_image(), VK_IMAGE_LAYOUT_GENERAL, &clear_value, clear_ranges);
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

	command_pool.get_command()->transition_image(swapchain.get_swapchain_image(), VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	resource->transition(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

	command_pool.get_command()->copy_image(resource->get_image(), swapchain.get_swapchain_image(), resource->get_info()->extent, VkExtent3D{ swapchain.get_swapchain().extent.width, swapchain.get_swapchain().extent.height, 1 });

	resource->transition(VK_IMAGE_LAYOUT_GENERAL);
	command_pool.get_command()->transition_image(swapchain.get_swapchain_image(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL);
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
	auto shader = resource_manager.try_get_graphics_program(handle).value();
	command_pool.get_command()->bind_pipeline(VK_PIPELINE_BIND_POINT_GRAPHICS, shader->get_pipeline());
};

void VulkanAPI::bind_uniform(LayoutHandle layout_handle, std::span<UniformBind> binds) {
	// Cache write_sets and info_objects.
	static std::vector<VkWriteDescriptorSet> write_sets = {};
	if(write_sets.size() < binds.size())
		write_sets.resize(binds.size());

	static std::vector<std::pair<VkDescriptorBufferInfo, VkDescriptorImageInfo>> info_objects(binds.size());
	if(info_objects.size() < binds.size())
		info_objects.resize(binds.size());

	auto layout = resource_manager.try_get_layout(layout_handle).value();

	for(uint32_t i = 0; i < binds.size(); i++) {
		UniformBind &bind = binds[i];

		VkWriteDescriptorSet descriptor_write = {};
		descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptor_write.dstSet = 0;
		descriptor_write.dstBinding = i;
		descriptor_write.dstArrayElement = 0;
		descriptor_write.descriptorCount = 1;

		switch(bind.type) {
			case UniformType::BUFFER:
			case UniformType::STORAGE: {
				auto buffer_resource = resource_manager.try_get_buffer(bind.buffer.buffer_handle).value();

				VkDeviceSize alignment = (bind.type == UniformType::STORAGE)
					? device.get_device().physical_device.properties.limits.minStorageBufferOffsetAlignment
					: device.get_device().physical_device.properties.limits.minUniformBufferOffsetAlignment;

				VkDescriptorBufferInfo *buffer_info = &info_objects.at(i).first;
				buffer_info->buffer = buffer_resource->get_buffer();
				buffer_info->offset = VK_ALIGN_BOUNDS(
					swapchain.get_image_index() > 0
					? bind.buffer.offset + (bind.buffer.range * swapchain.get_image_index())
					: bind.buffer.offset,
				alignment);
				buffer_info->range = bind.buffer.range;

				descriptor_write.descriptorType =
					(bind.type == UniformType::STORAGE)
						? VK_DESCRIPTOR_TYPE_STORAGE_BUFFER
						: VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				descriptor_write.pBufferInfo = buffer_info;
			} break;
			case UniformType::TEXTURE: {
				auto texture_view_resource = resource_manager.try_get_texture_view(bind.texture.texture_view_handle).value();
				auto sampler_resource = resource_manager.try_get_sampler_resource(bind.texture.sampler_handle).value();

				VkDescriptorImageInfo *image_info = &info_objects.at(i).second;
				image_info->imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				image_info->imageView = texture_view_resource->get_view();
				image_info->sampler = sampler_resource->get_sampler();

				descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				descriptor_write.pImageInfo = image_info;
			} break;
			default: break;
		}

		write_sets.at(i) = descriptor_write;
	}


	vkCmdPushDescriptorSetKHR(
		command_pool.get_command()->get_command(),
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		layout->get_pipeline_layout(),
		0,
		static_cast<uint32_t>(binds.size()), write_sets.data()
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
	return resource_manager.create_layout();
}

GraphicsProgramBuilder *VulkanAPI::create_graphics_program() {
	return resource_manager.create_graphics_program();
}

void VulkanAPI::buffer(BufferHandle handle, BufferType type, size_t size, void *data) {
	// Buffers of type Uniform or Storage are size * the amount of max flying frames.
	if(type == BufferType::UNIFORM || type == BufferType::STORAGE) {
		resource_manager.buffer(handle, convert::convert_buffer_type(type), data, size * command_pool.get_max_flying_frames());
	} else {
		resource_manager.buffer(handle, convert::convert_buffer_type(type), data, size);
	}
}

void VulkanAPI::buffer_sub(BufferHandle handle, size_t offset, size_t size, void *data) {
	auto buffer = resource_manager.try_get_buffer(handle).value();
	if(buffer->get_info().usage & VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT || buffer->get_info().usage & VK_BUFFER_USAGE_STORAGE_BUFFER_BIT) {
		resource_manager.buffer_sub(
			handle,
			command_pool.get_current_frame() > 0
				? offset + (size * command_pool.get_current_frame())
				: offset,
			data,
			size
		);
	} else {
		resource_manager.buffer_sub(handle, offset, data, size);
	}
}

void VulkanAPI::texture(
	TextureHandle handle,
	ImageDimensions dimensions,
	ImageSamples samples,
	ImageFormat format,
	ImageFlags flags,
	void *data,
	int width, int height
) {
	auto image_info = info::image_create_info(width, height, 1);
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

	resource_manager.texture(handle, image_info, data);
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