#include "vkdevice.h"
#include "vksampler.h"

void VulkanSampler::init(VkSamplerCreateInfo *create_info) {
    VK_CHECK(vkCreateSampler(device->get_device(), create_info, nullptr, &sampler));
    state = ResourceState::READY;
}

void VulkanSampler::fini() {
    vkDestroySampler(device->get_device(), sampler, nullptr);
    state = ResourceState::UNREADY;
}