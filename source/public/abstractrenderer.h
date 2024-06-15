#pragma once

class AbstractPass {};

class AbstractPassBuilder {
public:
    virtual ~AbstractPassBuilder() {};
    virtual AbstractPass *build() = 0;
};

class AbstractRenderer {
public:
    virtual ~AbstractRenderer() {};

    virtual void begin() = 0;
    virtual void end() = 0;

    virtual void begin_pass() = 0;
    virtual void end_pass() = 0;

    virtual AbstractPassBuilder *get_pass_builder() = 0;
};