#include "tools.h"
#include "info.h"

#include "vkinstance.h"
#include "vksurface.h"
#include "vkdevice.h"
#include "vkswapchain.h"
#include "vkcommandpool.h"

#include "rendererimpl.h"

VulkanRenderer::VulkanRenderer(AppContext *app_context) {
    this->app_context = app_context;
    app_context->current_window = surface.get_window();
}

VulkanRenderer::~VulkanRenderer() {

}

void VulkanRenderer::begin() {
    command_pool.begin_recording();
}

void VulkanRenderer::end() {
    command_pool.end_recording();
}

void VulkanRenderer::begin_pass() {

}

void VulkanRenderer::end_pass() {

}

static VulkanRenderer *singleton;

extern "C" EAPI AbstractRenderer *create_factory(void *user_data) {
    if(!singleton)
        singleton = new VulkanRenderer(reinterpret_cast<AppContext*>(user_data));
    return singleton;
}