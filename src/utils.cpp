/*
 * byteswap.cpp
 *
 *  Created on: 2 Mar 2018
 *      Author: P.H. van Heumen
 */

#include <application.h>
#include "utils.h"

template<>
uint16_t byteswap<uint16_t>(uint16_t x)
{
    return uint16_t (x >> 8)|(x << 8);
}

template<>
uint32_t byteswap<uint32_t>(uint32_t x)
{
    return uint32_t(byteswap<uint16_t>(x) << 16) | byteswap<uint16_t>(x >> 16);
}

template<>
int16_t byteswap<int16_t>(int16_t x)
{
    return int16_t ((uint16_t)x >> 8)|((uint16_t)x << 8);
}

template<>
int32_t byteswap<int32_t>(int32_t x)
{
    return int32_t ((uint16_t)byteswap<int16_t>(x) << 16) | (uint16_t)byteswap<int16_t>(x >> 16);
}



