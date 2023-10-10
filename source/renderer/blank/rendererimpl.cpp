#define EAPI_EXPORT
#include <factory.h>

#include <abstractrenderer.h>
#include "rendererimpl.h"

#include <spdlog/spdlog.h>

void BlankRenderer::create() {
    spdlog::info("Blank Rendering Implementation");
}

void BlankRenderer::destroy() {
    delete this;
}

void BlankRenderer::begin() {

}

void BlankRenderer::end() {

}

void BlankRenderer::begin_pass() {

}

void BlankRenderer::end_pass() {

}

AbstractRenderer *create_blank_renderer() {
    return new BlankRenderer();
}
