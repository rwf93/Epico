#define EAPI_EXPORT
#include <factory.h>

#include <abstractrenderer.h>
#include "rendererimpl.h"

#include <spdlog/spdlog.h>

VulkanRenderer::VulkanRenderer() {
    spdlog::info("Vulkan Renderer Instantiated");
}

VulkanRenderer::~VulkanRenderer() {
    spdlog::info("Vulkan Renderer Destroyed");
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
