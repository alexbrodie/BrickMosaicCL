//
//  Dither.hpp
//  BrickMosaicCL
//
//  Created by Alexander Brodie on 11/26/18.
//  Copyright © 2018 Alexander Brodie. All rights reserved.
//

#ifndef Dither_hpp
#define Dither_hpp

#include <vector>

template <typename T> class Triple;

enum class DitherType
{
    None,
    FloydSteinberg,
    JarvisJudiceNinke,
    Stuki
};

enum class DitherColorSpace
{
    RGB,
    LinearRGB,
    YIQ,
    XYZ,
    LAB,
    YCrCb
};

void Dither(
    DitherType type,
    DitherColorSpace colorSpace,
    const std::vector<Triple<uint8_t>> &rgbPalette,
    uint8_t *pixels,
    int width,
    int height,
    int stride);

#endif /* Dither_hpp */
