#pragma once

#include "vktexture.h"
#include "vktextureview.h"
#include "vksampler.h"
#include "vkbuffer.h"
#include "vkshader.h"

#include "vklayoutbuilder.h"
#include "vklayout.h"

#include "vkgraphicsprogrambuilder.h"
#include "vkgraphicsprogram.h"

#define CHECK_RESOURCE(res) 								\
	res.value_or(nullptr);									\
	assert(res.has_value());								\
	if(!res.has_value()) { 									\
		LOGGER->error("\n{}\nCause: Invalid Render Handle is being passed to the API.", cpptrace::generate_trace().to_string()); \
		return; 											\
	}														\

#define CHECK_RESOURCE_RET(res, ret) 						\
	res.value_or(nullptr);									\
	assert(res.has_value());								\
	if(!res.has_value()) { 									\
		LOGGER->error("\n{}\nCause: Invalid Render Handle is being passed to the API.", cpptrace::generate_trace().to_string()); \
		return ret; 										\
	}														\


class VulkanDevice;
class VulkanResource;
class VulkanCommandPool;
class VulkanInstance;
class VulkanResourceManager {
public:
	VulkanResourceManager(VulkanInstance *vkinstance, VulkanDevice *vkdevice, VulkanCommandPool *vkcommandpool);
	~VulkanResourceManager();

	BufferHandle create_buffer();
	TextureHandle create_texture();
	TextureViewHandle create_texture_view();
	SamplerHandle create_sampler();
	ShaderHandle create_shader();

	RenderLayoutBuilder &create_layout();
	GraphicsProgramBuilder &create_graphics_program();

	void buffer(BufferHandle handle, VkBufferCreateInfo create_info, void *data);
	void buffer_sub(BufferHandle handle, VkDeviceSize offset, void *data, VkDeviceSize size);

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

	void shader(ShaderHandle handle, VkShaderModuleCreateInfo shader_info, VkPipelineShaderStageCreateInfo stage_info);

	std::optional<VulkanTexture*> try_get_resource(TextureHandle handle) {
		return texture_pool.resource(handle);
	}

	std::optional<VulkanTextureView*> try_get_resource(TextureViewHandle handle) {
		return texture_view_pool.resource(handle);
	}

	std::optional<VulkanSampler*> try_get_resource(SamplerHandle handle) {
		return sampler_pool.resource(handle);
	}

	std::optional<VulkanBuffer*> try_get_resource(BufferHandle handle) {
		return buffer_pool.resource(handle);
	}

	std::optional<VulkanLayout*> try_get_resource(LayoutHandle handle) {
		return layout_pool.resource(handle);
	}

	std::optional<VulkanShader*> try_get_resource(ShaderHandle handle) {
		return shader_pool.resource(handle);
	}

	std::optional<VulkanGraphicsProgram*> try_get_resource(GraphicsProgramHandle handle) {
		return graphics_program_pool.resource(handle);
	}

	using HandleVariant = std::variant<
		TextureHandle,
		TextureViewHandle,
		SamplerHandle,
		BufferHandle,
		LayoutHandle,
		ShaderHandle,
		GraphicsProgramHandle
	>;

	template<class... Ts>
	struct OverloadedVisitor : Ts... { using Ts::operator()...; };

	void release(HandleVariant handle) {
		std::visit(OverloadedVisitor{
			[&](TextureHandle handle) 			{ texture_pool.release(handle); },
			[&](TextureViewHandle handle) 		{ texture_view_pool.release(handle); },
			[&](SamplerHandle handle) 			{ sampler_pool.release(handle); },
			[&](BufferHandle handle) 			{ buffer_pool.release(handle); },
			[&](LayoutHandle handle) 			{ layout_pool.release(handle); },
			[&](ShaderHandle handle) 			{ shader_pool.release(handle); },
			[&](GraphicsProgramHandle handle) 	{ graphics_program_pool.release(handle); }
		}, handle);
	}

	void map(VulkanBuffer *buffer, void **mapped) {
		vmaMapMemory(allocator, buffer->get_allocation(), mapped);
	}

	void unmap(VulkanBuffer *buffer) {
		vmaUnmapMemory(allocator, buffer->get_allocation());
	}

 	void flush(VulkanBuffer *buffer, VkDeviceSize offset, VkDeviceSize size) {
		vmaFlushAllocation(allocator, buffer->allocation, offset, size);
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

		std::optional<HandleType> acquire(ResourceType *type) {
			if(head < PoolSize) {
				auto handle = free_handles.at(head++);
				resources.at(static_cast<size_t>(handle)) = type;
				return handle;
			}

			return std::nullopt;
		}

		void release(HandleType handle) {
			if(head <= 0) return;
			free_handles.at(head--) = handle;

			ResourceType **resource = &resources.at(static_cast<size_t>(handle));
			if(*resource == nullptr) return;
			delete *resource;
			*resource = nullptr;
		}

		void release_all() {
			for(auto resource: resources)
				if(resource != nullptr)
					delete resource;
			free_handles.clear();
		}

		std::optional<ResourceType*> resource(HandleType handle) {
			try {
				return resources.at(static_cast<size_t>(handle));
			} catch(...) {
				return std::nullopt;
			}
		}

		std::vector<HandleType> free_handles;
		std::vector<ResourceType*> resources;
		uint32_t head = 0;
	};

	friend class VulkanLayoutBuilder;
	friend class VulkanGraphicsProgramBuilder;

	std::vector<VulkanLayoutBuilder> layout_builders;
	std::vector<VulkanGraphicsProgramBuilder> graphics_program_builders;

	ResourcePool<TextureHandle, 		VulkanTexture, 1024> texture_pool;
	ResourcePool<TextureViewHandle, 	VulkanTextureView, 1024> texture_view_pool;
	ResourcePool<SamplerHandle, 		VulkanSampler, 1024> sampler_pool;
	ResourcePool<BufferHandle, 			VulkanBuffer, 4096> buffer_pool;
	ResourcePool<LayoutHandle, 			VulkanLayout, 64> layout_pool;
	ResourcePool<ShaderHandle, 			VulkanShader, 256> shader_pool;
	ResourcePool<GraphicsProgramHandle, VulkanGraphicsProgram, 128> graphics_program_pool;
};
