#include "vkinfo.h"

#include "vkswapchain.h"
#include "vkdevice.h"
#include "vkcommandpool.h"

VulkanSwapchain::VulkanSwapchain() {}
VulkanSwapchain::~VulkanSwapchain() {}

void VulkanSwapchain::init(FunctorQueue<> &queue, VulkanDevice *vkdevice) {
	this->device = vkdevice;
	create_swapchain();

	queue.push([&] { fini(); });
}

void VulkanSwapchain::fini() {
	swapchain.destroy_image_views(swapchain_image_views);
	vkb::destroy_swapchain(swapchain);
}

void VulkanSwapchain::create_swapchain(bool rebuild) {
	vkb::SwapchainBuilder builder(device->get_device());
	builder = rebuild ? builder.set_old_swapchain(swapchain) : builder;

	VkSurfaceFormatKHR swapchain_format = {};
	swapchain_format.format = VK_FORMAT_B8G8R8A8_UNORM;
	swapchain_format.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

	auto builder_ret = builder
		.set_desired_format(swapchain_format)
		.set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
		.add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
		.build();

	if(!builder_ret.has_value()) {
		spdlog::error("Couldn't create Vulkan Swapchain: {}", builder_ret.error().message());
		std::abort();
	}

	if(rebuild) {
		swapchain.destroy_image_views(swapchain_image_views);
		vkb::destroy_swapchain(swapchain);
	}

	swapchain = builder_ret.value();
	swapchain_images = swapchain.get_images().value();
	swapchain_image_views = swapchain.get_image_views().value();
}

void VulkanSwapchain::transition_image(
	VkCommandBuffer command,
	VkImage image,
	VkImageLayout current_layout,
	VkImageLayout new_layout
) {
	VkImageMemoryBarrier2 image_barrier = {};
	image_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
	image_barrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
	image_barrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;
	image_barrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
	image_barrier.dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT;
	image_barrier.oldLayout = current_layout;
	image_barrier.newLayout = new_layout;

	VkImageAspectFlags aspect_mask = (new_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL)
		? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;

	image_barrier.subresourceRange = info::image_subresource_range(aspect_mask);
	image_barrier.image = image;

	VkDependencyInfo dependency_info = {};
	dependency_info.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
	dependency_info.imageMemoryBarrierCount = 1;
	dependency_info.pImageMemoryBarriers = &image_barrier;

	vkCmdPipelineBarrier2(command, &dependency_info);
}

bool VulkanSwapchain::aquire_next_image(VulkanCommandPool *command_pool) {
	VkResult aquire_result = vkAcquireNextImageKHR(
		device->get_device(),
		get_swapchain(),
		UINT64_MAX,
		command_pool->get_available_semaphore(),
		VK_NULL_HANDLE,
		&get_image_index()
	);

	if(aquire_result == VK_ERROR_OUT_OF_DATE_KHR) {
		return true;
	}

	return false;
}