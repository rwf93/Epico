#define EAPI_EXPORT
#include <platform/platform.h>
#include <abstractrenderer.h>

#include "info.h"

#include "vkinstance.h"
#include "vksurface.h"
#include "vkdevice.h"
#include "vkswapchain.h"

#include "rendererimpl.h"

VulkanRenderer::VulkanRenderer() {
}

VulkanRenderer::~VulkanRenderer() {
    spdlog::info("Killing VK");
}

void VulkanRenderer::begin() {
}

void VulkanRenderer::end() {

}

void VulkanRenderer::begin_pass() {

}

void VulkanRenderer::end_pass() {

}

extern "C" EAPI AbstractRenderer *create_factory() {
    return new VulkanRenderer();
}