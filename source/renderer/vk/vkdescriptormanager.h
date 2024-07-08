#pragma once

class VulkanDevice;
class VulkanSwapchain;
class VulkanDescriptorAllocator;
class VulkanDescriptorManager {
public:
    void init(FunctorQueue<> &queue, VulkanDevice *vkdevice, VulkanCommandPool *vkcommandpool);
    void fini();

protected:
    UniformHandle advance_handle() {
        UniformHandle last_resource_handle = current_uniform_handle;
		current_uniform_handle++;
		return last_resource_handle;
    }
private:
    VulkanDevice *device;
    VulkanCommandPool *command_pool;

    std::vector<VulkanDescriptorAllocator*> allocators;
    std::map<UniformHandle, std::vector<AbstractUniform*>> uniforms;

    UniformHandle current_uniform_handle = 0;
};