#include "vkdevice.h"
#include "vkbuffer.h"
#include "vkcommandpool.h"
#include "vktexture.h"

void VulkanTexture::init(
	VulkanDevice *vkdevice,
	VulkanCommandPool *vkcommandpool,
	VmaAllocator vkallocator,
	VkImageCreateInfo *image_info,
	VmaAllocationCreateInfo *create_info
) {
	this->device = vkdevice;
	this->command_pool = vkcommandpool;
	this->allocator = vkallocator;
	this->info = *image_info;

	VK_CHECK(vmaCreateImage(allocator, image_info, create_info, &image, &allocation, nullptr));
	state = ResourceState::READY;
}

void VulkanTexture::stage(VulkanBuffer *staging_buffer, VkExtent3D image_extent) {
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

		command->transition_image(get_image(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	});
}

void VulkanTexture::fini() {
	vmaDestroyImage(allocator, image, allocation);
	state = ResourceState::UNREADY;
}

void VulkanTexture::transition(VkImageLayout new_layout) {
	if(last_layout == new_layout) return;
	command_pool->get_command()->transition_image(get_image(), last_layout, new_layout);
	last_layout = new_layout;
}