#pragma once
// License: MIT  http://opensource.org/licenses/MIT
// Author: dustpg   mailto:dustpg@gmail.com


#include "au_config.h"
#include "au_base.h"

namespace PlayAU {
    // CoInitialize
    auto CoInitialize() noexcept->Result;
    // CoUninitialize
    void CoUninitialize() noexcept;
    // clip
    class CAUAudioClip;
    // stream
    struct IAUStream; struct XAUAudioStream;
    // audio device
    struct AudioDeviceInfo {
        // name of device
        const char16_t*     name;
        // id of device
        const char16_t*     id;
    };
    // enum devices, return now count of device(maybe less than real)
    auto EnumDevices(
        char16_t buf[], 
        AudioDeviceInfo infos[], 
        uint32_t buflen, 
        uint32_t infolen
    ) noexcept ->uint32_t;
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
    public:
        // create clip from file
        Clip CreateClipFromFile(ClipFlag, const char16_t file[]) noexcept;
        // create clip from stream
        Clip CreateClipFromStream(ClipFlag, IAUStream&) noexcept;
        // create clip from audio
        Clip CreateClipFromAudio(ClipFlag, XAUAudioStream&&) noexcept;
        // create clip from memory

        // create wave from memory

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
    private:
        // no copy
        CAUEngine(const CAUEngine&) noexcept = delete;
        // no move
        CAUEngine(CAUEngine&&) noexcept = delete;
    };
}