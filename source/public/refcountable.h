#pragma once

class RefCountable {
public:
    virtual void add_ref() = 0;
    virtual void del_ref() = 0;
    virtual uint32_t get_ref() = 0;
};