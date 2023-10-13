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
    auto builder_ret = rebuild ? builder.set_old_swapchain(swapchain).build() : builder.build();

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