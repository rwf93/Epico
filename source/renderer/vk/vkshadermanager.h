#pragma once

#include "vkgraphicshader.h"
#include "vkcomputeshader.h"

class VulkanDevice;
class VulkanShaderManager {
public:
	VulkanShaderManager();
	~VulkanShaderManager();

	void init(FunctorQueue<> &queue, VulkanDevice *vkdevice);
	void fini();

	AbstractGraphicShader *create_graphic_shader();
	AbstractComputeShader *create_compute_shader();

	VulkanGraphicShader *get_graphic_shader(ShaderHandle handle) {
		return get_shader<VulkanGraphicShader*>(handle);
	}

	VulkanComputeShader *get_compute_shader(ShaderHandle handle) {
		return get_shader<VulkanComputeShader*>(handle);
	}

	template<typename T>
	T get_shader(ShaderHandle handle) {
		AbstractShader *shader = nullptr;
		if(!shaders.contains(handle))
			goto err;

		shader = shaders.at(handle);
		if(!shader)
			goto err;

		return dynamic_cast<T>(shader);
	err:
		return nullptr;
	}

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