#pragma once

class VulkanInstance {
public:
	VulkanInstance() = default;
	~VulkanInstance() { vkb::destroy_instance(instance); };

	void init();

	vkb::Instance &get_instance() { return instance; }
private:
	vkb::Instance instance = {};
};