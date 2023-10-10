#pragma once

class BlankRenderer: public AbstractRenderer {
public:
    BlankRenderer();
    ~BlankRenderer() override;

    void begin() override;
    void end() override;

    void begin_pass() override;
    void end_pass() override;
};