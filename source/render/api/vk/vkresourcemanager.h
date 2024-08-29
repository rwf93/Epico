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
class VulkanInstance;
class VulkanResourceManager {
public:
	VulkanResourceManager(VulkanInstance *vkinstance, VulkanDevice *vkdevice, VulkanCommandPool *vkcommandpool);
	~VulkanResourceManager();

	TextureHandle create_texture();
	TextureViewHandle create_texture_view();
	SamplerHandle create_sampler();
	BufferHandle create_buffer();

	RenderLayoutBuilder *create_layout();
	GraphicsProgramBuilder *create_graphics_program();

	void texture(
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

	void buffer(BufferHandle handle, VkBufferUsageFlagBits type, void *data, VkDeviceSize size);
	void buffer_sub(BufferHandle handle, VkDeviceSize offset, void *data, VkDeviceSize size);

	std::optional<VulkanTexture*> try_get_texture(TextureHandle handle) {
		return texture_pool.resource(handle);
	}

	std::optional<VulkanTextureView*> try_get_texture_view(TextureViewHandle handle) {
		return texture_view_pool.resource(handle).value();
	}

	std::optional<VulkanSampler*> try_get_sampler_resource(SamplerHandle handle) {
		return sampler_pool.resource(handle).value();
	}

	std::optional<VulkanBuffer*> try_get_buffer(BufferHandle handle) {
		return buffer_pool.resource(handle).value();
	}

	std::optional<VulkanLayout*> try_get_layout(LayoutHandle handle) {
		return layout_pool.resource(handle).value();
	}

	std::optional<VulkanGraphicsProgram*> try_get_graphics_program(GraphicsProgramHandle handle) {
		return graphics_program_pool.resource(handle).value();
	}

private:
	VulkanInstance *instance;
	VulkanDevice *device;
	VulkanCommandPool *command_pool;

	VmaAllocator allocator;

	template<typename HandleType, typename ResourceType, size_t PoolSize>
	struct ResourcePool {
		ResourcePool()
		: free_handles(PoolSize)
		, resources(PoolSize) {
			for(uint32_t i = 0; i < PoolSize; i++) {
				free_handles.at(i) = static_cast<HandleType>(i);
			}
		}

		std::optional<HandleType> acquire() {
			if(head < PoolSize) {
				auto handle = free_handles.at(head++);
				return handle;
			}

			return std::nullopt;
		}

		void release(HandleType handle) {
			free_handles.at(head--) = handle;
		}

		void release_all() {
			resources.clear();
			free_handles.clear();
		}

		std::optional<ResourceType*> resource(HandleType handle) {
			return &resources.at(static_cast<size_t>(handle));
		}

		std::vector<HandleType> free_handles;
		std::vector<ResourceType> resources;
		uint32_t head = 0;
	};

	VulkanLayoutBuilder layout_builder;
	VulkanGraphicsProgramBuilder graphics_program_builder;

	ResourcePool<TextureHandle, VulkanTexture, 1024> texture_pool;
	ResourcePool<TextureViewHandle, VulkanTextureView, 1024> texture_view_pool;
	ResourcePool<SamplerHandle, VulkanSampler, 1024> sampler_pool;
	ResourcePool<BufferHandle, VulkanBuffer, 512> buffer_pool;
	ResourcePool<LayoutHandle, VulkanLayout, 64> layout_pool;
	ResourcePool<GraphicsProgramHandle, VulkanGraphicsProgram, 128> graphics_program_pool;
};
