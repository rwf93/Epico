#pragma once

class VulkanDevice;
class VulkanProgramManager;
class VulkanGraphicsProgram;
class VulkanResourceManager;
class VulkanGraphicsProgramBuilder: public GraphicsProgramBuilder {
public:
	VulkanGraphicsProgramBuilder() = default;
	~VulkanGraphicsProgramBuilder() override = default;

	GraphicsProgramHandle build() override;

	void init(VulkanDevice *vkdevice, VulkanResourceManager *vkresourcemanager);
	void clear(GraphicsProgramHandle program_handle);

	GraphicsProgramBuilder *set_primitive(ShaderPrimitive type) override;
	GraphicsProgramBuilder *set_polygon_mode(ShaderPolygonMode mode) override;
	GraphicsProgramBuilder *set_depth_format(ImageFormat format) override;
	GraphicsProgramBuilder *set_depth_test(
		bool test_enable,
		bool write_enable,
		ShaderCompareOp compare
	) override;

	GraphicsProgramBuilder *add_binding(
        uint32_t size,
        BindingRate rate,
        uint32_t binding = 0
    ) override;

	GraphicsProgramBuilder *add_attribute(
		uint32_t offset,
		AttributeType type,
		uint32_t binding = 0
	) override;

	GraphicsProgramBuilder *add_stage(
		ShaderStage stage,
		const char *data,
		size_t size
	) override;

	GraphicsProgramBuilder *set_layout(LayoutHandle layout);

	GraphicsProgramBuilder *add_attachment(ImageFormat format) override;
private:
	VulkanDevice *device;
	VulkanResourceManager *resource_manager;

	GraphicsProgramHandle handle;

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

	LayoutHandle current_pipeline_layout;
	LayoutHandle default_pipeline_layout; // Default layout.

	VkFormat depth_format = {};
};