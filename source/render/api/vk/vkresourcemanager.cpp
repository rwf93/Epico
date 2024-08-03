#include "vkinstance.h"
#include "vkdevice.h"
#include "vkcommandpool.h"
#include "vkresourcemanager.h"

void VulkanResourceManager::init(
	FunctorQueue<> &queue,
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

	queue.push([&] { fini(); });
}

#define DELETE_RESOURCE(res) 								\
	for(auto &resource: res) { 								\
		if(auto second = resource.second) { 				\
			if(second->get_state() == ResourceState::READY) \
				second->fini(); 							\
			delete second; 									\
		} 													\
	} 														\

void VulkanResourceManager::fini() {
	DELETE_RESOURCE(texture_resources);
	DELETE_RESOURCE(texture_view_resources);
	DELETE_RESOURCE(buffer_resources);

	vmaDestroyAllocator(allocator);
}

TextureHandle VulkanResourceManager::create_texture() {
	TextureHandle last_resource_handle = static_cast<TextureHandle>(texture_resources.size() + 1);
	texture_resources.insert(std::make_pair(last_resource_handle, new VulkanTexture()));
	return last_resource_handle;
}

TextureViewHandle VulkanResourceManager::create_texture_view() {
	TextureViewHandle last_resource_handle = static_cast<TextureViewHandle>(texture_view_resources.size() + 1);
	texture_view_resources.insert(std::make_pair(last_resource_handle, new VulkanTextureView()));
	return last_resource_handle;
}

BufferHandle VulkanResourceManager::create_buffer() {
	BufferHandle last_resource_handle = static_cast<BufferHandle>(buffer_resources.size() + 1);
	buffer_resources.insert(std::make_pair(last_resource_handle, new VulkanBuffer()));
	return last_resource_handle;
}

void VulkanResourceManager::texture_data(
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

void VulkanResourceManager::texture_sub_data(TextureHandle handle, void *data, VkDeviceSize size, VkDeviceSize offset) {
	UNUSED(handle);
	UNUSED(data);
	UNUSED(size);
	UNUSED(offset);
}

void VulkanResourceManager::texture_view(
	TextureViewHandle view_handle,
	TextureHandle image_handle,
	VkImageViewCreateInfo image_view_info
) {
	auto image_view = try_get_texture_view(view_handle).value();
	auto image = try_get_texture(image_handle).value();

	assert(image_view);
	assert(image);

	if(image_view->get_state() != ResourceState::UNREADY)
		image_view->fini();

	image_view_info.image = image->get_image();
	image_view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	if(image->get_info()->usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
		image_view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

	image_view->init(device, image, &image_view_info);
}

void VulkanResourceManager::buffer_data(BufferHandle handle, VkBufferUsageFlagBits type, void *data, VkDeviceSize size) {
	auto resource = try_get_buffer(handle).value();
	assert(resource);

	if(!resource) {
		LOGGER->error("Invalid resource or resource is the wrong type.");
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

void VulkanResourceManager::buffer_sub_data(BufferHandle handle, VkDeviceSize offset, void *data, VkDeviceSize size) {
	auto resource = try_get_buffer(handle).value();
	assert(resource);

	if(!resource) {
		LOGGER->error("Invalid resource or resource is the wrong type.");
		return;
	}

	resource->update(offset, size, data);
}