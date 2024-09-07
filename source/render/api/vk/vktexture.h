#pragma once

class VulkanDevice;
class VulkanCommandPool;
class VulkanBuffer;
class VulkanCommand;
class VulkanTexture: public RenderResource {
public:
	VulkanTexture(
		VulkanDevice *vkdevice,
		VulkanCommandPool *vkcommandpool,
		VmaAllocator vkallocator
	)
		: device(vkdevice)
		, command_pool(vkcommandpool)
		, allocator(vkallocator) {}

	~VulkanTexture() override { if(get_state() != ResourceState::UNREADY) fini(); }

	void init(
		VkImageCreateInfo *image_info,
		VmaAllocationCreateInfo *create_info
	);
	void fini() override;

	void stage(VulkanBuffer *staging_buffer, VkExtent3D image_extent);

	ResourceState get_state() override { return state; }

	VkImage &get_image() { return image; }

	VkImageCreateInfo *get_info() { return &info; }

	void transition(VulkanCommand *command, VkImageLayout new_layout);
	void transition(VkImageLayout new_layout);

	friend class VulkanResourceManager;
private:
	VulkanDevice *device;
	VulkanCommandPool *command_pool;
	VkImageCreateInfo info;

	VkImage image;

	VmaAllocator allocator;
	VmaAllocation allocation;

	ResourceState state = ResourceState::UNREADY;
	VkImageLayout last_layout;

	VK_TRACY_MEMORY_OVERLOADS;
};