#pragma once

class VulkanDevice;
class VulkanCommandPool;
class VulkanImage: public AbstractResource {
public:
	VulkanImage(
		VulkanDevice *vkdevice,
		VulkanCommandPool *vkcommandpool,
		VmaAllocator vkallocator
	);
	~VulkanImage() override;

	void init(VkImageCreateInfo image_info);
	void fini();

	bool is_prepared() { return prepared; }

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

	VkFormat format;

	bool prepared = false;
};