#pragma once

class BlankRenderer: public AbstractRenderer {
public:
    void create();
    void destroy();

    void begin();
    void end();

    void begin_pass();
    void end_pass();
};