#include "vkdevice.h"
#include "vkswapchain.h"
#include "vkcommandpool.h"
#include "vkresourcemanager.h"
#include "vkdescriptorbuffer.h"
#include "vkdescriptormanager.h"

void VulkanDescriptorManager::init(
	FunctorQueue<> &queue,
	VulkanDevice *vkdevice,
	VulkanCommandPool *vkcommandpool,
	VulkanResourceManager *vkresourcemanager
) {
	this->device = vkdevice;
	this->command_pool = vkcommandpool;
	this->resource_manager = vkresourcemanager;

	queue.push([&]() { fini(); });
}

UniformHandle VulkanDescriptorManager::create_uniform_buffer() {
	UniformHandle last_uniform_handle = advance_handle();

	std::vector<AbstractUniform*> buffer_uniforms = {};
	for(uint32_t i = 0; i < command_pool->get_max_flying_frames(); i++) {
		buffer_uniforms.push_back(new VulkanUniformBuffer());
	}

	uniforms.insert(std::make_pair(last_uniform_handle, buffer_uniforms));
	uniform_resources.insert(std::make_pair(last_uniform_handle, resource_manager->create_buffer()));

	return last_uniform_handle;
}

void VulkanDescriptorManager::fini() {
	for(auto &uniform_handle: uniforms) {
		for(auto &uniform: uniform_handle.second) {
			if(uniform->get_state() == UniformState::UNIFORM_READY)
				uniform->fini();
			delete uniform;
		}
	}
}