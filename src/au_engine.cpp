#include "../inc/playau.h"
#include "../inc/au_clip.h"
#include "private/p_au_engine_interface.h"
#include <cwchar>

namespace PlayAU {
#ifndef NDEBUG
    // debug check
    extern void debug_check_clip() noexcept;
#endif
    // XAudio 2.7
    auto InitInterfaceXAudio2_7(void* buf, IAUConfigure& config) noexcept->Result;
    // XAudio 2.8
    auto InitInterfaceXAudio2_8(void* buf, IAUConfigure& config) noexcept->Result;
    // default config
    struct CAUDefConfig final : IAUConfigure {
        // pick the device
        auto PickDevice(char16_t buf[256]) noexcept -> const char16_t* override {
#if 1
            return nullptr;
#else
            constexpr uint32_t ALLBUFLEN = 2048;
            constexpr uint32_t INFOLEN = 16;
            char16_t allbuf[ALLBUFLEN]; AudioDeviceInfo infos[INFOLEN];
            const auto count = PlayAU::EnumDevices(allbuf, infos, ALLBUFLEN, INFOLEN);
            if (!count) return nullptr;
            static_assert(sizeof(wchar_t) == sizeof(char16_t), "same!");
            std::wcscpy(
                reinterpret_cast<wchar_t*>(buf),
                reinterpret_cast<const wchar_t*>(infos[0].id)
            );
            return buf;
#endif
        }
    };
}

/// <summary>
/// private intercace for CAUEngine
/// </summary>
struct PlayAU::CAUEngine::Private {
    // init api
    static Result InitAPI(CAUEngine& engine, APILevel level) noexcept {
        Result hr = { Result::RE_NOINTERFACE };
        switch (level)
        {
        case PlayAU::APILevel::Level_Auto:
            // 尝试XAudio 2.8
            hr = InitInterfaceXAudio2_8(engine.m_buffer, *engine.m_pConfig);
            if (hr) break;
            // 尝试XAudio 2.7
            hr = InitInterfaceXAudio2_7(engine.m_buffer, *engine.m_pConfig);
            break;
        case PlayAU::APILevel::Level_XAudio2_7:
            hr = InitInterfaceXAudio2_7(engine.m_buffer, *engine.m_pConfig);
            break;
        case PlayAU::APILevel::Level_XAudio2_8:
            hr = InitInterfaceXAudio2_8(engine.m_buffer, *engine.m_pConfig);
            break;
        }
        return hr;
    }
};



/// <summary>
/// Initializes the specified configuration.
/// </summary>
/// <param name="config">The configuration.</param>
/// <param name="level">The level.</param>
/// <returns></returns>
auto PlayAU::CAUEngine::Initialize(
    IAUConfigure* config, 
    APILevel level) noexcept -> Result {
    // 采用默认配置
    if (!config) {
        config = new(m_defcfg) CAUDefConfig;
        static_assert(sizeof(CAUDefConfig) == sizeof(m_defcfg), "same!");
    }
    m_pConfig = config;
    // 获取
    Result hr = Private::InitAPI(*this, level);
    // 尝试利用
    return hr;
}

/// <summary>
/// Uninitializes this instance.
/// </summary>
/// <returns></returns>
void PlayAU::CAUEngine::Uninitialize() noexcept {
#ifndef NDEBUG
    // 简易内存泄漏检测
    debug_check_clip();
#endif
}

/// <summary>
/// Suspends this instance.
/// </summary>
/// <returns></returns>
void PlayAU::CAUEngine::Suspend() noexcept {
    const auto api = reinterpret_cast<IAUAudioAPI*>(m_buffer);
    api->Suspend();
}

/// <summary>
/// Resumes this instance.
/// </summary>
/// <returns></returns>
void PlayAU::CAUEngine::Resume() noexcept {
    const auto api = reinterpret_cast<IAUAudioAPI*>(m_buffer);
    api->Resume();
}


/// <summary>
/// Initializes a new instance of the <see cref="CAUEngine"/> class.
/// </summary>
PlayAU::CAUEngine::CAUEngine() noexcept {
}


/// <summary>
/// Finalizes an instance of the <see cref="CAUEngine"/> class.
/// </summary>
/// <returns></returns>
PlayAU::CAUEngine::~CAUEngine() noexcept {

}




#ifdef PLAYAU_TEST
#include <cstdio>

extern "C" void __stdcall Sleep(uint32_t);

int main() {
    PlayAU::CAUEngine engine;
    PlayAU::CoInitialize();
    //{
        char16_t buf[1024]; PlayAU::AudioDeviceInfo infos[64];
        PlayAU::EnumDevices(buf, infos, 1024, 64);
    //}
    if (engine.Initialize()) {
        const auto clip = engine.CreateClipFromFile(
            PlayAU::Flag_None, 
            u"../../audiofiledemo/Hymn_of_ussr_instrumental.ogg"
            //u"../../../Doc/Sakura.ogg"
        );

        clip->Play();
        for (int i = 0; i != 500; ++i) {
            const auto dur = clip->Tell();
            std::printf("%f\n", dur);
            ::Sleep(500);
            switch (i)
            {
            case 5:
            case 15:
                clip->Stop();
                break;
            case 9:
            case 19:
                clip->Play();
                break;
            }
        }



        std::getchar();
        clip->Pause();
        std::getchar();
        clip->Play();
        std::getchar();
        ::Sleep(1000);
        clip->Destroy();
        engine.Uninitialize();
    }
    PlayAU::CoUninitialize();
    return 0;
}
#endif