#pragma once

class VulkanCommandPool;
class VulkanImGUI: public AbstractUI {
public:
	VulkanImGUI(VulkanCommandPool *command_pool);
	~VulkanImGUI() override;

	void process_event(SDL_Event *event);
private:
	VulkanCommandPool *command_pool = nullptr;
};