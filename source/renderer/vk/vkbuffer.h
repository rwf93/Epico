#pragma once

class VulkanDevice;
class VulkanCommandPool;
class VulkanBuffer: public AbstractResource {
public:
	VulkanBuffer(
		VulkanDevice *vkdevice,
		VulkanCommandPool *vkcommandpool,
		VmaAllocator vkallocator
	);
	~VulkanBuffer() override;

	VkBuffer &get_buffer() { return buffer; }
	VmaAllocation &get_allocation() { return allocation; }
	VmaAllocationInfo &get_allocation_info() { return allocation_info; }

	ResourceType get_type() override { return ResourceType::BUFFER; }

	void stage(
		VulkanBuffer *staging_buffer,
    	VkDeviceSize size,
    	VkDeviceSize src_offset = 0,
    	VkDeviceSize dst_offset = 0
	);

	void init(
    	VkBufferCreateInfo *buffer_create_info,
    	VmaAllocationCreateInfo *allocation_create_info
	);
	void fini();
private:
    VulkanDevice *device;
	VulkanCommandPool *command_pool;

	VmaAllocator allocator;

	VkBuffer buffer;

	VmaAllocation allocation;
	VmaAllocationInfo allocation_info = {};
};