#include "vkgraphicshader.h"
#include "vkcomputeshader.h"
#include "vkshadermanager.h"

VulkanShaderManager::VulkanShaderManager() {}
VulkanShaderManager::~VulkanShaderManager() {}

void VulkanShaderManager::init(FunctorQueue<> &queue) {
    queue.push([&]() { fini(); });
}

void VulkanShaderManager::fini() {

}

ShaderHandle VulkanShaderManager::create_graphic_shader() {
    ShaderHandle last_shader_handle = advance_shader_handle();
    return last_shader_handle;
}

ShaderHandle VulkanShaderManager::create_compute_shader() {
    ShaderHandle last_shader_handle = advance_shader_handle();
    return last_shader_handle;
}