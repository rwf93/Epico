#pragma once

class VulkanDevice;
class VulkanShader: public RenderResource {
public:
    VulkanShader() = default;
    ~VulkanShader() override { if(get_state() != ResourceState::UNREADY) fini(); };

    void init(VulkanDevice *vkdevice, VkShaderModuleCreateInfo *vkcreateinfo, VkPipelineShaderStageCreateInfo *vkstageinfo);

    ResourceState get_state() override { return state; }
	void fini() override;
private:
    friend class VulkanGraphicsProgramBuilder;

    VulkanDevice *device;
    VkShaderModule shader_module;

    VkPipelineShaderStageCreateInfo stage_info;

    ResourceState state = ResourceState::UNREADY;
};