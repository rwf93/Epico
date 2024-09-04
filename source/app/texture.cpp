#include "texture.h"

void Texture::set_sampler_addressing(
    SamplerAddressMode u,
    SamplerAddressMode v,
    SamplerAddressMode w
) {
    address_u = u;
    address_v = v;
    address_w = w;
}

void Texture::set_format(ImageFormat image_format) {
    format = image_format;
}

void ImageTexture::upload_from_ktx(std::filesystem::path path) {
    ktxTexture *ktx_texture;
	ktxTexture_CreateFromNamedFile(
		filesystem->resolve_physical_dir(path).string().c_str(),
		KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT,
		&ktx_texture
	);

    api->texture(
		texture->texture,
		ImageDimensions::IMAGE_2D,
		ImageSamples::SAMPLE_COUNT_1_BIT,
		texture->format,
		ImageFlags::SAMPLED,
        ktxTexture_GetData(ktx_texture),
		ktx_texture->baseWidth, ktx_texture->baseHeight
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