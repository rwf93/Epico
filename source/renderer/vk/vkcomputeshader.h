#pragma once

class VulkanDevice;
class VulkanComputeShader: public AbstractComputeShader {
public:
	VulkanComputeShader(ShaderHandle shader_handle, VulkanDevice *vkdevice);
	~VulkanComputeShader() override;

	ShaderState get_state() override { return state; }

	ShaderHandle init() override;
	void fini() override;

private:
	VulkanDevice *device;
	ShaderHandle handle;
	ShaderState state = ShaderState::SHADER_UNREADY;
};