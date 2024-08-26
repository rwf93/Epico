#pragma once

class Texture {
public:
	Texture(Filesystem *filesystem, RenderAPI *api):
		filesystem(filesystem),
		api(api),
		texture(api->create_texture()),
		view(api->create_texture_view()),
		sampler(api->create_sampler()) {}

	void set_sampler_addressing(SamplerAddressMode u, SamplerAddressMode v, SamplerAddressMode w = SamplerAddressMode::CLAMP_BORDER);
	void set_format(ImageFormat format);
	void upload_from_ktx(std::filesystem::path path);

	TextureBind get_bind() {
		return {
			.texture_view_handle = view,
			.sampler_handle = sampler,
		};
	}

	UniformBind as_bind() {
		return {
			.texture = {
				.texture_view_handle = view,
				.sampler_handle = sampler
			},
			.type = UniformType::TEXTURE
		};
	}

	SubpassAttachment as_attachment(AttachmentType type, std::optional<SubpassAttachment::ClearValue> clear_value = std::nullopt) {
		return {
			.texture = texture,
			.view = view,
			.type = type,
			.clear = clear_value,
		};
	}

private:
	SamplerAddressMode address_u = SamplerAddressMode::CLAMP_BORDER;
	SamplerAddressMode address_v = SamplerAddressMode::CLAMP_BORDER;
	SamplerAddressMode address_w = SamplerAddressMode::CLAMP_BORDER;
	ImageFormat format 			 = ImageFormat::R8G8B8A8_UNORM;

	Filesystem *filesystem;
	RenderAPI *api;

	TextureHandle texture;
	TextureViewHandle view;
	SamplerHandle sampler;

	friend class BindableTexture;
	friend class SubpassTexture;
};

class BindableTexture {
public:
	BindableTexture(Texture *texture):
		texture(texture) {}
private:
	Texture *texture;
};
