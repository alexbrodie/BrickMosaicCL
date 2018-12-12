//
//  EngineC.hpp
//  BrickMosaicCL
//
//  Created by Alexander Brodie on 12/11/18.
//  Copyright © 2018 Alexander Brodie. All rights reserved.
//

#ifndef EngineC_hpp
#define EngineC_hpp

void* makeEngine();
void freeEngine(void* engine);

void dither(void* engine);

#endif /* EngineC_hpp */
