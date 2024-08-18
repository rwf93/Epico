#pragma once

class VulkanDevice;
class VulkanCommandPool;
class VulkanBuffer;
class VulkanTexture: public RenderResource {
public:
	~VulkanTexture() override { if(get_state() != ResourceState::UNREADY) fini(); }

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

	void transition(VkImageLayout new_layout);

	friend class VulkanResourceManager;
	friend class VulkanDescriptorManager;

private:
	VulkanDevice *device;
	VulkanCommandPool *command_pool;
	VkImageCreateInfo info;

	VkImage image;

	VmaAllocator allocator;
	VmaAllocation allocation;

	ResourceState state = ResourceState::UNREADY;
	VkImageLayout last_layout = VK_IMAGE_LAYOUT_UNDEFINED;
};