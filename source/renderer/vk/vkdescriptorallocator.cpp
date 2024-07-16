#include "vkdevice.h"
#include "vkdescriptorallocator.h"

void VulkanDescriptorAllocator::init(VulkanDevice *vkdevice, uint32_t inital_sets, std::span<PoolSizeRatio> pool_ratios) {
	this->device = vkdevice;

	ratios.clear();

	for(auto &r: pool_ratios) {
		ratios.push_back(r);
	}

	VkDescriptorPool new_pool = create_pool(inital_sets, pool_ratios);

	sets_per_pool = static_cast<uint32_t>(inital_sets * 1.5);

	ready_pools.push_back(new_pool);
}

void VulkanDescriptorAllocator::clear() {
	for(auto &p: ready_pools) {
		vkResetDescriptorPool(device->get_device(), p, 0);
	}

	for(auto &p: full_pools) {
		vkResetDescriptorPool(device->get_device(), p, 0);
		ready_pools.push_back(p);
	}

	full_pools.clear();
}

void VulkanDescriptorAllocator::fini() {
	for(auto &p: ready_pools)
		vkDestroyDescriptorPool(device->get_device(), p, nullptr);

	for(auto &p: full_pools)
		vkDestroyDescriptorPool(device->get_device(), p, nullptr);

	full_pools.clear();
}

VkDescriptorSet VulkanDescriptorAllocator::allocate(std::vector<VkDescriptorSetLayout> &layouts) {
	VkDescriptorPool pool_to_use = get_pool();

	VkDescriptorSetAllocateInfo allocate_info = info::descriptor_set_allocate_info(layouts, pool_to_use);

	VkDescriptorSet descriptor_set;
	VkResult res = vkAllocateDescriptorSets(device->get_device(), &allocate_info, &descriptor_set);
	if(res == VK_ERROR_OUT_OF_POOL_MEMORY || res == VK_ERROR_FRAGMENTED_POOL) {
		full_pools.push_back(pool_to_use);

		pool_to_use = get_pool();
		allocate_info.descriptorPool = pool_to_use;
	}

	ready_pools.push_back(pool_to_use);
	return descriptor_set;
}

VkDescriptorPool VulkanDescriptorAllocator::get_pool() {
	VkDescriptorPool new_pool;
	if(ready_pools.size() != 0) {
		new_pool = ready_pools.back();
		ready_pools.pop_back();
	} else {
		new_pool = create_pool(sets_per_pool, ratios);

		sets_per_pool = static_cast<uint32_t>(sets_per_pool * 1.5);
		if(sets_per_pool > 4092)
			sets_per_pool = 4092;
	}

	return new_pool;
}

VkDescriptorPool VulkanDescriptorAllocator::create_pool(uint32_t set_count, std::span<PoolSizeRatio> pool_ratios) {
	std::vector<VkDescriptorPoolSize> pool_sizes = {};
	for(auto &ratio: pool_ratios) {
		pool_sizes.push_back(VkDescriptorPoolSize {
			.type = ratio.type,
			.descriptorCount = static_cast<uint32_t>(ratio.ratio * set_count)
		});
	}

	VkDescriptorPoolCreateInfo pool_info = {};
	pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_info.maxSets = set_count;
	pool_info.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
	pool_info.pPoolSizes = pool_sizes.data();

	VkDescriptorPool new_pool;
	VK_CHECK(vkCreateDescriptorPool(device->get_device(), &pool_info, nullptr, &new_pool));

	return new_pool;
}
