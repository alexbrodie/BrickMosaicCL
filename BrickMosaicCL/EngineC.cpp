//
//  EngineC.cpp
//  BrickMosaicCL
//
//  Created by Alexander Brodie on 12/11/18.
//  Copyright © 2018 Alexander Brodie. All rights reserved.
//

#include "Engine.hpp"

extern "C" {

    void* makeEngine()
    {
        return reinterpret_cast<void*>(new Engine());
    }

    void freeEngine(void* engine)
    {
        delete reinterpret_cast<Engine*>(engine);
    }
    
    void dither(void* engine)
    {
        reinterpret_cast<Engine*>(engine)->Dither();
    }

}
