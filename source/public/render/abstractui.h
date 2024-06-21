#pragma once

union SDL_Event;
class AbstractUI {
public:
	virtual ~AbstractUI() {};
	virtual void process_event(SDL_Event *event) = 0;
};