#pragma once

union SDL_Event;
class AbstractUI {
public:
	virtual ~AbstractUI() {};
	virtual void process_event(SDL_Event *event) = 0;

	virtual void begin_ui() = 0;
	virtual void end_ui() = 0;

	virtual void show_demo_window() = 0;
};