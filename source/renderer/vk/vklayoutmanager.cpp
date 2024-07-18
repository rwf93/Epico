#include "vklayoutmanager.h"

void VulkanLayoutManager::init(
    FunctorQueue<> &queue,
    VulkanInstance *vkinstance,
    VulkanDevice *vkdevice,
    VulkanCommandPool *vkcommandpool
) {
    this->instance = vkinstance;
    this->device = vkdevice;
    this->command_pool = vkcommandpool;

    layout_builder.init(device);

    queue.push([&] { fini(); });
}

void VulkanLayoutManager::fini() {
    for(auto &layout: layouts) {
		if(auto second = layout.second) {
			if(second->get_state() == LayoutState::LAYOUT_READY)
				second->fini();
			delete second;
		}
	}

    layout_builder.fini();
}

AbstractLayoutBuilder *VulkanLayoutManager::create_layout() {
    auto last_layout = layouts.size() + 1;

    auto layout = new VulkanLayout();
    layouts.insert(std::make_pair(last_layout, layout));
    layout_builder.clear(last_layout, layout);

    return &layout_builder;
}