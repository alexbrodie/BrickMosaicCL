//
//  Dither.cpp
//  BrickMosaicCL
//
//  Created by Alexander Brodie on 11/26/18.
//  Copyright © 2018 Alexander Brodie. All rights reserved.
//

#include "Dither.hpp"

#include "ColorConversions.hpp"
#include "Triple.hpp"

#include <algorithm>
#include <cassert>
#include <vector>

using namespace std;

// Various dither types...

struct DitherDefinition
{
    int denominator; // sum of weights
    int cols;
    int rows;
    int weights[15];
};

static const DitherDefinition DitherNone =
{
    0, 1, 1,
    {
        0,
    }
};

// Floyd and Steinberg
//
//         X     7/16
// 3/16   5/16   1/16
//
static const DitherDefinition DitherFloydSteinberg =
{
    16, 3, 2,
    {
        1, 5, 3,
        7, 0, 0,
    }
};

// Jarvis, Judice and Ninke
static const DitherDefinition DitherJarvisJudiceNinke =
{
    48, 5, 3,
    {
        1, 3, 5, 3, 1,
        3, 5, 7, 5, 3,
        5, 7, 0, 0, 0,
    }
};

// Stuki
static const DitherDefinition DitherStuki =
{
    42, 5, 3,
    {
        1, 2, 4, 2, 1,
        2, 4, 8, 4, 2,
        4, 8, 0, 0, 0,
    }
};



// This array of dithers should be one to one with DitherType
static const DitherDefinition *ditherTypeToDefinition[] =
{
    &DitherNone,
    &DitherFloydSteinberg,
    &DitherJarvisJudiceNinke,
    &DitherStuki,
};


Triple<int> ConvertRgbToColorSpace(
    DitherColorSpace colorSpace,
    uint8_t red,
    uint8_t green,
    uint8_t blue)
{
    switch (colorSpace)
    {
    case DitherColorSpace::RGB:
    {
        return Triple<int>(red, green, blue);
    }
    case DitherColorSpace::LinearRGB:
    {
        return Triple<int>(StandardToLinearAsByte(red),
                            StandardToLinearAsByte(green),
                            StandardToLinearAsByte(blue));
    }
    case DitherColorSpace::YIQ:
    {
        int y, i, q;
        RgbToYiq(red, green, blue, y, i, q);
        return Triple<int>(y, i, q);
    }
    case DitherColorSpace::XYZ:
    {
        double x, y, z;
        RgbToXyz(red, green, blue, x, y, z);
        return Triple<int>(ClampToByte(x * 255.999),
                            ClampToByte(y * 255.999),
                            ClampToByte(z * 255.999));
    }
    case DitherColorSpace::LAB:
    {
        int l, a, b;
        RgbToLab(red, green, blue, l, a, b);
        return Triple<int>(l, a, b);
    }
    case DitherColorSpace::YCrCb:
    {
        int y, cr, cb;
        RgbToYcrcb(red, green, blue, y, cr, cb);
        return Triple<int>(y, cr, cb);
    }
    default:
        assert(!"Unexpected color space");
        return Triple<int>(255, 0, 255);  // hopefully noticable
    }
}


void Dither(
    DitherType type,
    DitherColorSpace colorSpace,
    const vector<Triple<uint8_t>> &rgbPalette,
    uint8_t *pixels,
    int width,
    int height,
    int stride)
{
    // Local copy because we access this a lot
    const DitherDefinition dither = *ditherTypeToDefinition[static_cast<int>(type)];
    
    // Pre-allocate space for error at each pixel.
    vector<vector<Triple<int>>> errors;
    for (int r = 0; r < dither.rows; ++r)
    {
        errors.emplace_back(width);
    }
    
    // Which row of the 2d array [errors] specifies the error values for the row currently
    // being processed? This will be a circular buffer at the highest vector level. This
    // represents the current error row which is being written to, and increases by one
    // each row so that errorRowIndex - 1 is the errors from the previous pass, and -2 is
    // the one before that, etc.
    int errorRowIndex = 0;
    
    // Convert the acceptable colors into a color space modified palette
    vector<Triple<int>> adjustedPalette;
    adjustedPalette.reserve(rgbPalette.size());
    for (auto &&it : rgbPalette)
    {
        adjustedPalette.push_back(ConvertRgbToColorSpace(colorSpace, get<0>(it), get<1>(it), get<2>(it)));
    }
    
    // Loop over each pixel
    for (int y = 0; y < height; ++y)
    {
        // Next error row in the circular buffer
        errorRowIndex = (errorRowIndex + 1) % dither.rows;
        
        //!! Gdiplus::ARGB *pargbRow = reinterpret_cast<Gdiplus::ARGB*>(static_cast<BYTE*>(bd.Scan0) + (bd.Stride * y));
        for (int x = 0; x < width; ++x)
        {
            // Pixels are ARGB format
            int pixelBufferIndex = y * stride + x * 4;
            //uint8_t& aa = pixels[pixelBufferIndex + 0];
            uint8_t& rr = pixels[pixelBufferIndex + 1];
            uint8_t& gg = pixels[pixelBufferIndex + 2];
            uint8_t& bb = pixels[pixelBufferIndex + 3];

            // Convert pixel RGB to target color space
            Triple<int> color = ConvertRgbToColorSpace(colorSpace, rr, gg, bb);
            
            // Loop around area of current pixel (x, y) and compute weighted sum of errors
            // (x', y') = (x + c - C/2, y + r - R + 1)
            // for r in [0, R), and c in [0, C)
            Triple<int> errorAccumulator{};
            int errorDenominator = 0;
            for (int r = 0; r < dither.rows; ++r)
            {
                int yy = y + r - dither.rows + 1;
                if (0 <= yy)
                {
                    for (int c = 0; c < dither.cols; ++c)
                    {
                        int xx = x + c - (dither.cols / 2);
                        if (0 <= xx && xx < width)
                        {
                            // Add weighted error for pixel (x', y') ro running total
                            int weight = dither.weights[c + r * dither.cols];

                            // errors[errorRowIndex][x] is meant for the current outer
                            // pixel, i.e. when r == R - 1. And the oldest error (r == 0)
                            // is the next error row: errors[errorRowIndex + 1].
                            errorAccumulator += errors[(errorRowIndex + r + 1) % dither.rows][xx] * weight;
                            errorDenominator += weight;
                        }
                    }
                }
            }
            
            // Add the weighted error average to the current pixel color
            if (errorDenominator > 0)
            {
                color += errorAccumulator / errorDenominator;
            }
            else
            {
                // Isn't this only true for the first pixel!?
                assert(x == 0 && y == 0);
            }
            
            // See what value in the provided colors is closest to this value
            size_t paletteIndexWithMinError = 0;
            {
                int64_t minError = numeric_limits<int64_t>::max();
                
                for (size_t paletteIndex = 0; paletteIndex < adjustedPalette.size(); ++paletteIndex)
                {
                    // Get the error for each channel if we use this color
                    Triple<int> delta = color - adjustedPalette[paletteIndex];
                    int64_t e0 = get<0>(delta), e1 = get<1>(delta), e2 = get<2>(delta);

                    // Put them all together. It seems to me that we should weight these
                    // just like we do when we convert to grayscale, but empirical evidence
                    // shows that this creates haloes and other bad juju. Also it doesn't work
                    // for other color spaces.
                    int64_t error = (e0 * e0) + (e1 * e1) + (e2 * e2);
                    if (error <= minError)
                    {
                        // This is the best we've got so far
                        minError = error;
                        paletteIndexWithMinError = paletteIndex;
                    }
                }
            }

            // This is what we've picked for this coordinate
            const Triple<int> &paletteEntry = adjustedPalette[paletteIndexWithMinError];

            // Calculate the error of this pixel compared to desired result.
            errors[errorRowIndex][x] = color - paletteEntry;
            
            // And use this palette entry at this pixel
            const Triple<uint8_t>& resultColor = rgbPalette[paletteIndexWithMinError];
            rr = get<0>(resultColor);
            gg = get<1>(resultColor);
            bb = get<2>(resultColor);
        }
    }
}

#if 0
static struct Test
{
    Test()
    {
        vector<Triple<uint8_t>> colors
        {
            { 255, 0, 0 },
            { 0, 255, 0 },
            { 0, 0, 255 },
        };
        
        Dither(DitherType::Stuki,
               DitherColorSpace::LinearRGB,
               colors,
               nullptr,
               0,
               0,
               0);
    }
} test;
#endif
