//
//  ColorConversions.hpp
//  BrickMosaicCL
//
//  Created by Alexander Brodie on 11/26/18.
//  Copyright © 2018 Alexander Brodie. All rights reserved.
//

#ifndef ColorConversions_hpp
#define ColorConversions_hpp

#include <stdint.h>


template <typename T> inline uint8_t ClampToByte(T n)
{
    return static_cast<uint8_t>(n > 255 ? 255 : (n < 0 ? 0 : n));
}


double StandardToLinearAsNorm(uint8_t by);
uint8_t StandardToLinearAsByte(uint8_t by);

void RgbToLab(uint8_t r, uint8_t g, uint8_t b, int &L, int &A, int &B);
void RgbToYcrcb(uint8_t r, uint8_t g, uint8_t b, int &Y, int &Cr, int &Cb);

void RgbToYiq(uint8_t r, uint8_t g, uint8_t b, int &y, int &i, int &q);
void YiqToRgb(int y, int i, int q, uint8_t &r, uint8_t &g, uint8_t &b);

void RgbToHsl(uint8_t r, uint8_t g, uint8_t b, int &h, int &s, int &l);

#endif /* ColorConversions_hpp */
