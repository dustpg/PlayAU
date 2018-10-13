#pragma once
// License: MIT  http://opensource.org/licenses/MIT
// Author: dustpg   mailto:dustpg@gmail.com


#include <cstdint>


namespace PlayAU {
    /// <summary>
    /// Result code
    /// </summary>
    struct Result {
        // code
        std::int32_t    code;
        // Commonly used code list
        enum CommonResult : int32_t { 
            RS_OK           = (int32_t)0x00000000, // Operation successful
            RS_FALSE        = (int32_t)0x00000001, // Operation successful
            RE_NOTIMPL      = (int32_t)0x80004001, // Not implemented
            RE_NOINTERFACE  = (int32_t)0x80004002, // No such interface supported
            RE_POINTER      = (int32_t)0x80004003, // Pointer that is not valid
            RE_ABORT        = (int32_t)0x80004004, // Operation aborted
            RE_FAIL         = (int32_t)0x80004005, // Unspecified failure
            RE_FILENOTFOUND = (int32_t)0x80070002, // File not found
            RE_UNEXPECTED   = (int32_t)0x8000FFFF, // Unexpected failure
            RE_ACCESSDENIED = (int32_t)0x80070005, // General access denied error
            RE_HANDLE       = (int32_t)0x80070006, // Handle that is not valid
            RE_OUTOFMEMORY  = (int32_t)0x8007000E, // Failed to allocate necessary memory
            RE_INVALIDARG   = (int32_t)0x80070057, // One or more arguments are not valid
        };
        // operator bool
        operator bool() const noexcept { return code >= 0; }
        // operator !
        bool operator !() const noexcept { return code < 0; }
    };
    // API level
    enum class APILevel : uint32_t {
        // auto pick
        Level_Auto = 0,
        // XAudio ver2.7, user need install DirectX Runtime
        Level_XAudio2_7,
        // XAudio ver2.8, system component in Windows 8
        Level_XAudio2_8,
        // XAudio ver2.9, system component in Windows 10
        //Level_XAudio2_9,
        // COUNT
        COUNT
    };
    // engine used const
    enum AUConstantE : uint32_t {
        // now ver - 0x00AABBCC: AA.BB.CC
        VERSION = 0x00000400,   // 0.4.0
        // group max count
        MAX_GROUP_COUNT = 8,
        // group name max length
        MAX_GROUP_NAME_LENGTH = 16,
        // group length in byte
        GROUP_BUFLEN_BYTE = MAX_GROUP_NAME_LENGTH + 4 * sizeof(void*),
        // audio stream header peek length
        AUDIO_HEADER_PEEK_LENGTH = 16,
        // audio api buffer length in pointer
        AUDIO_API_BUFLEN = 5,
        // audio context buffer length in pointer
        AUDIO_CTX_BUFLEN = 4,
        // file stream buffer lenth in pointer
        FILE_STREAM_BUFLEN = 4,
        // audio stream buffer lenth in byte
        AUDIO_STREAM_BUFLEN = (FILE_STREAM_BUFLEN + 6) * sizeof(void*) + 4 * 4,
    };
    // wave format
    enum FormatWave : uint16_t {
        // unknown
        Wave_Unknown = 0,
        // pcm
        Wave_PCM,
        // MS-ADPCM
        Wave_MSADPCM,
        // IEEE FLOAT
        Wave_IEEEFloat,
    };
    // clip flag and formt
    enum ClipFlag : uint32_t {
        // none flag
        Flag_None = 0,
        // public flag
        Flag_Public = 0xFFFF,
        // loop forever, cannot combine with Flag_AutoDestroy
        //Flag_LoopInfinite = 1 << 0,
        // auto destroy if end of playing
        //Flag_AutoDestroy = 1 << 1,
        // load all data
        //Flag_LoadAll = 1 << 2,

        // [private] live clip
        Flag_p_Live = 1 << 16
    };
    // wave format
    struct WaveFormat {
        // samples per sec
        uint32_t    samples_per_sec;
        // bits per sample
        uint16_t    bits_per_sample;
        // channels
        uint8_t     channels;
        // fmt tag
        uint8_t     fmt_tag;
    };
}