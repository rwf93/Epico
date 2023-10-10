#pragma once

class VulkanRenderer: public AbstractRenderer {
public:
    VulkanRenderer();
    ~VulkanRenderer() override;

    void begin() override;
    void end() override;

    void begin_pass() override;
    void end_pass() override;
};