#include "vkdevice.h"
#include "vkgraphicshader.h"

VulkanGraphicShader::VulkanGraphicShader(VulkanDevice *vkdevice) {
    this->device = vkdevice;
}

VulkanGraphicShader::~VulkanGraphicShader() {}

void VulkanGraphicShader::init(VkGraphicsPipelineCreateInfo *pipeline_create_info) {
    layout = pipeline_create_info->layout; // Again, cheesing it here.
    VK_CHECK(vkCreateGraphicsPipelines(device->get_device(), VK_NULL_HANDLE, 1, pipeline_create_info, nullptr, &pipeline));
    state = ShaderState::SHADER_READY;
}

void VulkanGraphicShader::fini() {
    vkDestroyPipeline(device->get_device(), pipeline, nullptr);
    state = ShaderState::SHADER_READY;
}
