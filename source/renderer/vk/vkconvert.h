#pragma once

namespace convert {

#define CONVERTER_HEADER(name, A, B) \
    B name (A type)

// A represents the type you are converting to, B represents the type you're converting from.
// Invariant is the result of when there is no suitable conversion to A.
// Varadict arguments is the conversion table.
// Check vkconvert.cpp for an example.
#define CONVERTER(name, A, B, invariant, ...)       \
    B convert::##name (A type) {                  \
        static std::map<A, B> name##_table = {      \
            __VA_ARGS__                             \
        };                                          \
        if(name##_table##.contains(type))           \
            return name##_table##[type];            \
        return invariant;                           \
    }

CONVERTER_HEADER(convert_image_format, ImageFormat, VkFormat);
CONVERTER_HEADER(convert_sample_bits, ImageSample, VkSampleCountFlagBits);

}