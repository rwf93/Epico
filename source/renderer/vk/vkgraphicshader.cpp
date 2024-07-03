#include "vkdevice.h"
#include "vkshadermanager.h"
#include "vkgraphicshader.h"

VulkanGraphicShader::VulkanGraphicShader(
    ShaderHandle shader_handle,
    VulkanDevice *vkdevice,
    VulkanShaderManager *vkshadermanager
) {
    this->device = vkdevice;
    this->shader_manager = vkshadermanager;
    this->handle = shader_handle;
}

VulkanGraphicShader::~VulkanGraphicShader() {}

ShaderHandle VulkanGraphicShader::init() {
    std::vector<VkDescriptorSetLayout> descriptor_layouts = {};
    auto pipeline_layout_info = info::pipeline_layout_info(descriptor_layouts);

    VK_CHECK(vkCreatePipelineLayout(device->get_device(), &pipeline_layout_info, nullptr, &pipeline_layout));

    auto input_info = info::input_vertex_info(bindings, attributes);

    for(auto &module: shader_modules)
        vkDestroyShaderModule(device->get_device(), module, nullptr);

    state = ShaderState::SHADER_READY;
    return handle;
}

void VulkanGraphicShader::fini() {
    vkDestroyPipelineLayout(device->get_device(), pipeline_layout, nullptr);

    state = ShaderState::SHADER_UNREADY;
}

AbstractGraphicShader *VulkanGraphicShader::add_binding(
    uint32_t binding,
    uint32_t size,
    BindingRate rate
) {
    VkVertexInputBindingDescription binding_description = {};
    binding_description.binding = binding;
    binding_description.stride = size;
    binding_description.inputRate = convert::convert_binding_rate(rate);

    bindings.push_back(binding_description);

    return this;
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

AbstractGraphicShader *VulkanGraphicShader::add_stage(
    ShaderStage stage,
    const char *data,
    size_t size
) {
    VkShaderModuleCreateInfo shader_create_info = {};
    shader_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shader_create_info.pCode = reinterpret_cast<const uint32_t*>(data);
    shader_create_info.codeSize = size;

    VkShaderModule shader_module = VK_NULL_HANDLE;
    VK_CHECK(vkCreateShaderModule(device->get_device(), &shader_create_info, nullptr, &shader_module));
    shader_modules.push_back(shader_module);

    VkPipelineShaderStageCreateInfo shader_stage_info = {};
    shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shader_stage_info.module = shader_module;
    shader_stage_info.stage = convert::convert_shader_stage(stage);
    shader_stage_info.pName = "main";

    shader_stages.push_back(shader_stage_info);

    return this;
}