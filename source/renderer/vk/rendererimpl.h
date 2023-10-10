#pragma once

class VulkanRenderer: public AbstractRenderer {
public:
    void create();
    void destroy();

    void begin();
    void end();

    void begin_pass();
    void end_pass();
};