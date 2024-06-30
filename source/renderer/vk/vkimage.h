#pragma once

class VulkanDevice;
class VulkanCommandPool;
class VulkanBuffer;
class VulkanImage: public AbstractResource {
public:
	VulkanImage(
		VulkanDevice *vkdevice,
		VulkanCommandPool *vkcommandpool,
		VmaAllocator vkallocator
	);
	~VulkanImage() override;

	void init(VkImageCreateInfo *image_info, VmaAllocationCreateInfo *create_info);
	void fini() override;

	void stage(VulkanBuffer *staging_buffer, VkExtent3D image_extent);

	ResourceState get_state() override { return state; }

	VkExtent3D &get_extent() { return extent; }

	VkImage &get_image() { return image; }
	VkImageView &get_view() { return view; }

	ResourceType get_type() { return ResourceType::IMAGE; }
private:
	VulkanDevice *device;
	VulkanCommandPool *command_pool;

	VmaAllocator allocator;

	VkImage image;
	VkImageView view;

	VmaAllocation allocation;

	VkExtent3D extent;

	ResourceState state = ResourceState::UNREADY;
};