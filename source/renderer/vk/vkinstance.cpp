#include "vkinstance.h"

VulkanInstance::VulkanInstance() {
    vkb::InstanceBuilder builder;
    auto builder_ret = builder
                        .set_app_name("Epico")
                        .set_engine_name("Epico Engine")
                        .require_api_version(VK_API_VERSION_1_1)
                        .request_validation_layers()
                        .enable_extension(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)
                        .build();

    if(!builder_ret.has_value()) {
        spdlog::error("Failed to create Vulkan Instance: {}", builder_ret.error().message());
        std::abort();
    }

    instance = builder_ret.value();
};

VulkanInstance::~VulkanInstance() {
    vkb::destroy_instance(instance);
};
