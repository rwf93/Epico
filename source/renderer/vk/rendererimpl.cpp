#include "tools.h"
#include "info.h"

#include "vkinstance.h"
#include "vksurface.h"
#include "vkdevice.h"
#include "vkswapchain.h"
#include "vkcommandpool.h"
#include "vkattachment.h"
#include "vkpassbuilder.h"

#include "rendererimpl.h"

VulkanRenderer::VulkanRenderer(AppContext *app_context) {
    this->app_context = app_context;
    app_context->current_window = surface.get_window();
}

VulkanRenderer::~VulkanRenderer() {

}

void VulkanRenderer::begin() {
    //command_pool.wait_fences();

#if 0
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
#endif

    command_pool.reset_fences();

    command_pool.begin_recording();
}

void VulkanRenderer::end() {
    command_pool.end_recording();

#if 0
    VkSemaphore wait_semaphores[] = { command_pool.get_available_semaphore() };
    VkSemaphore signal_semaphores[] = { command_pool.get_finished_semaphore() };

    VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

    VkSubmitInfo submit_info = {};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.waitSemaphoreCount = 1;
	submit_info.pWaitSemaphores = wait_semaphores;
	submit_info.pWaitDstStageMask = wait_stages;
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &command_pool.get_command();
	submit_info.signalSemaphoreCount = 1;
	submit_info.pSignalSemaphores = signal_semaphores;

    VK_CHECK(vkQueueSubmit(device.get_graphics_queue(), 1, &submit_info, command_pool.get_fence()));

    VkSwapchainKHR swap_chains[] = { swapchain.get_swapchain() };
    VkPresentInfoKHR present_info = {};
	present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present_info.waitSemaphoreCount = 1;
	present_info.pWaitSemaphores = signal_semaphores;
	present_info.swapchainCount = 1;
	present_info.pSwapchains = swap_chains;
	present_info.pImageIndices = &swapchain.get_image_index();

    VkResult present_result = vkQueuePresentKHR(device.get_present_queue(), &present_info);
    if(present_result == VK_ERROR_OUT_OF_DATE_KHR || present_result == VK_SUBOPTIMAL_KHR) {
        rebuild();
    }
#endif

    command_pool.advance();
}

void VulkanRenderer::begin_pass() {

}

void VulkanRenderer::end_pass() {

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