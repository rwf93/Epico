#pragma once

class VulkanDevice;
class VulkanGraphicShader: public AbstractShader {
public:
	VulkanGraphicShader(VulkanDevice *vkdevice);
	~VulkanGraphicShader() override;

	ShaderType get_type() override { return ShaderType::GRAPHIC; }
	ShaderState get_state() override { return state; }

	void fini() override;
private:
	VulkanDevice *device;
	ShaderState state = ShaderState::SHADER_UNREADY;
};