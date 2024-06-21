#include "vkinstance.h"
#include "vkdevice.h"
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

ResourceHandle VulkanResourceManager::create_image(VkImageCreateInfo image_info) {
	ResourceHandle last_resource_handle = advance_handle();

	auto *image_resource = new VulkanImage(device, command_pool, allocator);
	resources[last_resource_handle] = image_resource;

	image_resource->init(image_info);

	return last_resource_handle;
}

void VulkanResourceManager::fini() {
	for(auto &resource: resources) {
		if(auto second = resource.second) {
			switch(second->get_type()) {
				case ResourceType::IMAGE: dynamic_cast<VulkanImage*>(second)->fini();
				case ResourceType::BUFFER:
				default: break;
			}

			delete second;
		}
	}

	vmaDestroyAllocator(allocator);
}
