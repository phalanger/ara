#ifndef ARA_ENDIAN_H_202202
#define ARA_ENDIAN_H_202202

#include "ara_def.h"

//Reference to :  https://stackoverflow.com/questions/105252/how-do-i-convert-between-big-endian-and-little-endian-values-in-c

#include <cstdint>
#include <type_traits>

namespace ara {

    enum class endianness
    {
#ifdef ARA_WIN32_VER
        little = 0,
        big = 1,
        native = little
#else
        little = __ORDER_LITTLE_ENDIAN__,
        big = __ORDER_BIG_ENDIAN__,
        native = __BYTE_ORDER__
#endif
    };

    namespace internal {

        template<typename T, size_t sz>
        struct swap_bytes
        {
            inline T operator()(T val) {
                throw std::out_of_range("data size");
            }
        };

        template<typename T>
        struct swap_bytes<T, 1>
        {
            inline T operator()(T val) {
                return val;
            }
        };

        template<typename T>
        struct swap_bytes<T, 2>
        {
            inline T operator()(T val) {
                return ((((val) >> 8) & 0xff) | (((val) & 0xff) << 8));
            }
        };

        template<typename T>
        struct swap_bytes<T, 4>
        {
            inline T operator()(T val) {
                return ((((val) & 0xff000000) >> 24) |
                    (((val) & 0x00ff0000) >> 8) |
                    (((val) & 0x0000ff00) << 8) |
                    (((val) & 0x000000ff) << 24));
            }
        };

        template<>
        struct swap_bytes<float, 4>
        {
            inline float operator()(float val) {
                uint32_t mem = swap_bytes<uint32_t, sizeof(uint32_t)>()(*(uint32_t*)&val);
                return *(float*)&mem;
            }
        };

        template<typename T>
        struct swap_bytes<T, 8>
        {
            inline T operator()(T val) {
                return ((((val) & 0xff00000000000000ull) >> 56) |
                    (((val) & 0x00ff000000000000ull) >> 40) |
                    (((val) & 0x0000ff0000000000ull) >> 24) |
                    (((val) & 0x000000ff00000000ull) >> 8) |
                    (((val) & 0x00000000ff000000ull) << 8) |
                    (((val) & 0x0000000000ff0000ull) << 24) |
                    (((val) & 0x000000000000ff00ull) << 40) |
                    (((val) & 0x00000000000000ffull) << 56));
            }
        };

        template<>
        struct swap_bytes<double, 8>
        {
            inline double operator()(double val) {
                uint64_t mem = swap_bytes<uint64_t, sizeof(uint64_t)>()(*(uint64_t*)&val);
                return *(double*)&mem;
            }
        };

        template<endianness from, endianness to, class T>
        struct do_byte_swap
        {
            inline T operator()(T value) {
                return swap_bytes<T, sizeof(T)>()(value);
            }
        };
        // specialisations when attempting to swap to the same endianess
        template<class T> struct do_byte_swap<endianness::little, endianness::little, T> { inline T operator()(T value) { return value; } };
        template<class T> struct do_byte_swap<endianness::big, endianness::big, T> { inline T operator()(T value) { return value; } };

    } // namespace detail

    namespace endian {
        template<endianness from, endianness to, class T>
        inline T byte_swap(T value)
        {
            // ensure the data is only 1, 2, 4 or 8 bytes
            static_assert(sizeof(T) == 1 || sizeof(T) == 2 || sizeof(T) == 4 || sizeof(T) == 8, "Check Value Size");
            // ensure we're only swapping arithmetic types
            static_assert(std::is_arithmetic<T>::value, "Check Value Type");

            return internal::do_byte_swap<from, to, T>()(value);
        }

        template<class T>
        inline T hton(T value) {
            return byte_swap<endianness::native, endianness::big, T>(value);
        }
        template<class T>
        inline T ntoh(T value) {
            return byte_swap<endianness::big, endianness::native, T>(value);
        }
        template<class T>
        inline T htobig(T value) {
            return byte_swap<endianness::native, endianness::big, T>(value);
        }
        template<class T>
        inline T bigtoh(T value) {
            return byte_swap<endianness::big, endianness::native, T>(value);
        }
        template<class T>
        inline T htolittle(T value) {
            return byte_swap<endianness::native, endianness::little, T>(value);
        }
        template<class T>
        inline T littletoh(T value) {
            return byte_swap<endianness::little, endianness::native, T>(value);
        }
    }
}

#endif//ARA_ENDIAN_H_202202
