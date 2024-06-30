#include "vkdevice.h"
#include "vkbuffer.h"
#include "vkcommandpool.h"
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

void VulkanImage::init(VkImageCreateInfo *image_info, VmaAllocationCreateInfo *create_info) {
	// evil
	extent = image_info->extent;

	VK_CHECK(vmaCreateImage(allocator, image_info, create_info, &image, &allocation, nullptr));

	VkImageViewCreateInfo image_view_info = {};
	image_view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	image_view_info.image = get_image();
	image_view_info.viewType = (VkImageViewType)image_info->imageType; // this is actually illegal as fuck.
	image_view_info.format = image_info->format;
	image_view_info.subresourceRange.baseMipLevel = 0;
	image_view_info.subresourceRange.levelCount = image_info->mipLevels;
	image_view_info.subresourceRange.baseArrayLayer = 0;
	image_view_info.subresourceRange.layerCount = 1;
	image_view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

	if(image_view_info.format == VK_FORMAT_D32_SFLOAT)
		image_view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

	VK_CHECK(vkCreateImageView(device->get_device(), &image_view_info, nullptr, &get_view()));

	state = ResourceState::READY;
}

void VulkanImage::stage(VulkanBuffer *staging_buffer, VkExtent3D image_extent) {
	command_pool->submit_command([&](VkCommandBuffer command) {
		command_pool->transition_image(command, get_image(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		VkBufferImageCopy copy = {};
		copy.bufferOffset = 0;
		copy.bufferRowLength = 0;
		copy.bufferImageHeight = 0;
		copy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		copy.imageSubresource.mipLevel = 0;
		copy.imageSubresource.baseArrayLayer = 0;
		copy.imageSubresource.layerCount = 1;
		copy.imageExtent = image_extent;

		vkCmdCopyBufferToImage(
			command,
			staging_buffer->get_buffer(),
			get_image(),
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1,
			&copy
		);

		command_pool->transition_image(command, get_image(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	});
}

void VulkanImage::fini() {
	vkDestroyImageView(device->get_device(), get_view(), nullptr);
	vmaDestroyImage(allocator, image, allocation);
	state = ResourceState::UNREADY;
}


VulkanImage::~VulkanImage() {
}