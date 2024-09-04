#pragma once

class VulkanDevice;
class VulkanLayout;
class VulkanResourceManager;
class VulkanLayoutBuilder: public RenderLayoutBuilder {
public:
	void init(VulkanDevice *vkdevice, VulkanResourceManager *vkresourcemanager);
	void clear(LayoutHandle handle);

	RenderLayoutBuilder &add_uniform(ShaderStage stage, UniformType type) override;
	LayoutHandle build();

private:
	VulkanDevice *device;
	VulkanResourceManager *resource_manager;

	std::vector<VkDescriptorSetLayoutBinding> binding_infos = {};

	LayoutHandle layout_handle;
	VulkanLayout *layout;
};