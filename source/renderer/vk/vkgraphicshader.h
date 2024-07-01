#pragma once

class VulkanGraphicShader: public AbstractShader {
public:
    ShaderType get_type() override { return ShaderType::GRAPHIC; }
    ShaderState get_state() override { return state; }

private:
    ShaderState state = ShaderState::SHADER_UNREADY;
};