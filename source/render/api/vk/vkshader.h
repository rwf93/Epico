#pragma once

class VulkanDevice;
class VulkanShader: public RenderResource {
public:
    VulkanShader(
        VulkanDevice *vkdevice
    )
        : device(vkdevice) {}
    ~VulkanShader() override { if(get_state() != ResourceState::UNREADY) fini(); };

    VK_TRACY_MEMORY_OVERLOADS;

    void init(VkShaderModuleCreateInfo *vkcreateinfo, VkPipelineShaderStageCreateInfo *vkstageinfo);

    ResourceState get_state() override { return state; }
	void fini() override;
private:
    friend class VulkanGraphicsProgramBuilder;

    VulkanDevice *device;
    VkShaderModule shader_module;

    VkPipelineShaderStageCreateInfo stage_info;

    ResourceState state = ResourceState::UNREADY;
};