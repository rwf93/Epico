#pragma once

class VulkanCommandPool;
class VulkanImGUI: public AbstractUI {
public:
	VulkanImGUI();
	~VulkanImGUI() override;

	void process_event(SDL_Event *event);

	void init(FunctorQueue<> &queue, VulkanCommandPool *vkcommandpool);
	void fini();
private:
	VulkanCommandPool *command_pool = nullptr;
};