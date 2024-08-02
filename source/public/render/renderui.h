#pragma once

// Wrapper for ImGUI specific API calls.

union SDL_Event;
class RenderUI {
public:
	virtual ~RenderUI() {};
	virtual void process_event(SDL_Event *event) = 0;

	virtual void begin() = 0;
	virtual void end() = 0;

	virtual void *get_context() = 0;
};