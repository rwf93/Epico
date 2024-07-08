#include "vkdevice.h"
#include "vkswapchain.h"
#include "vkcommandpool.h"
#include "vkdescriptorallocator.h"
#include "vkdescriptormanager.h"

void VulkanDescriptorManager::init(FunctorQueue<> &queue, VulkanDevice *vkdevice, VulkanCommandPool *vkcommandpool) {
	this->device = vkdevice;
	this->command_pool = vkcommandpool;


	for(uint32_t i = 0; i < command_pool->get_max_flying_frames(); i++) {
		allocators.push_back(new VulkanDescriptorAllocator());
		VulkanDescriptorAllocator *allocator = allocators.at(i);

		std::vector<VulkanDescriptorAllocator::PoolSizeRatio> pool_ratios = {
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 3 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 3 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4 },
		};

		allocator->init(device, 1000, pool_ratios);
	}


	queue.push([&]() { fini(); });
}

void VulkanDescriptorManager::fini() {
	for(uint32_t i = 0; i < command_pool->get_max_flying_frames(); i++) {
		VulkanDescriptorAllocator *allocator = allocators.at(i);
		allocator->fini();
		delete allocator;
	}
}