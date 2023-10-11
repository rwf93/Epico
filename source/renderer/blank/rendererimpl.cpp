#define EAPI_EXPORT
#include <platform/platform.h>

#include <abstractrenderer.h>
#include "rendererimpl.h"

#include <spdlog/spdlog.h>

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

void BlankRenderer::begin_pass() {

}

void BlankRenderer::end_pass() {

}

extern "C" EAPI AbstractRenderer *create_factory() {
    return new BlankRenderer();
}