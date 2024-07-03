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

	VkPipeline get_pipeline() { return pipeline; }

	AbstractGraphicShader *set_primitive(ShaderPrimitive type) override;

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

	AbstractGraphicShader *add_attachment(ImageFormat format) override;

private:
	VulkanDevice *device;
	VulkanShaderManager *shader_manager;

	ShaderHandle handle;
	ShaderState state = ShaderState::SHADER_UNREADY;

    VkPipelineInputAssemblyStateCreateInfo assembly_info = {};
	VkPipelineViewportStateCreateInfo viewport_info = {};
	VkPipelineRasterizationStateCreateInfo rasterizer_info = {};
	VkPipelineMultisampleStateCreateInfo multisampling_info = {};
	VkPipelineColorBlendStateCreateInfo color_info = {};
	VkPipelineDepthStencilStateCreateInfo stencil_info = {};

	std::vector<VkVertexInputBindingDescription> bindings = {};
	std::vector<VkVertexInputAttributeDescription> attributes = {};

	std::vector<VkShaderModule> shader_modules = {};
	std::vector<VkPipelineShaderStageCreateInfo> shader_stages = {};

	std::vector<VkPipelineColorBlendAttachmentState> color_states = {};
	std::vector<VkFormat> attachment_formats = {};

	VkPipelineLayout pipeline_layout;
	VkPipeline pipeline;
};