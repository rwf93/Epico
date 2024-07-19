#pragma once

union SDL_Event;
class RenderUI {
public:
	virtual ~RenderUI() {};
	virtual void process_event(SDL_Event *event) = 0;

	virtual void begin_ui() = 0;
	virtual void end_ui() = 0;

	virtual void begin(const char *name) = 0;
	virtual void end() = 0;

	virtual void show_demo_window() = 0;
};