#include "vklayout.h"
#include "vklayoutbuilder.h"

void VulkanLayoutBuilder::init(VulkanDevice *vkdevice) {
	this->device = vkdevice;
}

void VulkanLayoutBuilder::fini() {

}

void VulkanLayoutBuilder::clear(LayoutHandle handle, VulkanLayout *vklayout) {
	layout_handle = handle;
	layout = vklayout;

	binding_infos.clear();
}

RenderLayoutBuilder *VulkanLayoutBuilder::add_uniform(ShaderStage stage) {
	uint64_t stage_bits = 0;

	if(stage & ShaderStage::VERTEX)
		stage_bits |= VK_SHADER_STAGE_VERTEX_BIT;

	if(stage & ShaderStage::FRAGMENT)
		stage_bits |= VK_SHADER_STAGE_FRAGMENT_BIT;

	auto info = info::descriptor_set_layout_binding(
		VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		static_cast<VkShaderStageFlagBits>(stage_bits),
		static_cast<uint32_t>(binding_infos.size() + 1)
	);

	binding_infos.push_back(info);

	return this;
}

LayoutHandle VulkanLayoutBuilder::build() {
	auto layout_info = info::descriptor_set_layout_info(binding_infos);

	layout->init(device, &layout_info);

	return layout_handle;
}