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

	ResourceHandle create_image();
	ResourceHandle create_buffer();

	void buffer_data(ResourceHandle handle, VkBufferCreateFlags type, void *data, VkDeviceSize size);
	void buffer_sub_data(ResourceHandle handle, VkDeviceSize offset, void *data, VkDeviceSize size);

	void image_data(
		ResourceHandle handle,
		VkImageCreateInfo image_info,
		void *data
	);

	void image_sub_data(ResourceHandle handle, void *data, VkDeviceSize size, VkDeviceSize offset);

	VulkanImage *get_image(ResourceHandle handle) {
		return get_resource<VulkanImage*>(handle);
	}

	VulkanBuffer *get_buffer(ResourceHandle handle) {
		return get_resource<VulkanBuffer*>(handle);
	}

	template<typename T>
	T get_resource(ResourceHandle handle) {
		AbstractResource *resource = nullptr;
		if(!resources.contains(handle))
			goto err;

		resource = resources.at(handle);
		if(!resource)
			goto err;

		return dynamic_cast<T>(resource);
	err:
		return nullptr;
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