//
//  Triple.hpp
//  BrickMosaicCL
//
//  Created by Alexander Brodie on 12/11/18.
//  Copyright © 2018 Alexander Brodie. All rights reserved.
//

#ifndef Triple_hpp
#define Triple_hpp

#include <tuple>

template <typename T> class Triple : public std::tuple<T, T, T>
{
public:
    constexpr Triple() = default;
    constexpr Triple(T a, T b, T c);
    
    Triple& operator+=(const Triple& other);
    //Triple& operator-=(const Triple& other);
    //Triple& operator*=(const Triple& other);
    //Triple& operator/=(const Triple& other);
    
    //Triple operator+(const Triple& other) const;
    Triple operator-(const Triple& other) const;
    //Triple operator*(const Triple& other) const;
    //Triple operator/(const Triple& other) const;
    
    template <typename U> Triple operator+(U other) const;
    template <typename U> Triple operator-(U other) const;
    template <typename U> Triple operator*(U other) const;
    template <typename U> Triple operator/(U other) const;
    
};

template <typename T> constexpr Triple<T>::Triple(T a, T b, T c) : std::tuple<T, T, T>(a, b, c)
{
}

template <typename T> Triple<T>& Triple<T>::operator+=(const Triple &other)
{
    std::get<0>(*this) += std::get<0>(other);
    std::get<1>(*this) += std::get<1>(other);
    std::get<2>(*this) += std::get<2>(other);
    return *this;
}

template <typename T> Triple<T> Triple<T>::operator-(const Triple& other) const
{
    return Triple(std::get<0>(*this) - std::get<0>(other),
                  std::get<1>(*this) - std::get<1>(other),
                  std::get<2>(*this) - std::get<2>(other));
}

template <typename T> template <typename U> Triple<T> Triple<T>::operator+(U other) const
{
    return Triple(std::get<0>(*this) + other,
                  std::get<1>(*this) + other,
                  std::get<1>(*this) + other);
}

template <typename T> template <typename U> Triple<T> Triple<T>::operator-(U other) const
{
    return Triple(std::get<0>(*this) - other,
                  std::get<1>(*this) - other,
                  std::get<1>(*this) - other);
}

template <typename T> template <typename U> Triple<T> Triple<T>::operator*(U other) const
{
    return Triple(std::get<0>(*this) * other,
                  std::get<1>(*this) * other,
                  std::get<1>(*this) * other);
}

template <typename T> template <typename U> Triple<T> Triple<T>::operator/(U other) const
{
    return Triple(std::get<0>(*this) / other,
                  std::get<1>(*this) / other,
                  std::get<1>(*this) / other);
}

#endif /* Triple_hpp */
