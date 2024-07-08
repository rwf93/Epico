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

protected:
    UniformHandle advance_handle() {
        UniformHandle last_resource_handle = current_uniform_handle;
		current_uniform_handle++;
		return last_resource_handle;
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