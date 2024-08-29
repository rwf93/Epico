#include <platform/platform.h>

#include <public/render/renderapi.h>

#include "rendererimpl.h"

#include <spdlog/spdlog.h>

CREATE_FACTORY(BlankRenderer);

BlankRenderer::BlankRenderer() {
	spdlog::info("Unused Renderer... What are you doing??");
}

BlankRenderer::~BlankRenderer() {
	spdlog::info("Okay bro...");
}

void BlankRenderer::begin() {

}

void BlankRenderer::end() {

}
