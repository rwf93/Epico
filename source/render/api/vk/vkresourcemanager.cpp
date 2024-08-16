#include "vkinstance.h"
#include "vkdevice.h"
#include "vkcommandpool.h"
#include "vkresourcemanager.h"

#define DELETE_RESOURCE(res) 								\
	for(auto &resource: res) { 								\
		if(resource->get_state() == ResourceState::READY) \
			resource->fini(); 							\
		delete resource; 									\
	}

VulkanResourceManager::~VulkanResourceManager() {
	//DELETE_RESOURCE(texture_resources);
	//DELETE_RESOURCE(texture_view_resources);
	//DELETE_RESOURCE(sampler_resources);
	//DELETE_RESOURCE(buffer_resources);
	//DELETE_RESOURCE(layout_resources);
	//DELETE_RESOURCE(graphics_program_resources)

	texture_pool.release_all();
	texture_view_pool.release_all();
	sampler_pool.release_all();
	buffer_pool.release_all();
	layout_pool.release_all();
	graphics_program_pool.release_all();

	vmaDestroyAllocator(allocator);
}

void VulkanResourceManager::init(
	VulkanInstance *vkinstance,
	VulkanDevice *vkdevice,
	VulkanCommandPool *vkcommandpool
) {
	this->instance = vkinstance;
	this->device = vkdevice;
	this->command_pool = vkcommandpool;

	// This is due to Volk loading Vulkan functions dynamically.
	VmaVulkanFunctions vma_functions = {};
	vma_functions.vkGetInstanceProcAddr					= vkGetInstanceProcAddr;
	vma_functions.vkGetDeviceProcAddr 					= vkGetDeviceProcAddr ;
	vma_functions.vkAllocateMemory                    	= vkAllocateMemory;
	vma_functions.vkBindBufferMemory                  	= vkBindBufferMemory;
	vma_functions.vkBindImageMemory                   	= vkBindImageMemory;
	vma_functions.vkCreateBuffer                      	= vkCreateBuffer;
	vma_functions.vkCreateImage                       	= vkCreateImage;
	vma_functions.vkDestroyBuffer                     	= vkDestroyBuffer;
	vma_functions.vkDestroyImage                      	= vkDestroyImage;
	vma_functions.vkFlushMappedMemoryRanges           	= vkFlushMappedMemoryRanges;
	vma_functions.vkFreeMemory                        	= vkFreeMemory;
	vma_functions.vkGetBufferMemoryRequirements       	= vkGetBufferMemoryRequirements;
	vma_functions.vkGetImageMemoryRequirements        	= vkGetImageMemoryRequirements;
	vma_functions.vkGetPhysicalDeviceMemoryProperties 	= vkGetPhysicalDeviceMemoryProperties;
	vma_functions.vkGetPhysicalDeviceProperties       	= vkGetPhysicalDeviceProperties;
	vma_functions.vkInvalidateMappedMemoryRanges      	= vkInvalidateMappedMemoryRanges;
	vma_functions.vkMapMemory                         	= vkMapMemory;
	vma_functions.vkUnmapMemory                       	= vkUnmapMemory;
	vma_functions.vkCmdCopyBuffer                     	= vkCmdCopyBuffer;

	VmaAllocatorCreateInfo allocator_info = {};
	allocator_info.instance = instance->get_instance();
	allocator_info.device = device->get_device();
	allocator_info.physicalDevice = device->get_device().physical_device;
	allocator_info.pVulkanFunctions = &vma_functions;
	allocator_info.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;

	VK_CHECK(vmaCreateAllocator(&allocator_info, &allocator));

	layout_builder.init(device, this);
	graphics_program_builder.init(device, this);
}

TextureHandle VulkanResourceManager::create_texture() {
	return texture_pool.acquire().value();
}

TextureViewHandle VulkanResourceManager::create_texture_view() {
	return texture_view_pool.acquire().value();
}

SamplerHandle VulkanResourceManager::create_sampler() {
	return sampler_pool.acquire().value();
}

BufferHandle VulkanResourceManager::create_buffer() {
	return buffer_pool.acquire().value();
}

RenderLayoutBuilder *VulkanResourceManager::create_layout() {
	auto handle = layout_pool.acquire().value();
	layout_builder.clear(handle);
	return &layout_builder;
}

GraphicsProgramBuilder *VulkanResourceManager::create_graphics_program() {
	auto handle = graphics_program_pool.acquire().value();
	graphics_program_builder.clear(handle);
	return &graphics_program_builder;
}

void VulkanResourceManager::texture(
	TextureHandle handle,
	VkImageCreateInfo image_info,
	void *data
) {
	auto resource = try_get_texture(handle).value();
	assert(resource);

	if(resource->get_state() == ResourceState::READY)
		resource->fini();

	auto allocate_info = info::allocation_create_info(0);
	allocate_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	allocate_info.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

	resource->init(
		device,
		command_pool,
		allocator,
		&image_info,
		&allocate_info
	);

	// ditto.
	if(!data)
		return;

	size_t size = image_info.extent.depth * image_info.extent.width * image_info.extent.height * 4;

	auto staging_allocate_info = info::allocation_create_info();
	auto staging_buffer_info = info::buffer_create_info(size);

	VulkanBuffer staging_buffer;

	staging_buffer.init(
		device,
		command_pool,
		allocator,
		&staging_buffer_info,
		&staging_allocate_info
	);

	memcpy(staging_buffer.get_allocation_info().pMappedData, data, size);
	resource->stage(&staging_buffer, image_info.extent);

	staging_buffer.fini();
}

void VulkanResourceManager::texture_view(
	TextureViewHandle view_handle,
	TextureHandle image_handle,
	VkImageViewCreateInfo image_view_info
) {
	auto image_view = try_get_texture_view(view_handle).value();
	auto image = try_get_texture(image_handle).value();

	if(!image_view)
		throw std::runtime_error("Image view handle was invalid at creation time.");

	if(!image)
		throw std::runtime_error("Image handle was invalid at creation time.");


	if(image_view->get_state() != ResourceState::UNREADY)
		image_view->fini();

	image_view_info.image = image->get_image();
	image_view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	if(image->get_info()->usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
		image_view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

	image_view->init(device, &image_view_info);
}

void VulkanResourceManager::sampler(
	SamplerHandle handle,
	VkSamplerCreateInfo sampler_create_info
) {
	auto sampler = try_get_sampler_resource(handle).value();
	assert(sampler);

	if(sampler->get_state() != ResourceState::UNREADY)
		sampler->fini();

	sampler->init(device, &sampler_create_info);
}

void VulkanResourceManager::buffer(BufferHandle handle, VkBufferUsageFlagBits type, void *data, VkDeviceSize size) {
	auto resource = try_get_buffer(handle).value();
	assert(resource);

	if(!resource) {
		LOGGER->error("Invalid resource or resource is the wrong type.");
		std::abort();
		return;
	}

	if(resource->get_state() == ResourceState::READY)
		resource->fini();

	auto staging_buffer_info = info::buffer_create_info(size);

	auto buffer_info = info::buffer_create_info(
		size,
		(type == VK_BUFFER_USAGE_VERTEX_BUFFER_BIT || type == VK_BUFFER_USAGE_INDEX_BUFFER_BIT) ? type | VK_BUFFER_USAGE_TRANSFER_DST_BIT : type
	);
	auto allocate_info = info::allocation_create_info();

	resource->init(
		device,
		command_pool,
		allocator,
		&buffer_info,
		&allocate_info
	);

	if(!data)
		return;

	if(buffer_info.usage & VK_BUFFER_USAGE_TRANSFER_DST_BIT) {
		VulkanBuffer staging_buffer;
		staging_buffer.init(
			device,
			command_pool,
			allocator,
			&staging_buffer_info,
			&allocate_info
		);

		memcpy(staging_buffer.get_allocation_info().pMappedData, data, size);

		resource->stage(&staging_buffer, size);

		staging_buffer.fini();
	} else {
		memcpy(resource->get_allocation_info().pMappedData, data, size);
	}
}

void VulkanResourceManager::buffer_sub(BufferHandle handle, VkDeviceSize offset, void *data, VkDeviceSize size) {
	auto resource = try_get_buffer(handle).value();
	assert(resource);

	if(!resource) {
		LOGGER->error("Invalid resource or resource is the wrong type.");
		return;
	}

	if(resource->create_info->usage & VK_BUFFER_USAGE_TRANSFER_DST_BIT) {
		auto allocate_info = info::allocation_create_info();
		auto staging_buffer_info = info::buffer_create_info(size);

		VulkanBuffer staging_buffer;
		staging_buffer.init(
			device,
			command_pool,
			allocator,
			&staging_buffer_info,
			&allocate_info
		);

		memcpy(staging_buffer.get_allocation_info().pMappedData, data, size);
		resource->stage(&staging_buffer, size, 0, offset);

		staging_buffer.fini();
	} else {
		memcpy(reinterpret_cast<char*>(resource->get_allocation_info().pMappedData) + offset, data, size);
	}
}