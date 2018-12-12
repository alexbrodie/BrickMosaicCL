//
//  Engine.cpp
//  BrickMosaicCL
//
//  Created by Alexander Brodie on 12/11/18.
//  Copyright © 2018 Alexander Brodie. All rights reserved.
//

#include "Engine.hpp"

#include "Dither.hpp"


Engine::Engine()
{
    palette = { { 0, 0, 0 }, { 64, 64, 64 }, { 128, 128, 128 }, { 196, 196, 196 }, { 255, 255, 255 }};
}

Engine::~Engine()
{
}

void Engine::Process(uint8_t* buffer, int width, int height, int stride)
{
    Dither(DitherType::FloydSteinberg,
           DitherColorSpace::LinearRGB,
           palette,
           buffer,
           width,
           height,
           stride);
}

extern "C" {
    
    void* makeEngine()
    {
        return reinterpret_cast<void*>(new Engine());
    }
    
    void freeEngine(void* engine)
    {
        delete reinterpret_cast<Engine*>(engine);
    }
    
    void process(void *engine, void* buffer, int width, int height, int stride)
    {
        reinterpret_cast<Engine*>(engine)->Process(reinterpret_cast<uint8_t*>(buffer), width, height, stride);
    }
    
}
