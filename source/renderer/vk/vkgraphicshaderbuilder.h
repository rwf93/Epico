#pragma once

class VulkanDevice;
class VulkanShaderManager;
class VulkanGraphicShader;
class VulkanGraphicShaderBuilder: public AbstractGraphicShaderBuilder {
public:
	VulkanGraphicShaderBuilder();
	~VulkanGraphicShaderBuilder() override;

	ShaderHandle build() override;

	void init(VulkanDevice *vkdevice);
	void fini();

	void clear(
		ShaderHandle shader_handle,
    	VulkanGraphicShader *vkshader
	);

	AbstractGraphicShaderBuilder *set_primitive(ShaderPrimitive type) override;
	AbstractGraphicShaderBuilder *set_polygon_mode(ShaderPolygonMode mode) override;
	AbstractGraphicShaderBuilder *set_depth_format(ImageFormat format) override;
	AbstractGraphicShaderBuilder *set_depth_test(bool write_enable, ShaderCompareOp compare) override;

	AbstractGraphicShaderBuilder *add_binding(
        uint32_t binding,
        uint32_t size,
        BindingRate rate
    ) override;

	AbstractGraphicShaderBuilder *add_attribute(
		uint32_t location,
		uint32_t binding,
		uint32_t offset,
		AttributeType type
	) override;

	AbstractGraphicShaderBuilder *add_stage(
		ShaderStage stage,
		const char *data,
		size_t size
	) override;

	AbstractGraphicShaderBuilder *add_attachment(ImageFormat format) override;

	AbstractGraphicShaderBuilder *add_uniform(
		uint32_t set,
		uint32_t binding,
		UniformType uniform,
		ShaderStage stage
	) override;

private:
	VulkanDevice *device;

	ShaderHandle handle;
	VulkanGraphicShader *shader;

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

	std::map<uint32_t, std::vector<VkDescriptorSetLayoutBinding>> descriptor_layout_bindings = {};

	std::vector<VkDescriptorSetLayout> descriptor_layouts;
	std::vector<VkPipelineLayout> pipeline_layouts;

	VkFormat depth_format = {};
};