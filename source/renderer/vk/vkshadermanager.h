#pragma once

class VulkanDevice;
class VulkanShaderManager {
public:
	VulkanShaderManager();
	~VulkanShaderManager();

	void init(FunctorQueue<> &queue, VulkanDevice *vkdevice);
	void fini();

	AbstractGraphicShader *create_graphic_shader();
	AbstractComputeShader *create_compute_shader();

protected:
	ShaderHandle advance_shader_handle() {
		ShaderHandle last_shader_handle = current_shader_handle;
		current_shader_handle++;
		return last_shader_handle;
	}

private:
	VulkanDevice *device;
	std::map<ShaderHandle, AbstractShader*> shaders;
	ShaderHandle current_shader_handle = 0;
};