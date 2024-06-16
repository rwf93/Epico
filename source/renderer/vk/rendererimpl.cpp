#include "vkinstance.h"
#include "vksurface.h"
#include "vkdevice.h"
#include "vkswapchain.h"
#include "vkcommandpool.h"
#include "vkimgui.h"

#include "rendererimpl.h"

VulkanRenderer::VulkanRenderer(AppContext *app_context) {
    this->app_context = app_context;

    instance.init(cleanup_queue);
    surface.init(cleanup_queue, app_context, &instance);
    device.init(cleanup_queue, &instance, &surface);
    swapchain.init(cleanup_queue, &device);
    command_pool.init(cleanup_queue, &device, &swapchain);
}

VulkanRenderer::~VulkanRenderer() {
    cleanup_queue.destroy();
}

void VulkanRenderer::begin() {
    command_pool.wait_fences();
    command_pool.reset_fences();

    VkResult aquire_result = vkAcquireNextImageKHR(
        device.get_device(),
        swapchain.get_swapchain(),
        UINT64_MAX,
        command_pool.get_available_semaphore(),
        VK_NULL_HANDLE,
        &swapchain.get_image_index()
    );
    if(aquire_result == VK_ERROR_OUT_OF_DATE_KHR) {
        rebuild();
    }

    command_pool.begin_recording();

    swapchain.transition_image(
        command_pool.get_command(),
        swapchain.get_swapchain_image(),
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_GENERAL
    );
}

void VulkanRenderer::end() {
    swapchain.transition_image(
        command_pool.get_command(),
        swapchain.get_swapchain_image(),
        VK_IMAGE_LAYOUT_GENERAL,
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
    );

    command_pool.end_recording();

    auto command_info = info::command_buffer_submit_info(command_pool.get_command());
    auto wait_info = info::semaphore_submit_info(
        VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR,
        command_pool.get_available_semaphore()
    );
    auto signal_info = info::semaphore_submit_info(
        VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT,
        command_pool.get_finished_semaphore()
    );

    auto submit_info = info::submit_info(&command_info, &signal_info, &wait_info);

    VK_CHECK(vkQueueSubmit2(device.get_graphics_queue(), 1, &submit_info, command_pool.get_fence()));

    VkPresentInfoKHR present_info = {};
	present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present_info.pSwapchains = &swapchain.get_swapchain().swapchain;
	present_info.swapchainCount = 1;

	present_info.pWaitSemaphores = &command_pool.get_finished_semaphore();
	present_info.waitSemaphoreCount = 1;

	present_info.pImageIndices = &swapchain.get_image_index();

    VkResult present_result = vkQueuePresentKHR(device.get_graphics_queue(), &present_info);
    if(present_result == VK_ERROR_OUT_OF_DATE_KHR || present_result == VK_SUBOPTIMAL_KHR) {
        rebuild();
    }

    command_pool.advance();
}

void VulkanRenderer::clear(float r, float g, float b, float a) {
    VkClearColorValue clear_value = { { r, g, b, a } };

    auto clear_range = info::image_subresource_range(VK_IMAGE_ASPECT_COLOR_BIT);
    vkCmdClearColorImage(
        command_pool.get_command(),
        swapchain.get_swapchain_image(),
        VK_IMAGE_LAYOUT_GENERAL,
        &clear_value,
        1,
        &clear_range
    );
}

void VulkanRenderer::rebuild() {
    swapchain.rebuild();
    command_pool.rebuild();
}

static VulkanRenderer *singleton;

extern "C" EAPI AbstractRenderer *create_factory(void *user_data) {
    if(!singleton)
        singleton = new VulkanRenderer(reinterpret_cast<AppContext*>(user_data));
    return singleton;
}