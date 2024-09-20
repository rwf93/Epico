#pragma once

class Texture {
public:
	Texture(RenderAPI *api):
		api(api),
		texture(api->create_texture()),
		view(api->create_texture_view()),
		sampler(api->create_sampler()) {}

	void set_sampler_addressing(SamplerAddressMode u, SamplerAddressMode v, SamplerAddressMode w = SamplerAddressMode::CLAMP_BORDER);
	void set_format(ImageFormat format);

	operator UniformBind() {
		return {
			.texture = {
				.view = view,
				.sampler = sampler
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

	RenderAPI *api;

	TextureHandle texture;
	TextureViewHandle view;
	SamplerHandle sampler;

	friend class RenderTexture;
	friend class ImageTexture;
};
/*
class RenderTexture {
	RenderTexture(Texture *texture, float width, float height, ImageFlags flags)
		: texture(texture)
		, api(texture->api) {
			 api->texture(
				texture->texture,
				ImageDimensions::IMAGE_2D,
				ImageSamples::SAMPLE_COUNT_1_BIT,
				texture->format,
				flags,
        		nullptr,
				width, height
			);

			api->texture_view(
				texture->view,
				texture->texture,
				ImageViewDimensions::IMAGE_2D,
				texture->format, 0, 0
			);

			api->sampler(
				texture->sampler,
				texture->address_u,
				texture->address_v,
				texture->address_w
			);
		}

private:
	Texture *texture;
	RenderAPI *api;
};
*/
class ImageTexture {
public:
	ImageTexture(Texture *texture, Filesystem* filesystem)
		: texture(texture)
		, api(texture->api)
		, filesystem(filesystem) {}

	void upload_from_ktx(std::filesystem::path path);
private:
	Texture *texture;
	RenderAPI *api;
	Filesystem *filesystem;
};