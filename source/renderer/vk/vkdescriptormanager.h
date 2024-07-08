#pragma once

class VulkanDevice;
class VulkanSwapchain;
class VulkanResourceManager;
class VulkanDescriptorManager {
public:
	void init(
		FunctorQueue<> &queue,
		VulkanDevice *vkdevice,
		VulkanCommandPool *vkcommandpool,
		VulkanResourceManager *vkresourcemanager
	);
	void fini();

	UniformHandle create_uniform_buffer();
	void init_uniform_buffer(UniformHandle uniform_handle, ResourceHandle resource_handle);

	// This code sadly makes a copy...
	template<typename T>
	std::vector<T*> get_uniform(UniformHandle handle) {
		if(!uniforms.contains(handle))
			return {};

		std::vector<T*> const &uniforms_list = reinterpret_cast<std::vector<T*>&>(uniforms.at(handle));
		return uniforms_list;
	}

protected:
	UniformHandle advance_handle() {
		UniformHandle last_uniform_handle = current_uniform_handle;
		current_uniform_handle++;
		return last_uniform_handle;
	}
private:
	VulkanDevice *device;
	VulkanCommandPool *command_pool;
	VulkanResourceManager *resource_manager;

	// Tied {handle = {...}} where ... is max flying frames.
	std::map<UniformHandle, std::vector<AbstractUniform*>> uniforms;
	std::map<UniformHandle, ResourceHandle> uniform_resources;

	UniformHandle current_uniform_handle = 0;
};