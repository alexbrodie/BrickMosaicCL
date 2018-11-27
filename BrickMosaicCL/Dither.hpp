//
//  Dither.hpp
//  BrickMosaicCL
//
//  Created by Alexander Brodie on 11/26/18.
//  Copyright © 2018 Alexander Brodie. All rights reserved.
//

#ifndef Dither_hpp
#define Dither_hpp

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

/*
bool Dither(
            EDitherType eType,
            EDitherColorSpace eColorSpace,
            size_t cAvailableColors,
            const Gdiplus::ARGB *pargbAvailableColors,
            UINT *pcTimesUsed,
            Gdiplus::Bitmap &bmp);

 */

#endif /* Dither_hpp */
