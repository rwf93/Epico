#pragma once

class VulkanDevice;
class VulkanComputeShader: public AbstractShader {
public:
	VulkanComputeShader(VulkanDevice *vkdevice);
	~VulkanComputeShader() override;

	ShaderType get_type() override { return ShaderType::COMPUTE; }
	ShaderState get_state() override { return state; }

	void fini() override;

private:
	VulkanDevice *device;
	ShaderState state = ShaderState::SHADER_UNREADY;
};