#define EAPI_EXPORT
#include <factory.h>

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

std::shared_ptr<AbstractRenderer> create_blank_renderer() {
    return std::make_shared<BlankRenderer>();
}
