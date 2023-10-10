#pragma once

class AbstractRenderer {
public:
    virtual void create() = 0;
    virtual void destroy() = 0;

    virtual void begin() = 0;
    virtual void end() = 0;

    virtual void begin_pass() = 0;
    virtual void end_pass() = 0;
};