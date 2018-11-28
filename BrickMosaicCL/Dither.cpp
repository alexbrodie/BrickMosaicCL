//
//  Dither.cpp
//  BrickMosaicCL
//
//  Created by Alexander Brodie on 11/26/18.
//  Copyright © 2018 Alexander Brodie. All rights reserved.
//

#include "Dither.hpp"

#include "ColorConversions.hpp"

#include <algorithm>
#include <cassert>
#include <tuple>
#include <vector>

using namespace std;

template <typename T> class TripleT : public tuple<T, T, T>
{
public:
    TripleT() = default;
    TripleT(T a, T b, T c);

    
};

using Triple = TripleT<int>;

//using Triple = tuple<int, int, int>;

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


Triple ConvertRgbToColorSpace(
    DitherColorSpace colorSpace,
    uint8_t red,
    uint8_t green,
    uint8_t blue)
{
    switch (colorSpace)
    {
    case DitherColorSpace::RGB:
    {
        return Triple(red, green, blue);
    }
    case DitherColorSpace::LinearRGB:
    {
        return Triple(StandardToLinearAsByte(red),
                          StandardToLinearAsByte(green),
                          StandardToLinearAsByte(blue));
    }
    case DitherColorSpace::YIQ:
    {
        int y, i, q;
        RgbToYiq(red, green, blue, y, i, q);
        return Triple(y, i, q);
    }
    case DitherColorSpace::XYZ:
    {
        double x, y, z;
        RgbToXyz(red, green, blue, x, y, z);
        return Triple(ClampToByte(x * 255.999),
                          ClampToByte(y * 255.999),
                          ClampToByte(z * 255.999));
    }
    case DitherColorSpace::LAB:
    {
        int l, a, b;
        RgbToLab(red, green, blue, l, a, b);
        return Triple(l, a, b);
    }
    case DitherColorSpace::YCrCb:
    {
        int y, cr, cb;
        RgbToYcrcb(red, green, blue, y, cr, cb);
        return Triple(y, cr, cb);
    }
    default:
        assert(!"Unexpected color space");
        return Triple(255, 0, 255);  // hopefully noticable
    }
}


bool Dither(
    DitherType type,
    DitherColorSpace colorSpace,
    uint8_t *pixels,
    int width,
    int height,
    int stride)
{
    
    // Local copy because we access this a lot
    const DitherDefinition dither = *ditherTypeToDefinition[static_cast<int>(type)];
    
    // Pre-allocate space for error at each pixel.
    vector<vector<Triple>> errors;
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
    vector<Triple> adjustedPalette;
    /* TODO!!
    palette.reserve(...);
    for (...)
    {
        palette.push_back(ConvertRgbToColorSpace(colorSpace, red, green, blue));
    }
     */
    
    // Loop over each pixel
    for (int y = 0; y < height; ++y)
    {
        // Next error row in the circular buffer
        errorRowIndex = (errorRowIndex + 1) % dither.rows;
        
        //!! Gdiplus::ARGB *pargbRow = reinterpret_cast<Gdiplus::ARGB*>(static_cast<BYTE*>(bd.Scan0) + (bd.Stride * y));
        for (int x = 0; x < width; ++x)
        {
            // Convert pixel RGB to target color space
            uint8_t r; // !!TODO
            uint8_t g; // !!TODO
            uint8_t b; // !!TODO
            Triple color = ConvertRgbToColorSpace(colorSpace, r, g, b);
            
            // Loop around area of current pixel (x, y) and compute weighted sum of errors
            // (x', y') = (x + c - C/2, y + r - R + 1)
            // for r in [0, R), and c in [0, C)
            Triple errorAccumulator{};
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
                            Triple neighborError = errors[errorRowIndex + 42][xx]; // TODO!! 42 should be some math with r
                            get<0>(errorAccumulator) += get<0>(neighborError) * weight;
                            get<1>(errorAccumulator) += get<1>(neighborError) * weight;
                            get<2>(errorAccumulator) += get<2>(neighborError) * weight;
                            errorDenominator += weight;
                        }
                    }
                }
            }
            
            // Add the weighted error average to the current pixel color
            get<0>(color) += get<0>(errorAccumulator) / errorDenominator;
            get<1>(color) += get<1>(errorAccumulator) / errorDenominator;
            get<2>(color) += get<2>(errorAccumulator) / errorDenominator;
            
            // See what value in the provided colors is closest to this value
            size_t paletteIndexWithMinError = 0;
            {
                int64_t minError = numeric_limits<int64_t>::max();
                
                for (size_t paletteIndex = 0; paletteIndex < adjustedPalette.size(); ++paletteIndex)
                {
                    // Get the error for each channel if we use this color
                    const Triple &paletteEntry = adjustedPalette[paletteIndex];
                    int64_t err0 = get<0>(color) - get<0>(paletteEntry);
                    int64_t err1 = get<1>(color) - get<1>(paletteEntry);
                    int64_t err2 = get<2>(color) - get<2>(paletteEntry);
                    
                    // Put them all together. It seems to me that we should weight these
                    // just like we do when we convert to grayscale, but empirical evidence
                    // shows that this creates haloes and other bad juju. Also it doesn't work
                    // for other color spaces.
                    int64_t error = ((err0 * err0) + (err1 * err1) + (err2 * err2));
                    if (error <= minError)
                    {
                        // This is the best we've got so far
                        minError = error;
                        paletteIndexWithMinError = paletteIndex;
                    }
                }
            }

            // This is what we've picked for this coordinate
            const Triple &paletteEntry = adjustedPalette[paletteIndexWithMinError];

            // Calculate the error of this pixel compared to desired result.
            Triple& newError = errors[errorRowIndex][x];
            get<0>(newError) = get<0>(color) - get<0>(paletteEntry);
            get<1>(newError) = get<1>(color) - get<1>(paletteEntry);
            get<2>(newError) = get<2>(color) - get<2>(paletteEntry);
            
            // And use this palette entry at this pixel
            
            
            // Finally, put this color
            pargbRow[x] =
            (pargbAvailableColors[iColorWithMinError] & ~Gdiplus::Color::AlphaMask) |
            (pargbRow[x] & Gdiplus::Color::AlphaMask);
            
            
                /*
                // Determine dither bounds around the already processed
                int top    = y - dither->rows + 1;
                int bottom = y + 1;
                int left   = x - (dither->cols / 2);
                int right  = left + dither->cols;
                
                // Need to keep track of the error denominator espcially for literal edge cases
                int errorDenominator = 0;
                
                // Clamp to bounds
                top    = max(top, 0);
                bottom = min(bottom, height);
                left   = max(left, 0);
                right  = min(right, width);

                 // Loop around area of current pixel (x, y) and compute weighted sum of errors
                 Triple
                 for (int yy = top; yy < bottom; ++yy)
                 {
                 for (int xx = left; xx < right; ++x)
                 {
                 // Add weighted error to running total
                 double
                 
                 }
                 }
*/
            
            /*
                // The (sub) bounds of dither->weights that's applicable for the current pixel (x,y)
                // such that that whole neighborhood is in bounds. This is relative to the current
                // pixel, i.e. a col=0, row=0 is at (x, y). Left and top are inclusive, bottom and right
                // are exclusive.
                int topRow = -dither->rows + 1;
                int bottomRow = 1;
                int leftColumn = 0;
                int rightColumn = dither->cols;
                
                int rowOffset;
                int columnOffset
                // TODO: constrain to bounds
              */
            
                // Loop aroun
                
                
                // The left and right cols of the neighborhood to be included here
                // !!TODO!! is this math right?!?
                int leftCol  = max(0, (dither->cols / 2) - x);
                int rightCol = min(dither->cols, ((dither->cols / 2) + (width - x)));
                
                // Loop over the full rows in the dither matrix
                for (int row = max(0, (dither->rows - y - 1)); row < dither->rows; ++row)
                {
                    const int *ditherWeightsRow = &dither->weights[row * dither->cols];
                    const Tripple *errorRow  = (errors[(errorRowIndex + 1 + row) % dither->rows] + x - (dither->cols / 2));
                    for (int col = leftCol; col < rightCol; ++col)
                    {
                        int nDither = pnDitherRow[iCol];
                        //if (nDither)
                        {
                            //nErrorDen += nDither;
                            for (int iError = 0; iError < 3; ++iError)
                            {
                                triErrorNum[iError] += (nDither * ptriErrorRow[iCol][iError]);
                            }
                        }
                    }
                }
                
                // And add that to the weight for this pixel
                //if (0 < nErrorDen)
                {
                    for (int iError = 0; iError < 3; ++iError)
                    {
                        // Watch out for large errors
                        //!!!Assert(abs(nErrorNum / nErrorDen) < (nWeightRange / 2));
                        
                        tri[iError] += (triErrorNum[iError] / nErrorDen);
                    }
                }
            }
            

        }
        
    }
}
