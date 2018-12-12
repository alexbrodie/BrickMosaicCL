//
//  Engine.hpp
//  BrickMosaicCL
//
//  Created by Alexander Brodie on 12/11/18.
//  Copyright © 2018 Alexander Brodie. All rights reserved.
//

#ifndef Engine_hpp
#define Engine_hpp

#include <vector>

enum class DitherType;
enum class DitherColorSpace;
template <typename T> class Triple;


class Engine
{
public:
    
    Engine();
    virtual ~Engine();
    
    void setDitherType(const char* type);
    void setDitherColorSpace(const char* colorSpace);

    void Process(uint8_t* buffer, int width, int height, int stride);
    
private:
    
    DitherType _ditherType;
    DitherColorSpace _ditherColorSpace;
    std::vector<Triple<uint8_t>> _palette;
    
};

#endif /* Engine_hpp */
