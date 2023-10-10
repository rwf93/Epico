#pragma once

#include <platform/platform.h>
#include <memory>

class AbstractRenderer;
EAPI std::shared_ptr<AbstractRenderer> create_blank_renderer();
EAPI std::shared_ptr<AbstractRenderer> create_vulkan_renderer();