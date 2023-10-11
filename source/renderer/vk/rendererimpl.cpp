#define EAPI_EXPORT
#include <factory.h>
#include <abstractrenderer.h>

#include "vkinstance.h"
#include "vksurface.h"
#include "vkdevice.h"
#include "vkswapchain.h"

#include "rendererimpl.h"

VulkanRenderer::VulkanRenderer() {
}

VulkanRenderer::~VulkanRenderer() {
}

void VulkanRenderer::begin() {
}

void VulkanRenderer::end() {

}

void VulkanRenderer::begin_pass() {

}

void VulkanRenderer::end_pass() {

}

std::shared_ptr<AbstractRenderer> create_vulkan_renderer() {
    return std::make_shared<VulkanRenderer>();
}
