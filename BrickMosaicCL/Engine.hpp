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

template <typename T> class Triple;


class Engine
{
public:
    
    Engine();
    virtual ~Engine();
    
    void Process(uint8_t* buffer, int width, int height, int stride);
    
private:
    
    std::vector<Triple<uint8_t>> palette;
    
};

#endif /* Engine_hpp */
