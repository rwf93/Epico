#include "vkdevice.h"
#include "vkbuffer.h"
#include "vkcommandpool.h"
#include "vkimage.h"

void VulkanImage::init(
	VulkanDevice *vkdevice,
	VulkanCommandPool *vkcommandpool,
	VmaAllocator vkallocator,
	VkImageCreateInfo *image_info,
	VmaAllocationCreateInfo *create_info
) {
	this->device = vkdevice;
	this->command_pool = vkcommandpool;
	this->allocator = vkallocator;

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

	if(image_info->usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
		image_view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

	VK_CHECK(vkCreateImageView(device->get_device(), &image_view_info, nullptr, &get_view()));

	state = ResourceState::READY;
}

void VulkanImage::stage(VulkanBuffer *staging_buffer, VkExtent3D image_extent) {
	command_pool->submit_command([&](VulkanCommand *command) {
		command->transition_image(get_image(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		VkBufferImageCopy copy = {};
		copy.bufferOffset = 0;
		copy.bufferRowLength = 0;
		copy.bufferImageHeight = 0;
		copy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		copy.imageSubresource.mipLevel = 0;
		copy.imageSubresource.baseArrayLayer = 0;
		copy.imageSubresource.layerCount = 1;
		copy.imageExtent = image_extent;

		std::vector<VkBufferImageCopy> copy_regions = {
			copy
		};

		command->copy_buffer_to_image(
			staging_buffer->get_buffer(),
			get_image(),
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			copy_regions
		);

		command->transition_image(get_image(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL);
	});
}

void VulkanImage::fini() {
	vkDestroyImageView(device->get_device(), get_view(), nullptr);
	vmaDestroyImage(allocator, image, allocation);
	state = ResourceState::UNREADY;
}


VulkanImage::~VulkanImage() {
}