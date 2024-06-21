#pragma once

class AbstractUI;
class AbstractRenderer {
public:
	virtual ~AbstractRenderer() {};

	virtual void begin() = 0;
	virtual void end() = 0;

	virtual void clear(float r, float g, float b, float a) = 0;

	virtual AbstractUI *ui() = 0;
};