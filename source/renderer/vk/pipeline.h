#pragma once

#define FUNC_PIPELINE_BUILD VkDevice device, VkRenderPass pass, VkPipelineCache cache
#define FUNC_PIPELINE_ADD_SHADER VkShaderStageFlagBits stage, VkShaderModule shader
#define FUNC_PIPELINE_ADD_VIEWPORT VkViewport viewport
#define FUNC_PIPELINE_ADD_SCISSOR VkRect2D scissor
#define FUNC_PIPELINE_ADD_DYNAMIC_STATE VkDynamicState state

// 4l8r
class renderPipelineConstructor {
public:
    renderPipelineConstructor();
    ~renderPipelineConstructor();

    bool build(FUNC_PIPELINE_BUILD);
    void destroy();

    void pipeline_add_shader(FUNC_PIPELINE_ADD_SHADER);
    void pipeline_add_vertex_input_info();
    void pipeline_add_vertex_input_assembly();
    void pipeline_add_viewport(FUNC_PIPELINE_ADD_VIEWPORT) { this->viewport = viewport; }
    void pipeline_add_scissor(FUNC_PIPELINE_ADD_SCISSOR) { this->scissor = scissor; }
    void pipeline_add_dynamic_state(FUNC_PIPELINE_ADD_DYNAMIC_STATE) { dynamic_states.push_back(state); };

public:
    VkPipelineLayout pipeline_layout;
    VkPipeline pipeline;

private:
    VkDevice device;
    std::vector<VkPipelineShaderStageCreateInfo> shader_stages;
    VkPipelineVertexInputStateCreateInfo vertex_input_info;
    VkPipelineInputAssemblyStateCreateInfo input_assembly;
    VkPipelineRasterizationStateCreateInfo rasterizer;
    VkPipelineColorBlendAttachmentState color_blend_attachment;
    VkPipelineMultisampleStateCreateInfo multisampling;
    VkViewport viewport;
    VkRect2D scissor;
    std::vector<VkDynamicState> dynamic_states;

};
