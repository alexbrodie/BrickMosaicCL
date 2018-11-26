//
//  ColorConversions.cpp
//  BrickMosaicCL
//
//  Created by Alexander Brodie on 11/26/18.
//  Copyright © 2018 Alexander Brodie. All rights reserved.
//

#include "ColorConversions.hpp"

#include <algorithm>
#include <cmath>

using namespace std;

uint8_t GetGrayscaleValue(uint8_t red, uint8_t green, uint8_t blue)
{
    return static_cast<uint8_t>((
           (static_cast<int>(red)   * 299) +
           (static_cast<int>(green) * 587) +
           (static_cast<int>(blue)  * 114)
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

void RgbToXyz(uint8_t red, uint8_t green, uint8_t blue, double &x, double &y, double &z)
{
    // sRGB to Linear RGB
    double rLin = StandardToLinearAsNorm(red);
    double gLin = StandardToLinearAsNorm(green);
    double bLin = StandardToLinearAsNorm(blue);
    
    // Linear RGB to XYZ
    x = ((rLin * 0.4124) + (gLin * 0.3576) + (bLin * 0.1805));  // 0 to 0.9505
    y = ((rLin * 0.2126) + (gLin * 0.7152) + (bLin * 0.0722));  // 0 to 1
    z = ((rLin * 0.0193) + (gLin * 0.1192) + (bLin * 0.9505));  // 0 to 1.089
}

void XyzToLab(double x, double y, double z, int &l, int &a, int &b)
{
    double fx = XyzToFxyz(x);
    double fy = XyzToFxyz(y);
    double fz = XyzToFxyz(z);
    
    l = static_cast<int>((116.0 * fy) - 16.0);
    a = static_cast<int>(500.0 * (fx - fy));
    b = static_cast<int>(200.0 * (fy - fz));
}

void RgbToLab(uint8_t red, uint8_t green, uint8_t blue, int &l, int &a, int &b)
{
    double x, y, z;
    RgbToXyz(red, green, blue, x, y, z);
    XyzToLab(x, y, z, l, a, b);
}

void RgbToYcrcb(uint8_t red, uint8_t green, uint8_t blue, int &y, int &cr, int &cb)
{
    y  = (((static_cast<int>(red) *  76) + (static_cast<int>(green) *  150) + (static_cast<int>(blue) *  28)) / 256);
    cr = (((static_cast<int>(red) * 128) - (static_cast<int>(green) * -107) - (static_cast<int>(blue) * -20) + 32768) / 256);
    cb = (((static_cast<int>(red) * -43) - (static_cast<int>(green) *  -84) + (static_cast<int>(blue) * 128) + 32768) / 256);
}

void RgbToYiq(uint8_t red, uint8_t green, uint8_t blue, int &y, int &i, int &q)
{
    y = (((static_cast<int>(red) * 299) + (static_cast<int>(green) * 587) + (static_cast<int>(blue) * 114) + 500) / 1000);
    i = (((static_cast<int>(red) * 596) - (static_cast<int>(green) * 274) - (static_cast<int>(blue) * 322) + 500) / 1000);
    q = (((static_cast<int>(red) * 212) - (static_cast<int>(green) * 523) + (static_cast<int>(blue) * 311) + 500) / 1000);
}

void YiqToRgb(int y, int i, int q, uint8_t &red, uint8_t &green, uint8_t &blue)
{
    red   = ClampToByte(((y * 1000) + (i * 956)  + (q * 621)  + 500) / 1000);
    green = ClampToByte(((y * 1000) - (i * 272)  - (q * 647)  + 500) / 1000);
    blue  = ClampToByte(((y * 1000) - (i * 1105) + (q * 1702) + 500) / 1000);
}

void RgbToHsl(uint8_t red, uint8_t green, uint8_t blue, int &hue, int &saturation, int &luminosity)
{
    int nMin = min(red, min(green, blue));
    int nMax = max(red, max(green, blue));
    
    if (nMin == nMax)
    {
        // Grayscale (no saturation)
        hue = 0;
        saturation = 0;
        luminosity = nMin;
    }
    else
    {
        // There's some color
        int h1, h2;
        if (red == nMax)
        {
            h1 = 0;
            h2 = green - blue;
        }
        else if (green == nMax)
        {
            h1 = 0x55;
            h2 = blue - red;
        }
        else
        {
            h1 = 0xaa;
            h2 = red - green;
        }
        
        h1 += (0x2b * h2) / (nMax - nMin);
        if (h1 < 0)
        {
            h1 += 0xff;
        }

        hue = h1;
        luminosity = (nMin + nMax) / 2;
        saturation = ((nMax - nMin) * 0xff) / ((luminosity & 0x80) ? (0x1fe - nMax - nMin) : (nMax + nMin));
    }
}

uint8_t HueToColor(int m1, int m2, int h)
{
    if (h < 0)
    {
        h += 0xff;
    }
    else if (0xff < h)
    {
        h -= 0xff;
    }
    
    int nRet;
    if ((2 * h) < 0x100)
    {
        if ((6 * h) < 0x100)
        {
            nRet = m1 + ((m2 - m1) * h) / 0x2b;
        }
        else
        {
            nRet = m2;
        }
    }
    else
    {
        if ((3 * h) < 0x1ff)
        {
            nRet = m1 + ((m2 - m1) * (0xaa - h)) / 0x2b;
        }
        else
        {
            nRet = m1;
        }
    }
    
    return ClampToByte(nRet);
}

void HslToRgb(int hue, int saturation, int luminosity, uint8_t &red, uint8_t &green, uint8_t &blue)
{
    int m2 = ((luminosity & 0x80) ? (luminosity + saturation - ((luminosity * saturation) / 0xff)) : ((luminosity * (0xff + saturation)) / 0xff));
    int m1 = (2 * luminosity) - m2;
    
    red   = HueToColor(m1, m2, hue + 0x55);
    green = HueToColor(m1, m2, hue);
    blue  = HueToColor(m1, m2, hue - 0x55);
}
