//
//  ColorConversions.cpp
//  BrickMosaicCL
//
//  Created by Alexander Brodie on 11/26/18.
//  Copyright © 2018 Alexander Brodie. All rights reserved.
//

#include "ColorConversions.hpp"

#include <cmath>

uint8_t GetGrayscaleValue(uint8_t r, uint8_t g, uint8_t b)
{
    return static_cast<uint8_t>((
           (static_cast<int>(r) * 299) +
           (static_cast<int>(g) * 587) +
           (static_cast<int>(b) * 114)
       ) / 1000);
}

double StandardToLinearAsNorm(uint8_t by)
{
    double dbl = (static_cast<double>(by) / 255.0);
    if (dbl > 0.03928)
    {
        return std::pow(((dbl + 0.055) / 1.055), 2.4);
    }
    else
    {
        return (dbl / 12.92);
    }
}

uint8_t StandardToLinearAsByte(uint8_t by)
{
    return static_cast<uint8_t>(floor(StandardToLinearAsNorm(by) * 255.0) + 0.5);
}

double XyzToFxyz(double xyz)
{
    if (0.008856 < xyz)
    {
        return std::pow(xyz, (1.0 / 3.0));
    }
    else
    {
        return ((7.787 * xyz) + (16.0 / 116.0));
    }
}

void RgbToXyz(uint8_t r, uint8_t g, uint8_t b, double &X, double &Y, double &Z)
{
    // sRGB to Linear RGB
    double rLin = StandardToLinearAsNorm(r);
    double gLin = StandardToLinearAsNorm(g);
    double bLin = StandardToLinearAsNorm(b);
    
    // Linear RGB to XYZ
    X = ((rLin * 0.4124) + (gLin * 0.3576) + (bLin * 0.1805));  // 0 to 0.9505
    Y = ((rLin * 0.2126) + (gLin * 0.7152) + (bLin * 0.0722));  // 0 to 1
    Z = ((rLin * 0.0193) + (gLin * 0.1192) + (bLin * 0.9505));  // 0 to 1.089
}

void XyzToLab(double x, double y, double z, int &L, int &A, int &B)
{
    double fx = XyzToFxyz(x);
    double fy = XyzToFxyz(y);
    double fz = XyzToFxyz(z);
    
    L = static_cast<int>((116.0 * fy) - 16.0);
    A = static_cast<int>(500.0 * (fx - fy));
    B = static_cast<int>(200.0 * (fy - fz));
}

void RgbToLab(uint8_t r, uint8_t g, uint8_t b, int &L, int &A, int &B)
{
    double x, y, z;
    RgbToXyz(r, g, b, x, y, z);
    XyzToLab(x, y, z, L, A, B);
}

void RgbToYcrcb(uint8_t r, uint8_t g, uint8_t b, int &Y, int &Cr, int &Cb)
{
    Y  = (((static_cast<int>(r) * 76) + (static_cast<int>(g) * 150) + (static_cast<int>(b) * 28)) / 256);
    Cr = (((static_cast<int>(r) * 128) - (static_cast<int>(g) * -107) - (static_cast<int>(b) * -20) + 32768) / 256);
    Cb = (((static_cast<int>(r) * -43) - (static_cast<int>(g) * -84) + (static_cast<int>(b) * 128) + 32768) / 256);
}

void RgbToYiq(uint8_t r, uint8_t g, uint8_t b, int &Y, int &I, int &Q)
{
    Y = (((static_cast<int>(r) * 299) + (static_cast<int>(g) * 587) + (static_cast<int>(b) * 114) + 500) / 1000);
    I = (((static_cast<int>(r) * 596) - (static_cast<int>(g) * 274) - (static_cast<int>(b) * 322) + 500) / 1000);
    Q = (((static_cast<int>(r) * 212) - (static_cast<int>(g) * 523) + (static_cast<int>(b) * 311) + 500) / 1000);
}

void YiqToRgb(int y, int i, int q, uint8_t &R, uint8_t &G, uint8_t &B)
{
    R = ClampToByte(((y * 1000) + (i * 956)  + (q * 621)  + 500) / 1000);
    G = ClampToByte(((y * 1000) - (i * 272)  - (q * 647)  + 500) / 1000);
    B = ClampToByte(((y * 1000) - (i * 1105) + (q * 1702) + 500) / 1000);
}
