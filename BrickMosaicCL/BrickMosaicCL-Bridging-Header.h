//
//  BrickMosaicCL-Bridging-Header.h
//  BrickMosaicCL
//
//  Created by Alexander Brodie on 12/11/18.
//  Copyright © 2018 Alexander Brodie. All rights reserved.
//

#ifndef BrickMosaicCL_Bridging_Header_h
#define BrickMosaicCL_Bridging_Header_h

// #import Objective-C classes and #include C files here:


void* makeEngine();
void freeEngine(void* engine);

// One of ["", "FS", "JJN", "Stuki"]
void setDitherType(void* engine, const char* type);
// One of ["RGB", "LinearRGB", "YIQ", "XYZ", "LAB", "YCrCb"]
void setDitherColorSpace(void* engine, const char* colorSpace);

// Assumes ARGB format
void process(void* engine, void* buffer, int width, int height, int stride);


#endif /* BrickMosaicCL_Bridging_Header_h */
