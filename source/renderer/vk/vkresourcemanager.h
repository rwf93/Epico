#pragma once

#include "vkimage.h"
#include "vkbuffer.h"

class VulkanDevice;
class VulkanResource;
class VulkanCommandPool;

class VulkanResourceManager {
public:
	void init(FunctorQueue<> &queue, VulkanInstance *vkinstance, VulkanDevice *vkdevice, VulkanCommandPool *vkcommandpool);
	void fini();

	ResourceHandle create_image(VkImageCreateInfo image_info);
	ResourceHandle create_buffer(void *data, VkDeviceSize size, VkBufferCreateFlags type);

	VulkanImage *get_image(ResourceHandle handle) {
		return get_resource<VulkanImage*, ResourceType::IMAGE>(handle);
	}

	template<typename T, ResourceType R>
	T get_resource(ResourceHandle handle) {
		AbstractResource *resource = resources[handle];
		if(!resource)
			return nullptr;

		if(resource->get_type() != R)
			return nullptr;

		return dynamic_cast<T>(resource);
	}

private:


	// increments the resource handle and returns the last one
	ResourceHandle advance_handle() {
		ResourceHandle last_resource_handle = current_resource_handle;
		current_resource_handle++;
		return last_resource_handle;
	}
private:
	VulkanInstance *instance;
	VulkanDevice *device;
	VulkanCommandPool *command_pool;

	VmaAllocator allocator;

	std::map<ResourceHandle, AbstractResource*> resources;
	ResourceHandle current_resource_handle = 0;
};