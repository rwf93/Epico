#pragma once

class VulkanInstance {
public:
	VulkanInstance() = default;
	~VulkanInstance() = default;

	void init(FunctorQueue<> &queue);
	void fini();

	vkb::Instance &get_instance() { return instance; }
private:
	vkb::Instance instance = {};
};