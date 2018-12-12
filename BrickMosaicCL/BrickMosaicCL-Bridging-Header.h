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

void dither(void* engine);


#endif /* BrickMosaicCL_Bridging_Header_h */
