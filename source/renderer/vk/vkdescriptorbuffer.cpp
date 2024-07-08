#include "vkdevice.h"
#include "vkbuffer.h"
#include "vkdescriptorbuffer.h"

VulkanUniformBuffer::~VulkanUniformBuffer() {

}

void VulkanUniformBuffer::init() {
    state = UniformState::UNIFORM_READY;
}

void VulkanUniformBuffer::fini() {
    state = UniformState::UNIFORM_UNREADY;
}