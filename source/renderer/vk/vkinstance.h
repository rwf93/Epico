#pragma once

class VulkanInstance {
public:
    VulkanInstance();
    ~VulkanInstance();

    void init(FunctorQueue<> &queue);
    void fini();

    vkb::Instance &get_instance() { return instance; }
private:
    vkb::Instance instance = {};
};