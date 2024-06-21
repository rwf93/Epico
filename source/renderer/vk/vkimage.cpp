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
}

void VulkanImage::fini() {

}


VulkanImage::~VulkanImage() {
}