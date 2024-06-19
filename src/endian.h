#ifndef __TAO_ENDIAN_H__
#define __TAO_ENDIAN_H__

#define TAO_LITTLE_ENDIAN 1
#define TAO_BIG_ENDIAN 2

#include <byteswap.h>
#include <stdint.h>
#include <type_traits>

namespace {

/*
template metaprogramming combined with type traits and conditional compilation.
*/
template<class T>
typename std::enable_if<sizeof(T) == sizeof(uint64_t), T>::type
byteswap(T value) {
    return (T)bswap_64((uint64_t)value);
}

template<class T>
typename std::enable_if<sizeof(T) == sizeof(uint32_t), T>::type
byteswap(T value) {
    return (T)bswap_32((uint32_t)value);
}

template<class T>
typename std::enable_if<sizeof(T) == sizeof(uint16_t), T>::type
byteswap(T value) {
    return (T)bswap_16((uint16_t)value);
}

#if HYPE_ORDER == BIG_ENDIAN
#define TAO_BYTE_ORDER TAO_BIG_ENDIAN
#else
#define TAO_BYTE_ORDER TAO_LITTLE_ENDIAN
#endif

#if TAO_BYTE_ORDER == TAO_BIG_ENDIAN
template<class T>
T byteswapOnLittleEndian(T t) {
    return t;
}

template<class T>
T byteswapOnBigEndian(T t) {
    return byteswap(t);
}
#else
template<class T>
T byteswapOnLittleEndian(T t) {
    return byteswap(t);
}

template<class T>
T byteswapOnBigEndian(T t) {
    return t;
}
#endif

}

#endif