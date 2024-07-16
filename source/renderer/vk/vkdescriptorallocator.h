#pragma once

class VulkanDevice;
class VulkanDescriptorAllocator {
public:
    struct PoolSizeRatio {
        VkDescriptorType type;
        float ratio;
    };

    void init(VulkanDevice *vkdevice, uint32_t inital_sets, std::span<PoolSizeRatio> pool_ratios);
    void clear();
    void fini();

    VkDescriptorSet allocate(std::vector<VkDescriptorSetLayout> &layouts);
private:
    VulkanDevice *device;

    VkDescriptorPool get_pool();
    VkDescriptorPool create_pool(uint32_t set_count, std::span<PoolSizeRatio> pool_ratios);

    std::vector<PoolSizeRatio> ratios;
    std::vector<VkDescriptorPool> full_pools;
    std::vector<VkDescriptorPool> ready_pools;
    uint32_t sets_per_pool;
};