#pragma once

class BlankRenderer: public AbstractRenderer {
public:
	BlankRenderer();
	~BlankRenderer() override;

	void begin() override;
	void end() override;

	void clear(float,float,float,float) override {}
	AbstractUI *ui() override { return nullptr; };
};