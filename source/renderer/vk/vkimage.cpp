#include "vkimage.h"

VulkanImage::VulkanImage(
	VulkanDevice *vkdevice,
	VulkanCommandPool *vkcommandpool,
	VmaAllocator vkallocator
) {
	this->device = vkdevice;
	this->command_pool = vkcommandpool;
	this->allocator = vkallocator;
}

void VulkanImage::init(VkImageCreateInfo image_info) {
	UNUSED(image_info);
	prepared = true;
}

void VulkanImage::fini() {
	if(is_prepared()) {}
	prepared = false;
}


VulkanImage::~VulkanImage() {
}