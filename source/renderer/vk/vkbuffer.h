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

	void stage(
		VulkanBuffer *staging_buffer,
		VkDeviceSize size,
		VkDeviceSize src_offset = 0,
		VkDeviceSize dst_offset = 0
	);

	void update(VkDeviceSize offset, VkDeviceSize size, void *data);

	ResourceState get_state() override { return state; }

	void init(
		VkBufferCreateInfo *buffer_create_info,
		VmaAllocationCreateInfo *allocation_create_info
	);
	void fini() override;
private:
	VulkanDevice *device;
	VulkanCommandPool *command_pool;

	VmaAllocator allocator;

	VkBuffer buffer;

	VmaAllocation allocation;
	VmaAllocationInfo allocation_info = {};

	ResourceState state = ResourceState::RESOURCE_UNREADY;
};