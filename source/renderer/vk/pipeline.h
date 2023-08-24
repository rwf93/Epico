#pragma once

#define FUNC_PIPELINE_CONSTRUCTOR VkDevice device, VkRenderPass render_pass, VkPipelineCache pipeline_cache
#define FUNC_PIPELINE_ADD_SHADER VkShaderStageFlagBits stage, VkShaderModule shader
#define FUNC_PIPELINE_BUILD VkPipeline *pipeline, VkPipelineLayout *pipeline_layout


namespace render {
// 4l8r
class RenderPipelineConstructor {
public:
    RenderPipelineConstructor(FUNC_PIPELINE_CONSTRUCTOR);
    ~RenderPipelineConstructor();

    void add_shader(FUNC_PIPELINE_ADD_SHADER);
    void build(FUNC_PIPELINE_BUILD);

public:
    VkPipelineVertexInputStateCreateInfo input_info = {};
    VkPipelineInputAssemblyStateCreateInfo input_assembly = {};

    VkPipelineViewportStateCreateInfo viewport_state = {};

    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    VkPipelineMultisampleStateCreateInfo multisampling = {};

    std::vector<VkPipelineColorBlendAttachmentState> color_attachments = {};
    VkPipelineColorBlendStateCreateInfo color_blending = {};

    std::vector<VkDynamicState> dynamic_states = {};

    std::vector<VkPipelineShaderStageCreateInfo> shader_stages = {};

    VkPipelineLayoutCreateInfo pipeline_layout_info = {};
    VkGraphicsPipelineCreateInfo pipeline_info = {};
private:
    VkDevice device;
    VkRenderPass pass;
    VkPipelineCache cache;
};

struct PipelinePair {
    VkPipeline pipeline;
    VkPipelineLayout layout;

    operator VkPipeline() { return pipeline; };
    operator VkPipelineLayout() { return layout; };
};

}