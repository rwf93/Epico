#include "vkgraphicshader.h"

VulkanGraphicShader::VulkanGraphicShader(VulkanDevice *vkdevice) {
    this->device = vkdevice;
}

VulkanGraphicShader::~VulkanGraphicShader() {}

void VulkanGraphicShader::fini() {
    state = ShaderState::SHADER_UNREADY;
}