#include "vkimgui.h"

VulkanImGUI::VulkanImGUI() {}
VulkanImGUI::~VulkanImGUI() {}

void VulkanImGUI::init(FunctorQueue<> &queue, VulkanCommandPool *vkcommandpool) {
	this->command_pool = vkcommandpool;
	queue.push([&] { fini(); });
}

void VulkanImGUI::fini() {

}


void VulkanImGUI::process_event(SDL_Event *event) {
	UNUSED(event);
}