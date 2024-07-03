#pragma once

class VulkanDevice;
class VulkanGraphicShader: public AbstractGraphicShader {
public:
	VulkanGraphicShader(ShaderHandle shader_handle, VulkanDevice *vkdevice);
	~VulkanGraphicShader() override;

	ShaderState get_state() override { return state; }

	ShaderHandle init() override;
	void fini() override;

	AbstractGraphicShader *add_attribute(
		uint32_t location,
		uint32_t binding,
		uint32_t offset,
		AttributeType type
	) override;

private:
	VulkanDevice *device;
	ShaderHandle handle;
	ShaderState state = ShaderState::SHADER_UNREADY;

	std::vector<VkVertexInputAttributeDescription> attributes;
};