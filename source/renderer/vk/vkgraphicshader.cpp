#include "vkgraphicshader.h"

VulkanGraphicShader::VulkanGraphicShader(ShaderHandle shader_handle, VulkanDevice *vkdevice) {
    this->device = vkdevice;
    this->handle = shader_handle;
}

VulkanGraphicShader::~VulkanGraphicShader() {}

ShaderHandle VulkanGraphicShader::init() {
    state = ShaderState::SHADER_READY;
    return handle;
}

void VulkanGraphicShader::fini() {
    state = ShaderState::SHADER_UNREADY;
}

AbstractGraphicShader *VulkanGraphicShader::add_attribute(
    	uint32_t location,
		uint32_t binding,
		uint32_t offset,
		AttributeType type
) {
    VkVertexInputAttributeDescription attribute_description = {};
    attribute_description.location = location;
    attribute_description.binding = binding;
    attribute_description.offset = offset;
    attribute_description.format = convert::convert_attribute_format(type);
    attributes.push_back(attribute_description);

    return this;
}