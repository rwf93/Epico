#pragma once

#include "vktexture.h"
#include "vktextureview.h"
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
	BufferHandle create_buffer();

	void texture_data(
		TextureHandle handle,
		VkImageCreateInfo image_info,
		void *data
	);

	void texture_sub_data(TextureHandle handle, void *data, VkDeviceSize size, VkDeviceSize offset);

	void texture_view(
		TextureViewHandle view_handle,
		TextureHandle image_handle,
		VkImageViewCreateInfo image_view_info
	);

	void buffer_data(BufferHandle handle, VkBufferCreateFlags type, void *data, VkDeviceSize size);
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

	std::optional<VulkanTexture*> try_get_texture(TextureViewHandle handle) {
		if(!texture_view_cross.contains(handle))
			return std::nullopt;

		auto resource = texture_view_cross.at(handle);
		if(!resource)
			return std::nullopt;

		return dynamic_cast<VulkanTexture*>(resource);
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

	std::map<TextureHandle, AbstractResource*> texture_resources;

	std::map<TextureViewHandle, AbstractResource*> texture_view_resources;
	std::map<TextureViewHandle, AbstractResource*> texture_view_cross; // Crossreference into texture_resources.

	std::map<BufferHandle, AbstractResource*> buffer_resources;

};