#pragma once

#include "vktexture.h"
#include "vktextureview.h"
#include "vksampler.h"
#include "vkbuffer.h"

enum CreateBufferFlags {
	CREATE_BUFFER_COHERENT = 1 << 0,
};

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
		if(!texture_resources.contains(handle))
			return std::nullopt;

		auto resource = texture_resources.at(handle);
		if(!resource)
			return std::nullopt;

		return dynamic_cast<VulkanTexture*>(resource);
	}

	std::optional<VulkanTextureView*> try_get_texture_view(TextureViewHandle handle) {
		if(!texture_view_resources.contains(handle))
			return std::nullopt;

		auto resource = texture_view_resources.at(handle);
		if(!resource)
			return std::nullopt;

		return dynamic_cast<VulkanTextureView*>(resource);
	}

	std::optional<VulkanSampler*> try_get_sampler_resource(SamplerHandle handle) {
		if(!sampler_resources.contains(handle))
			return std::nullopt;

		auto resource = sampler_resources.at(handle);
		if(!resource)
			return std::nullopt;

		return dynamic_cast<VulkanSampler*>(resource);
	}

	std::optional<VulkanBuffer*> try_get_buffer(BufferHandle handle) {
		if(!buffer_resources.contains(handle))
			return std::nullopt;

		auto resource = buffer_resources.at(handle);
		if(!resource)
			return std::nullopt;

		return dynamic_cast<VulkanBuffer*>(resource);
	}

private:
	VulkanInstance *instance;
	VulkanDevice *device;
	VulkanCommandPool *command_pool;

	VmaAllocator allocator;

	std::map<TextureHandle, RenderResource*> texture_resources;
	std::map<TextureViewHandle, RenderResource*> texture_view_resources;
	std::map<SamplerHandle, RenderResource*> sampler_resources;
	std::map<BufferHandle, RenderResource*> buffer_resources;
};