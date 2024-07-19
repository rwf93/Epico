#pragma once

class VulkanDevice;
class VulkanLayout;
class VulkanLayoutBuilder: public RenderLayoutBuilder {
public:
	void init(VulkanDevice *vkdevice);
	void fini();

	void clear(LayoutHandle handle, VulkanLayout *vklayout);

	RenderLayoutBuilder *add_uniform(ShaderStage stage);
	LayoutHandle build();

private:
	VulkanDevice *device;

	std::vector<VkDescriptorSetLayoutBinding> binding_infos = {};

	LayoutHandle layout_handle;
	VulkanLayout *layout;
};