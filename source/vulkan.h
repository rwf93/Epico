#pragma once

#define FUNC_CREATE_VK_INSTANCE gameGlobals *game
#define FUNC_CREATE_SURFACE gameGlobals *game
#define FUNC_CREATE_DEVICE gameGlobals *game
#define FUNC_CREATE_SWAPCHAIN gameGlobals *game
#define FUNC_GET_QUEUES gameGlobals *game
#define FUNC_CREATE_RENDER_PASS gameGlobals *game
#define FUNC_CREATE_SHADER gameGlobals *game, const std::vector<char> &code
#define FUNC_CREATE_GRAPHICS_PIPELINE gameGlobals *game

bool create_vk_instance(FUNC_CREATE_VK_INSTANCE);
bool create_surface(FUNC_CREATE_SURFACE);
bool create_device(FUNC_CREATE_DEVICE);
bool create_swapchain(FUNC_CREATE_SWAPCHAIN);
bool get_queues(FUNC_GET_QUEUES);
bool create_render_pass(FUNC_CREATE_RENDER_PASS);
VkShaderModule create_shader(FUNC_CREATE_SHADER);
bool create_graphics_pipeline(FUNC_CREATE_GRAPHICS_PIPELINE);