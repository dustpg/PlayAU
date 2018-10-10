#pragma once


#define PLAYAU_API

#define PLAYAU_OBJ

#include <cstdint>

namespace PlayAU {
    // constant
    enum Constant : uint32_t {
        // bucket length in byte
        BUCKET_LENGTH = 8 * 1024,
        // bucket count
        BUCKET_COUNT = 3,
    };
    // bucket
    struct alignas(uint32_t) Bucket {
        // object
        PLAYAU_OBJ;
        // data
        uint8_t        data[BUCKET_LENGTH * BUCKET_COUNT];
    };
    // safe release interface
    template<class T>
    auto SafeRelease(T*& pointer) noexcept {
        if (pointer) {
            pointer->Release();
            pointer = nullptr;
        }
    }
    // config interface
    struct IAUConfigure {
        // pick the divice
        virtual auto PickDevice(char16_t id[256]) noexcept -> const char16_t* = 0;
    };
}