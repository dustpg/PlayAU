#pragma once
// License: MIT  http://opensource.org/licenses/MIT
// Author: dustpg   mailto:dustpg@gmail.com


#include "au_config.h"
#include "au_base.h"

namespace PlayAU {
    // clip
    class CAUAudioClip;
    // group
    class CAUAudioGroup;
    // stream
    struct XAUStream; struct XAUAudioStream;
    // audio device
    struct AudioDeviceInfo {
        // name of device
        const char16_t*     name;
        // id of device
        const char16_t*     id;
    };
    // api
    struct PLAYAU_API API {
        // enum devices, return now count of device(maybe less than real)
        static auto EnumDevices(
            char16_t buf[],
            AudioDeviceInfo infos[],
            uint32_t buflen,
            uint32_t infolen
        ) noexcept->uint32_t;
    };
    // audio config interface
    struct IAUConfigure;
    // Audio Engine
    class PLAYAU_API CAUEngine {
    public:
        // private interface
        struct Private;
        // clip
        using Clip = CAUAudioClip * ;
        // ctor
        CAUEngine() noexcept;
        // dtor
        ~CAUEngine() noexcept;
        // get version
        auto GetVersion() const noexcept { return m_version; }
        // get api level
        auto GetAPILevel() const noexcept { return m_level; }
        // init
        auto Initialize(
            IAUConfigure* config = nullptr,
            APILevel level = APILevel::Level_Auto
        ) noexcept->Result;
        // uninit
        void Uninitialize() noexcept;
        // Suspend
        void Suspend() noexcept;
        // Resume
        void Resume() noexcept;
        // call context
        void CallContext(void* ctx1, void* ctx2) noexcept;
    public:
        // create clip from file
        Clip CreateClipFromFile(ClipFlag, const char16_t file[], const char*group=nullptr) noexcept;
        // create clip from stream
        Clip CreateClipFromStream(ClipFlag, XAUStream&, const char*group = nullptr) noexcept;
        // create clip from audio
        Clip CreateClipFromAudio(ClipFlag, XAUAudioStream&&, const char*group = nullptr) noexcept;
        // create live clip
        Clip CreateLiveClip(const WaveFormat&, const char*group = nullptr) noexcept;
    public:
        // find group
        auto FindGroup(const char name[]) noexcept->CAUAudioGroup*;
        // create empty group
        auto CreateEmptyGroup(const char name[]) noexcept->CAUAudioGroup*;
    private:
        // config
        IAUConfigure*       m_pConfig = nullptr;
        // api level
        APILevel            m_level = APILevel::Level_Auto;
        // version number
        uint32_t    const   m_version = PlayAU::VERSION;
        // audio api buffer
        uintptr_t           m_buffer[AUDIO_API_BUFLEN];
        // def-config buffer
        uintptr_t           m_defcfg[1];
        // group buffer
        uint8_t             m_group[GROUP_BUFLEN_BYTE * MAX_GROUP_COUNT];
    private:
        // no copy
        CAUEngine(const CAUEngine&) noexcept = delete;
        // no move
        CAUEngine(CAUEngine&&) noexcept = delete;
    };
}