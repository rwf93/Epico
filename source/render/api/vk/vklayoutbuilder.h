#pragma once

class VulkanDevice;
class VulkanLayout;
class VulkanResourceManager;
class VulkanLayoutBuilder: public RenderLayoutBuilder {
public:
	VulkanLayoutBuilder(VulkanDevice *vkdevice, VulkanResourceManager *vkresourcemanager)
		: device(vkdevice)
		, resource_manager(vkresourcemanager) { clear(); }

	~VulkanLayoutBuilder() { clear(); }

	void clear();

	RenderLayoutBuilder &add_uniform(ShaderStage stage, UniformType type) override;
	LayoutHandle build();

private:
	VulkanDevice *device;
	VulkanResourceManager *resource_manager;

	std::vector<VkDescriptorSetLayoutBinding> binding_infos = {};

	VulkanLayout *layout;
};