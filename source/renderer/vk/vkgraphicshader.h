#pragma once

class VulkanDevice;
class VulkanShaderManager;
class VulkanGraphicShader: public AbstractGraphicShader {
public:
	VulkanGraphicShader(
		ShaderHandle shader_handle,
		VulkanDevice *vkdevice,
		VulkanShaderManager *vkshadermanager
	);
	~VulkanGraphicShader() override;

	ShaderState get_state() override { return state; }

	ShaderHandle init() override;
	void fini() override;

	AbstractGraphicShader *add_binding(
        uint32_t binding,
        uint32_t size,
        BindingRate rate
    ) override;

	AbstractGraphicShader *add_attribute(
		uint32_t location,
		uint32_t binding,
		uint32_t offset,
		AttributeType type
	) override;

	AbstractGraphicShader *add_stage(
		ShaderStage stage,
		const char *data,
		size_t size
	) override;

private:
	VulkanDevice *device;
	VulkanShaderManager *shader_manager;

	ShaderHandle handle;
	ShaderState state = ShaderState::SHADER_UNREADY;

	std::vector<VkVertexInputBindingDescription> bindings = {};
	std::vector<VkVertexInputAttributeDescription> attributes = {};

	std::vector<VkShaderModule> shader_modules = {};
	std::vector<VkPipelineShaderStageCreateInfo> shader_stages = {};

	VkPipelineLayout pipeline_layout;
};