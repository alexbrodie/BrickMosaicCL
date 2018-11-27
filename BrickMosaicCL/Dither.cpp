//
//  Dither.cpp
//  BrickMosaicCL
//
//  Created by Alexander Brodie on 11/26/18.
//  Copyright © 2018 Alexander Brodie. All rights reserved.
//

#include "Dither.hpp"

#include "ColorConversions.hpp"

#include <cassert>
#include <tuple>

using namespace std;

// Various dither types...

struct DitherDefinition
{
    int cols;
    int rows;
    int weights[15];
};

// Floyd and Steinberg
//
//         X     7/16
// 3/16   5/16   1/16
//
static const DitherDefinition DitherFloydSteinberg =
{
    3, 2,
    {
        1, 5, 3,
        7, 0, 0,
    }
};

// Jarvis, Judice and Ninke
static const DitherDefinition DitherJarvisJudiceNinke =
{
    5, 3,
    {
        1, 3, 5, 3, 1,
        3, 5, 7, 5, 3,
        5, 7, 0, 0, 0,
    }
};

// Stuki
static const DitherDefinition DitherStuki =
{
    5, 3,
    {
        1, 2, 4, 2, 1,
        2, 4, 8, 4, 2,
        4, 8, 0, 0, 0,
    }
};


// This array of dithers should be one to one with DitherType
static const DitherDefinition *ditherTypeToDefinition[] =
{
    nullptr,
    &DitherFloydSteinberg,
    &DitherJarvisJudiceNinke,
    &DitherStuki,
};

tuple<int, int, int> ConvertRgbToColorSpace(DitherColorSpace colorSpace, uint8_t red, uint8_t green, uint8_t blue)
{
    switch (colorSpace)
    {
    case DitherColorSpace::RGB:
    {
        return make_tuple(red, green, blue);
    }
    case DitherColorSpace::LinearRGB:
    {
        return make_tuple(StandardToLinearAsByte(red),
                          StandardToLinearAsByte(green),
                          StandardToLinearAsByte(blue));
    }
    case DitherColorSpace::YIQ:
    {
        int y, i, q;
        RgbToYiq(red, green, blue, y, i, q);
        return make_tuple(y, i, q);
    }
    case DitherColorSpace::XYZ:
    {
        double x, y, z;
        RgbToXyz(red, green, blue, x, y, z);
        return make_tuple(ClampToByte(x * 255.999),
                          ClampToByte(y * 255.999),
                          ClampToByte(z * 255.999));
    }
    case DitherColorSpace::LAB:
    {
        int l, a, b;
        RgbToLab(red, green, blue, l, a, b);
        return make_tuple(l, a, b);
    }
    case DitherColorSpace::YCrCb:
    {
        int y, cr, cb;
        RgbToYcrcb(red, green, blue, y, cr, cb);
        return make_tuple(y, cr, cb);
    }
    default:
        assert(!"Unexpected color space");
        return make_tuple(255, 0, 255);  // hopefully noticable
    }
}


bool Dither(
            DitherType type,
            DitherColorSpace colorSpace,
             size_t cAvailableColors,
             size_t *pcTimesUsed)
{
    if (pcTimesUsed)
    {
        for (size_t i = 0; i < cAvailableColors; ++i)
        {
            pcTimesUsed[i] = 0;
        }
    }
    
    // Get dither info
    const DitherDefinition* dither = ditherTypeToDefinition[static_cast<int>(type)];
    
    // Allocate space for error at each pixel.
    // And get the error denominator
    CAutoVectorPtr<Tripple> rgrgErrors[3];  // Max for all the SDither::cRows options
    int nErrorDen = 0;
    if (pDither)
    {
        int iDither = 0;
        
        assert(pDither->cRows <= countof(rgrgErrors));
        for (int i = 0; i < pDither->cRows; ++i)
        {
            rgrgErrors[i].Attach(new Tripple[cxBmp]);
            
            for (int j = 0; j < pDither->cCols; ++j)
            {
                nErrorDen += pDither->rgnDither[iDither++];
            }
        }
    }
    
    // Which row of rgrgnErrors specifies the error values for the row currently
    // being processed?
    int iErrorRow = 0;
    
    // Convert each ARGB to Tripple of the desired type
    CAutoVectorPtr<Tripple> rgColors(new Tripple[cAvailableColors]);
    for (size_t iColor = 0; iColor < cAvailableColors; ++iColor)
    {
        ::ConvertToColorSpace(eColorSpace, pargbAvailableColors[iColor], rgColors[iColor]);
    }
    
    // Loop over each pixel
    for (int y = 0; y < cyBmp; ++y)
    {
        Gdiplus::ARGB *pargbRow = reinterpret_cast<Gdiplus::ARGB*>(static_cast<BYTE*>(bd.Scan0) + (bd.Stride * y));
        for (int x = 0; x < cxBmp; ++x)
        {
            Tripple tri;
            ::ConvertToColorSpace(eColorSpace, pargbRow[x], tri);
            
            if (pDither)
            {
                // Error for previous pixels in neighborhood
                Tripple triErrorNum = {0};
                //int nErrorDen = 0;
                
                // The left and right cols of the neighborhood to be included here
                int nLeftCol  = max(0, ((pDither->cCols / 2) - x));
                int nRightCol = min(pDither->cCols, ((pDither->cCols / 2) + (cxBmp - x)));
                
                // Loop over the full rows in the dither matrix
                for (int iRow = max(0, (pDither->cRows - y - 1)); iRow < pDither->cRows; ++iRow)
                {
                    const int *pnDitherRow = &pDither->rgnDither[iRow * pDither->cCols];
                    const Tripple *ptriErrorRow  = (rgrgErrors[(iErrorRow + 1 + iRow) % pDither->cRows] + x - (pDither->cCols / 2));
                    for (int iCol = nLeftCol; iCol < nRightCol; ++iCol)
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
            
            // See what value in the provided colors is closest to this value
            size_t iColorWithMinError = 0;
            {
                __int64 nMinError = 0x7fffffffffffffffi64;
                
                for (size_t iColor = 0; iColor < cAvailableColors; ++iColor)
                {
                    // Get the error for each channel if we use this color
                    const Tripple &triThisColor = rgColors[iColor];
                    int64_t nErr0 = (tri[0] - triThisColor[0]);
                    __int64 nErr1 = (tri[1] - triThisColor[1]);
                    __int64 nErr2 = (tri[2] - triThisColor[2]);
                    
                    // Put them all together. It seems to me that we should weight these
                    // just like we do when we convert to grayscale, but empirical evidence
                    // shows that this creates haloes and other bad juju. Also it doesn't work
                    // for other color spaces.
                    __int64 nError = ((nErr0 * nErr0) + (nErr1 * nErr1) + (nErr2 * nErr2));
                    if (nError < nMinError)
                    {
                        // This is the best we've got so far
                        nMinError = nError;
                        iColorWithMinError = iColor;
                    }
                }
            }
            
            // Calculate the error of this pixel compared to desired result
            if (pDither)
            {
                const Tripple &triToUse = rgColors[iColorWithMinError];
                for (int iError = 0; iError < 3; ++iError)
                {
                    rgrgErrors[iErrorRow][x][iError] = (tri[iError] - triToUse[iError]);
                }
            }
            
            // Update frequency
            if (pcTimesUsed)
            {
                pcTimesUsed[iColorWithMinError]++;
            }
            
            // Finally, put this color
            pargbRow[x] =
            (pargbAvailableColors[iColorWithMinError] & ~Gdiplus::Color::AlphaMask) |
            (pargbRow[x] & Gdiplus::Color::AlphaMask);
        }
        
        if (pDither)
        {
            // Next error row
            iErrorRow = ((iErrorRow + 1) % pDither->cRows);
        }
    }
    
    return VerifyGdiSucceeded(lockbits.Unlock());
}
