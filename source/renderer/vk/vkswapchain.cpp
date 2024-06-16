#include "vkswapchain.h"
#include "vkdevice.h"

VulkanSwapchain::VulkanSwapchain(VulkanDevice *device) {
    this->device = device;
    create_swapchain();
}

VulkanSwapchain::~VulkanSwapchain() {
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