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

    AbstractLayoutBuilder *create_layout();

    VulkanLayout *get_layout(LayoutHandle handle) {
        if(layouts.contains(handle))
            return dynamic_cast<VulkanLayout*>(layouts.at(handle));
        return nullptr;
    }

protected:
    LayoutHandle advance_handle() {
		LayoutHandle last_layout_handle = current_layout_handle;
		current_layout_handle++;
		return last_layout_handle;
	}
private:
	VulkanInstance *instance;
	VulkanDevice *device;
	VulkanCommandPool *command_pool;

    VulkanLayoutBuilder layout_builder;

    std::map<LayoutHandle, AbstractLayout*> layouts;
	LayoutHandle current_layout_handle = 0;
};