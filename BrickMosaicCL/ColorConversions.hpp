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

uint8_t GetGrayscaleValue(uint8_t red, uint8_t green, uint8_t blue);

double StandardToLinearAsNorm(uint8_t by);
uint8_t StandardToLinearAsByte(uint8_t by);

void RgbToXyz(uint8_t red, uint8_t green, uint8_t blue, double &x, double &y, double &z);
void XyzToLab(double x, double y, double z, int &l, int &a, int &b);
void RgbToLab(uint8_t red, uint8_t green, uint8_t blue, int &l, int &a, int &b);
void RgbToYcrcb(uint8_t red, uint8_t green, uint8_t blue, int &y, int &cr, int &cb);

void RgbToYiq(uint8_t red, uint8_t green, uint8_t blue, int &y, int &i, int &q);
void YiqToRgb(int y, int i, int q, uint8_t &red, uint8_t &green, uint8_t &blue);

void RgbToHsl(uint8_t red, uint8_t green, uint8_t blue, int &hue, int &saturation, int &luminosity);
void HslToRgb(int hue, int saturation, int luminosity, uint8_t &red, uint8_t &green, uint8_t &blue);

#endif /* ColorConversions_hpp */
