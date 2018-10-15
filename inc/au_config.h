#pragma once

#define PLAYAU_FLAG_NULL_THISPTR_SAFE

#define PLAYAU_API
//#define PLAYAU_API __declspec(dllexport) 


// PlayAU object

#if 1

#define PLAYAU_OBJ

#else
#include <new>
#include <cstdlib>

static inline void* myalloc(size_t size) noexcept {
    return std::malloc(size);
}
static inline void myfree(void* ptr) noexcept {
    std::free(ptr);
}

#define PLAYAU_OBJ \
void* operator new(size_t size, const std::nothrow_t&) noexcept {\
    return myalloc(size);\
}\
void operator delete(void* ptr) noexcept {\
    myfree(ptr);\
}

#endif

#include <cstdint>

namespace PlayAU {
    // constant
    enum Constant : uint32_t {
        // bucket length in byte
        BUCKET_LENGTH = 4 * 8 * 1024,
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
    // engine
    class CAUEngine;
    // config interface
    struct IAUConfigure {
        // pick the divice
        virtual auto PickDevice(char16_t id[256]) noexcept -> const char16_t* = 0;
        // call context, maybe async
        virtual void CallContext(CAUEngine&, void* ctx1, void* ctx2) noexcept = 0;
    };
}