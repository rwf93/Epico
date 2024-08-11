#pragma once

class Texture {
public:
	Texture(AppContext *app, Filesystem *filesystem, RenderAPI *api):
		app(app),
		filesystem(filesystem),
		api(api) {};

	void set_sampler_addressing(SamplerAddressMode u, SamplerAddressMode v, SamplerAddressMode w);
	void set_format(ImageFormat format);
	void upload_from_raw();

	void bind(std::span<UniformBind> previous_bindings);
private:
	SamplerAddressMode address_u = SamplerAddressMode::CLAMP_BORDER;
	SamplerAddressMode address_v = SamplerAddressMode::CLAMP_BORDER;
	SamplerAddressMode address_w = SamplerAddressMode::CLAMP_BORDER;
	ImageFormat format;

	AppContext *app;
	Filesystem *filesystem;
	RenderAPI *api;

	TextureHandle texture;
	TextureViewHandle view;
	SamplerHandle sampler;
};