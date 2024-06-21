#include "vkimgui.h"

VulkanImGUI::VulkanImGUI(VulkanCommandPool *command_pool) {
	this->command_pool = command_pool;
}

VulkanImGUI::~VulkanImGUI() {

}

void VulkanImGUI::process_event(SDL_Event *event) {
	UNUSED(event);
}