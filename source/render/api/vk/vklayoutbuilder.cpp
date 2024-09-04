#include "vklayout.h"
#include "vklayoutbuilder.h"
#include "vkresourcemanager.h"

void VulkanLayoutBuilder::init(VulkanDevice *vkdevice, VulkanResourceManager *vkresourcemanager) {
	this->device = vkdevice;
	this->resource_manager = vkresourcemanager;
}

void VulkanLayoutBuilder::clear() {
	binding_infos.clear();
}

RenderLayoutBuilder &VulkanLayoutBuilder::add_uniform(ShaderStage stage, UniformType type) {
	uint64_t stage_bits = 0;

	if(stage & ShaderStage::VERTEX)
		stage_bits |= VK_SHADER_STAGE_VERTEX_BIT;

	if(stage & ShaderStage::FRAGMENT)
		stage_bits |= VK_SHADER_STAGE_FRAGMENT_BIT;

	auto info = info::descriptor_set_layout_binding(
		convert::convert_uniform_type(type),
		static_cast<VkShaderStageFlagBits>(stage_bits),
		static_cast<uint32_t>(binding_infos.size())
	);

	binding_infos.push_back(info);

	return *this;
}

LayoutHandle VulkanLayoutBuilder::build() {
	auto layout_info = info::descriptor_set_layout_info(binding_infos, VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR);
	auto handle = resource_manager->layout_pool.acquire().value();
	resource_manager->try_get_layout(handle).value()->init(device, &layout_info);

	return handle;
}