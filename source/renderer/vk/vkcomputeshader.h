#pragma once

class VulkanDevice;
class VulkanComputeShader: public AbstractShader {
public:
    ShaderType get_type() override { return ShaderType::COMPUTE; }
    ShaderState get_state() override { return state; }

private:
    VulkanDevice *device;
    ShaderState state = ShaderState::SHADER_UNREADY;
};