#define EAPI_EXPORT
#include <factory.h>

#include <abstractrenderer.h>
#include "rendererimpl.h"

#include <spdlog/spdlog.h>

void VulkanRenderer::create() {
    spdlog::info("Vulkan Rendering Implementation");
}

void VulkanRenderer::destroy() {
    delete this;
}

void VulkanRenderer::begin() {

}

void VulkanRenderer::end() {

}

void VulkanRenderer::begin_pass() {

}

void VulkanRenderer::end_pass() {

}

AbstractRenderer *create_vulkan_renderer() {
    return new VulkanRenderer();
}
