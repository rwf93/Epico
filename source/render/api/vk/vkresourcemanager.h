#pragma once

#include "vktexture.h"
#include "vktextureview.h"
#include "vksampler.h"
#include "vkbuffer.h"

#include "vklayoutbuilder.h"
#include "vklayout.h"

#include "vkgraphicsprogrambuilder.h"
#include "vkgraphicsprogram.h"

class VulkanDevice;
class VulkanResource;
class VulkanCommandPool;
class VulkanResourceManager {
public:
	void init(FunctorQueue<> &queue, VulkanInstance *vkinstance, VulkanDevice *vkdevice, VulkanCommandPool *vkcommandpool);
	void fini();

	TextureHandle create_texture();
	TextureViewHandle create_texture_view();
	SamplerHandle create_sampler();
	BufferHandle create_buffer();

	RenderLayoutBuilder *create_layout();
	RenderGraphicProgramBuilder *create_graphics_program();

	void texture_data(
		TextureHandle handle,
		VkImageCreateInfo image_info,
		void *data
	);

	void texture_view(
		TextureViewHandle view_handle,
		TextureHandle image_handle,
		VkImageViewCreateInfo image_view_info
	);

	void sampler(SamplerHandle handle, VkSamplerCreateInfo sampler_create_info);

	void buffer_data(BufferHandle handle, VkBufferUsageFlagBits type, void *data, VkDeviceSize size);
	void buffer_sub_data(BufferHandle handle, VkDeviceSize offset, void *data, VkDeviceSize size);

	std::optional<VulkanTexture*> try_get_texture(TextureHandle handle) {
		auto resource = texture_resources[static_cast<size_t>(handle)];
		if(!resource)
			return std::nullopt;

		return dynamic_cast<VulkanTexture*>(resource);
	}

	std::optional<VulkanTextureView*> try_get_texture_view(TextureViewHandle handle) {
		auto resource = texture_view_resources[static_cast<size_t>(handle)];
		if(!resource)
			return std::nullopt;

		return dynamic_cast<VulkanTextureView*>(resource);
	}

	std::optional<VulkanSampler*> try_get_sampler_resource(SamplerHandle handle) {
		auto resource = sampler_resources[static_cast<size_t>(handle)];
		if(!resource)
			return std::nullopt;

		return dynamic_cast<VulkanSampler*>(resource);
	}

	std::optional<VulkanBuffer*> try_get_buffer(BufferHandle handle) {
		auto resource = buffer_resources[static_cast<size_t>(handle)];
		if(!resource)
			return std::nullopt;

		return dynamic_cast<VulkanBuffer*>(resource);
	}

	std::optional<VulkanLayout*> try_get_layout(LayoutHandle handle) {
		auto resource = layout_resources[static_cast<size_t>(handle)];
		if(!resource)
			return std::nullopt;

		return dynamic_cast<VulkanLayout*>(resource);
	}

	std::optional<VulkanGraphicsProgram*> try_get_graphics_program(GraphicsProgramHandle handle) {
		auto resource = graphics_program_resources[static_cast<size_t>(handle)];
		if(!resource)
			return std::nullopt;

		return dynamic_cast<VulkanGraphicsProgram*>(resource);
	}

private:
	VulkanInstance *instance;
	VulkanDevice *device;
	VulkanCommandPool *command_pool;

	VmaAllocator allocator;


	std::vector<RenderResource*> texture_resources;
	std::vector<RenderResource*> texture_view_resources;
	std::vector<RenderResource*> sampler_resources;
	std::vector<RenderResource*> buffer_resources;

	// Builders

	VulkanLayoutBuilder layout_builder;
	std::vector<RenderResource*> layout_resources;

	VulkanGraphicsProgramBuilder graphics_program_builder;
	std::vector<RenderResource*> graphics_program_resources;
};