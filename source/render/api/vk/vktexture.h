#pragma once

class VulkanDevice;
class VulkanCommandPool;
class VulkanBuffer;
class VulkanTexture: public RenderResource {
public:
	~VulkanTexture() override;

	void init(
		VulkanDevice *vkdevice,
		VulkanCommandPool *vkcommandpool,
		VmaAllocator vkallocator,
		VkImageCreateInfo *image_info,
		VmaAllocationCreateInfo *create_info
	);
	void fini() override;

	void stage(VulkanBuffer *staging_buffer, VkExtent3D image_extent);

	ResourceState get_state() override { return state; }

	VkImage &get_image() { return image; }

	VkImageCreateInfo *get_info() { return &info; }

	friend class VulkanResourceManager;
	friend class VulkanDescriptorManager;

	VK_TRACY_MEMORY_OVERLOADS;

private:
	VulkanDevice *device;
	VulkanCommandPool *command_pool;
	VkImageCreateInfo info;

	VkImage image;

	VmaAllocator allocator;
	VmaAllocation allocation;

	ResourceState state = ResourceState::UNREADY;
};