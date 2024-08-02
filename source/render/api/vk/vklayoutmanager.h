#pragma once

#include "vklayoutbuilder.h"
#include "vklayout.h"

class VulkanInstance;
class VulkanDevice;
class VulkanCommandPool;
class VulkanLayoutManager {
public:
    void init(
        FunctorQueue<> &queue,
        VulkanInstance *vkinstance,
	    VulkanDevice *vkdevice,
	    VulkanCommandPool *vkcommandpool
    );
    void fini();

    RenderLayoutBuilder *create_layout();

	std::optional<VulkanLayout*> try_get_layout(LayoutHandle handle) {
		if(!layouts.contains(handle))
			return std::nullopt;

		auto resource = layouts.at(handle);
		if(!resource)
			return std::nullopt;

		return dynamic_cast<VulkanLayout*>(resource);
	}

private:
	VulkanInstance *instance;
	VulkanDevice *device;
	VulkanCommandPool *command_pool;

    VulkanLayoutBuilder layout_builder;

    std::map<LayoutHandle, RenderLayout*> layouts;
	LayoutHandle current_layout_handle = LayoutHandle::Invalid;
};