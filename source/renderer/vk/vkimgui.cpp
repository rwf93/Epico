#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_vulkan.h>

#include "vkinstance.h"
#include "vkdevice.h"
#include "vkswapchain.h"
#include "vkcommandpool.h"
#include "vkimgui.h"

VulkanImGUI::VulkanImGUI() {}
VulkanImGUI::~VulkanImGUI() {}

void VulkanImGUI::init(
	FunctorQueue<> &queue,
	AppContext *app_context,
	VulkanInstance *vkinstance ,
	VulkanDevice *vkdevice,
	VulkanSwapchain *vkswapchain,
	VulkanCommandPool *vkcommandpool
) {
	this->context = app_context;
	this->instance = vkinstance;
	this->device = vkdevice;
	this->swapchain = vkswapchain;
	this->command_pool = vkcommandpool;

	UNUSED(app_context);

	VkDescriptorPoolSize pool_sizes[] = {
		{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
	};

	VkDescriptorPoolCreateInfo pool_info = {};
	pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	pool_info.maxSets = 1000;
	pool_info.poolSizeCount = (uint32_t)std::size(pool_sizes);
	pool_info.pPoolSizes = pool_sizes;

	VK_CHECK(vkCreateDescriptorPool(device->get_device(), &pool_info, nullptr, &descriptor_pool));

	ImGui::CreateContext();
	ImGui_ImplSDL2_InitForVulkan(app_context->window);

	ImGui_ImplVulkan_InitInfo init_info = {};
	init_info.Instance = instance->get_instance();
	init_info.Device = device->get_device();
	init_info.PhysicalDevice = device->get_device().physical_device;
	init_info.Queue = device->get_graphics_queue();
	init_info.DescriptorPool = descriptor_pool;
	init_info.MinImageCount = command_pool->get_max_flying_frames();
	init_info.ImageCount = command_pool->get_max_flying_frames();
	init_info.UseDynamicRendering = true;

	init_info.PipelineRenderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
	init_info.PipelineRenderingCreateInfo.colorAttachmentCount  = 1;
	init_info.PipelineRenderingCreateInfo.pColorAttachmentFormats = &swapchain->get_swapchain_image_format();

	init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

	ImGui_ImplVulkan_LoadFunctions(
		[](const char* function_name, void* user_data) {
			return vkGetInstanceProcAddr((VkInstance)user_data, function_name);
		}, instance->get_instance().instance
	);
	ImGui_ImplVulkan_Init(&init_info);
	ImGui_ImplVulkan_CreateFontsTexture();

	queue.push([&]() { fini(); });
}

void VulkanImGUI::fini() {
	ImGui_ImplVulkan_Shutdown();
	vkDestroyDescriptorPool(device->get_device(), descriptor_pool, nullptr);
}

void VulkanImGUI::begin_ui() {
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplSDL2_NewFrame();
	ImGui::NewFrame();
}

void VulkanImGUI::end_ui() {
	ImGui::Render();

	auto color_attachment = info::attachment_info(swapchain->get_swapchain_image_view(), nullptr, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	auto rendering_info = info::rendering_info(swapchain->get_swapchain().extent, &color_attachment, nullptr);

	command_pool->transition_image(
		swapchain->get_swapchain_image(),
		VK_IMAGE_LAYOUT_GENERAL,
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
	);

	vkCmdBeginRendering(command_pool->get_command(), &rendering_info);
	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), command_pool->get_command());
	vkCmdEndRendering(command_pool->get_command());

	command_pool->transition_image(
		swapchain->get_swapchain_image(),
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		VK_IMAGE_LAYOUT_GENERAL
	);
}

void VulkanImGUI::show_demo_window() {
	ImGui::ShowDemoWindow();
}

void VulkanImGUI::process_event(SDL_Event *event) {
	ImGui_ImplSDL2_ProcessEvent(event);
}