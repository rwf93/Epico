#define EAPI_EXPORT
#include <platform/platform.h>
#include <abstractrenderer.h>

#include <app/appcontext.h>

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

extern "C" EAPI AbstractRenderer *create_factory(void *user_data) {
    return new VulkanRenderer(reinterpret_cast<AppContext*>(user_data));
}