//
//  Engine.cpp
//  BrickMosaicCL
//
//  Created by Alexander Brodie on 12/11/18.
//  Copyright © 2018 Alexander Brodie. All rights reserved.
//

#include "Engine.hpp"

#include "Dither.hpp"
#include "Triple.hpp"

struct ColorInfo
{
    int id;
    const char* officialName;
    const char* commonName;
    Triple<uint8_t> rgb;
};

// Sourced from https://lego.fandom.com/wiki/Colour_Palette
constexpr ColorInfo colors[] =
{
    { 1,    "White",                    "White",                    { 0xff, 0xff, 0xff } },
    { 5,    "Brick Yellow",             "Tan",                      { 0xd9, 0xbb, 0x7b } },
    { 18,   "Nougat",                   "Flesh",                    { 0xd6, 0x72, 0x40 } },
    { 21,   "Bright Red",               "Red",                      { 0xff, 0x00, 0x00 } },
    { 23,   "Bright Blue",              "Blue",                     { 0x00, 0x00, 0xff } },
    { 24,   "Bright Yellow",            "Yellow",                   { 0xff, 0xff, 0x00 } },
    { 26,   "Black",                    "Black",                    { 0x00, 0x00, 0x00 } },
    { 28,   "Dark Green",               "Green",                    { 0x00, 0x99, 0x00 } },
    { 37,   "Bright Green",             "Bright Green",             { 0x00, 0xcc, 0x00 } },
    { 38,   "Dark Orange",              "Dark Orange",              { 0xa8, 0x3d, 0x15 } },
    /*
    { 102,  "Medium Blue",              "Medium Blue",              { 0x47, 0x8c, 0xc6 } },
    { 106,  "Bright Orange",            "Orange",                   { 0xff, 0x66, 0x00 } },
    { 107,  "Bright Bluish Green",      "Teal",                     { 0x05, 0x9d, 0x9e } },
    { 119,  "Bright Yellowish-Green",   "Lime",                     { 0x95, 0xb9, 0x0b } },
    { 124,  "Bright Reddish Violet",    "Magenta",                  { 0x99, 0x00, 0x66 } },
    { 135,  "Sand Blue",                "Sand Blue",                { xxx } },
    { 138,  "Sand Yellow",              "Dark Tan",                 { xxx } },
    { 140,  "Earth Blue",               "Dark Blue",                { xxx } },
    { 141,  "Earth Green",              "Dark Green",               { xxx } },
    { 151,  "Sand Green",               "Sand Green",               { xxx } },
    { 154,  "Dark Red",                 "Dark Red",                 { xxx } },
    { 191,  "Flame Yellowish Orange",   "Bright Light Orange",      { xxx } },
    { 192,  "Reddish Brown",            "Reddish Brown",            { xxx } },
    { 194,  "Medium Stone Grey",        "Light Grey",               { xxx } },
    { 199,  "Dark Stone Grey",          "Dark Grey",                { xxx } },
    { 208,  "Light Stone Grey",         "Very Light Grey",          { xxx } },
    { 212,  "Light Royal Blue",         "Light Blue",               { xxx } },
    { 221,  "Bright Purple",            "Bright Pink",              { xxx } },
    { 222,  "Light Purple",             "Light Pink",               { xxx } },
    { 226,  "Cool Yellow",              "Blonde",                   { xxx } },
    { 268,  "Medium Lilac",             "Dark Purple",              { xxx } },
    { 283,  "Light Nougat",             "Light Flesh",              { xxx } },
    { 308,  "Dark Brown",               "Dark Brown",               { xxx } },
    { 312,  "Medium Nougat",            "Medium Dark Flesh",        { xxx } },
    { 321,  "Dark Azure",               "Dark Azure",               { xxx } },
    { 322,  "Medium Azure",             "Azure",                    { xxx } },
    { 323,  "Aqua",                     "Unikitty Blue",            { xxx } },
    { 324,  "Medium Lavender",          "Medium Lavender",          { xxx } },
    { 325,  "Lavender",                 "Lavender",                 { xxx } },
    { 329,  "White Glow",               "Glow-in-the-dark",         { xxx } },
    { 326,  "Spring Yellowish Green",   "Unikitty Green",           { xxx } },
    { 330,  "Olive Green",              "Olive Green",              { xxx } },
    { 331,  "Medium-Yellowish green",   "Dark Lime; Medium Lime",   { xxx } },
*/
};

Engine::Engine()
{
    //palette = { { 0, 0, 0 }, { 64, 64, 64 }, { 128, 128, 128 }, { 196, 196, 196 }, { 255, 255, 255 }};
    
    for (auto&& color : colors)
    {
        palette.push_back(color.rgb);
    }
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
