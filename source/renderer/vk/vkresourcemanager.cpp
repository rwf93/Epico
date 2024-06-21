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

	VK_CHECK(vmaCreateAllocator(&allocator_info, &allocator));

	queue.push([&] { fini(); });
}

ResourceHandle VulkanResourceManager::create_image() {
	ResourceHandle last_resource_handle = advance_handle();

	auto *image_resource = new VulkanImage(device, command_pool, allocator);
	resources[last_resource_handle] = image_resource;

	return last_resource_handle;
}

ResourceHandle VulkanResourceManager::create_buffer() {
	ResourceHandle last_resource_handle = advance_handle();

	auto *buffer_resource = new VulkanBuffer(device, command_pool, allocator);
	resources[last_resource_handle] = buffer_resource;

	return last_resource_handle;
}

void VulkanResourceManager::buffer_data(ResourceHandle handle, void *data, VkDeviceSize size, VkBufferCreateFlags type) {
	auto resource = get_resource<VulkanBuffer*, ResourceType::BUFFER>(handle);

	// Deallocate the previous object if it was prepared.
	resource->fini();

	auto staging_buffer_info = info::buffer_create_info(size);
	auto buffer_info = info::buffer_create_info(
		size,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | type
	);

	auto allocate_info = info::allocation_create_info();

	resource->init(&buffer_info, &allocate_info);

	VulkanBuffer staging_buffer = { device, command_pool, allocator };
	staging_buffer.init(&staging_buffer_info, &allocate_info);

	memcpy(staging_buffer.get_allocation_info().pMappedData, data, size);
	resource->stage(&staging_buffer, size);

	staging_buffer.fini();
}

void VulkanResourceManager::buffer_sub_data(ResourceHandle handle, void *data, VkDeviceSize size, VkDeviceSize offset) {
	auto resource = get_resource<VulkanBuffer*, ResourceType::BUFFER>(handle);
	resource->update(data, size, offset);
}

void VulkanResourceManager::fini() {
	for(auto &resource: resources) {
		if(auto second = resource.second) {
			switch(second->get_type()) {
				case ResourceType::IMAGE: dynamic_cast<VulkanImage*>(second)->fini(); break;
				case ResourceType::BUFFER: dynamic_cast<VulkanBuffer*>(second)->fini(); break;
				default: break;
			}

			delete second;
		}
	}

	vmaDestroyAllocator(allocator);
}
