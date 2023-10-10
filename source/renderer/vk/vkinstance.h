#pragma once

class VulkanInstance {
public:
    VulkanInstance();
    ~VulkanInstance();

    vkb::Instance &get_instance() { return instance; }
private:
    vkb::Instance instance = {};
};