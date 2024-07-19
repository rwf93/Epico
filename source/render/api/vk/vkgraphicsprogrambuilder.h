#pragma once

class VulkanDevice;
class VulkanProgramManager;
class VulkanGraphicsProgram;
class VulkanLayoutManager;
class VulkanGraphicsProgramBuilder: public RenderGraphicProgramBuilder {
public:
	VulkanGraphicsProgramBuilder();
	~VulkanGraphicsProgramBuilder() override;

	GraphicsProgramHandle build() override;

	void init(VulkanDevice *vkdevice, VulkanLayoutManager *vklayoutmanager);
	void fini();

	void clear(
		GraphicsProgramHandle shader_handle,
    	VulkanGraphicsProgram *vkshader
	);

	RenderGraphicProgramBuilder *set_primitive(ShaderPrimitive type) override;
	RenderGraphicProgramBuilder *set_polygon_mode(ShaderPolygonMode mode) override;
	RenderGraphicProgramBuilder *set_depth_format(ImageFormat format) override;
	RenderGraphicProgramBuilder *set_depth_test(bool write_enable, ShaderCompareOp compare) override;

	RenderGraphicProgramBuilder *add_binding(
        uint32_t binding,
        uint32_t size,
        BindingRate rate
    ) override;

	RenderGraphicProgramBuilder *add_attribute(
		uint32_t location,
		uint32_t binding,
		uint32_t offset,
		AttributeType type
	) override;

	RenderGraphicProgramBuilder *add_stage(
		ShaderStage stage,
		const char *data,
		size_t size
	) override;

	RenderGraphicProgramBuilder *add_layout(LayoutHandle layout);

	RenderGraphicProgramBuilder *add_attachment(ImageFormat format) override;
private:
	VulkanDevice *device;
	VulkanLayoutManager *layout_manager;

	GraphicsProgramHandle handle;
	VulkanGraphicsProgram *shader;

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

	std::vector<VkDescriptorSetLayout> descriptor_layouts;
	std::vector<VkPipelineLayout> pipeline_layouts;

	VkFormat depth_format = {};
};