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

void VulkanResourceManager::fini() {
	for(auto &resource: resources) {
		if(auto second = resource.second) {
			if(second->get_state() == ResourceState::RESOURCE_READY)
				second->fini();
			delete second;
		}
	}

	vmaDestroyAllocator(allocator);
}

ResourceHandle VulkanResourceManager::create_image() {
	ResourceHandle last_resource_handle = advance_handle();
	resources.insert(std::make_pair(last_resource_handle,new VulkanImage()));
	return last_resource_handle;
}

ResourceHandle VulkanResourceManager::create_buffer() {
	ResourceHandle last_resource_handle = advance_handle();
	resources.insert(std::make_pair(last_resource_handle, new VulkanBuffer()));
	return last_resource_handle;
}

void VulkanResourceManager::buffer_data(ResourceHandle handle, VkBufferCreateFlags type, void *data, VkDeviceSize size) {
	auto resource = get_buffer(handle);
	assert(resource);

	if(!resource) {
		spdlog::error("Invalid resource or resource is the wrong type.");
		return;
	}

	if(resource->get_state() == ResourceState::RESOURCE_READY)
		resource->fini();

	auto staging_buffer_info = info::buffer_create_info(size);
	auto buffer_info = info::buffer_create_info(
		size,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | type
	);

	auto allocate_info = info::allocation_create_info();

	resource->init(
		device,
		command_pool,
		allocator,
		&buffer_info,
		&allocate_info
	);

	// Skip initalizing the buffer with data, just create the buffer object's metadata.
	if(!data)
		return;

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
}

void VulkanResourceManager::buffer_sub_data(ResourceHandle handle, VkDeviceSize offset, void *data, VkDeviceSize size) {
	auto resource = get_buffer(handle);
	assert(resource);

	if(!resource) {
		spdlog::error("Invalid resource or resource is the wrong type.");
		return;
	}

	resource->update(offset, size, data);
}

void VulkanResourceManager::image_data(
	ResourceHandle handle,
	VkImageCreateInfo image_info,
	void *data
) {
	auto resource = get_image(handle);
	assert(resource);

	if(!resource) {
		spdlog::error("Invalid resource or resource is the wrong type.");
		return;
	}

	if(resource->get_state() == ResourceState::RESOURCE_READY)
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

void VulkanResourceManager::image_sub_data(ResourceHandle handle, void *data, VkDeviceSize size, VkDeviceSize offset) {
	UNUSED(handle);
	UNUSED(data);
	UNUSED(size);
	UNUSED(offset);
}
