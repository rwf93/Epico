#pragma once

class VulkanDevice;
class VulkanCommandPool;
class VulkanBuffer: public RenderResource {
public:
	VulkanBuffer(
		VulkanDevice *vkdevice,
		VulkanCommandPool *vkcommandpool,
		VmaAllocator vkallocator
	)
		: device(vkdevice)
		, command_pool(vkcommandpool)
		, allocator(vkallocator) {}
	~VulkanBuffer() override {
		if(get_state() != ResourceState::UNREADY) fini();
	}

	VK_TRACY_MEMORY_OVERLOADS;

	VkBuffer &get_buffer() { return buffer; }
	VmaAllocation &get_allocation() { return allocation; }
	VmaAllocationInfo &get_allocation_info() { return allocation_info; }

 	void stage(
		VulkanBuffer *staging_buffer,
		VkDeviceSize size,
		VkDeviceSize src_offset = 0,
		VkDeviceSize dst_offset = 0
	);

	ResourceState get_state() override { return state; }
	VkBufferCreateInfo &get_info() { return create_info; }

	void init(
		VkBufferCreateInfo *buffer_create_info,
		VmaAllocationCreateInfo *allocation_create_info
	);
	void fini() override;

	friend class VulkanResourceManager;
private:
	VulkanDevice *device;
	VulkanCommandPool *command_pool;

	VmaAllocator allocator;

	VkBuffer buffer;

	VmaAllocation allocation;
	VmaAllocationInfo allocation_info = {};

	VkBufferCreateInfo create_info;

	ResourceState state = ResourceState::UNREADY;
};