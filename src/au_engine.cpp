#include "../inc/playau.h"
#include "../inc/au_clip.h"
#include "../inc/au_group.h"
#include "private/p_au_engine_interface.h"

#include <cwchar>
#include <cstring>

namespace PlayAU {
#ifndef NDEBUG
    // debug check
    extern void debug_check_clip() noexcept;
#endif
    // dispose groups
    void DisposeGroups(CAUEngine&) noexcept;
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
        // call context
        void CallContext(CAUEngine& engine, void* ctx1, void* ctx2) noexcept {
            engine.CallContext(ctx1, ctx2);
        }
    };
}

/// <summary>
/// private intercace for CAUEngine
/// </summary>
struct PlayAU::CAUEngine::Private {
    // init api
    static Result InitAPI(CAUEngine& engine, APILevel& level) noexcept {
        Result hr = { Result::RE_NOINTERFACE };
        switch (level)
        {
        case PlayAU::APILevel::Level_Auto:
            // 尝试XAudio 2.8
            hr = InitInterfaceXAudio2_8(engine.m_buffer, *engine.m_pConfig);
            level = APILevel::Level_XAudio2_8;
            if (hr) break;
            // 尝试XAudio 2.7
            hr = InitInterfaceXAudio2_7(engine.m_buffer, *engine.m_pConfig);
            level = APILevel::Level_XAudio2_7;
            break;
        case PlayAU::APILevel::Level_XAudio2_7:
            // 尝试XAudio 2.7
            hr = InitInterfaceXAudio2_7(engine.m_buffer, *engine.m_pConfig);
            break;
        case PlayAU::APILevel::Level_XAudio2_8:
            // 尝试XAudio 2.8
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
    m_level = level;
    // 获取
    Result hr = Private::InitAPI(*this, m_level);
    // 尝试利用
    return hr;
}

/// <summary>
/// Uninitializes this instance.
/// </summary>
/// <returns></returns>
void PlayAU::CAUEngine::Uninitialize() noexcept {
    // 释放所有分组
    PlayAU::DisposeGroups(*this);
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
/// Calls the context.
/// </summary>
/// <param name="ctx1">The CTX1.</param>
/// <param name="ctx2">The CTX2.</param>
/// <returns></returns>
void PlayAU::CAUEngine::CallContext(void* ctx1, void* ctx2) noexcept {
    const auto api = reinterpret_cast<IAUAudioAPI*>(m_buffer);
    api->CallContext(ctx1, ctx2);
}


/// <summary>
/// Initializes a new instance of the <see cref="CAUEngine"/> class.
/// </summary>
PlayAU::CAUEngine::CAUEngine() noexcept {
    std::memset(m_group, 0, sizeof(m_group));
}


/// <summary>
/// Finalizes an instance of the <see cref="CAUEngine"/> class.
/// </summary>
/// <returns></returns>
PlayAU::CAUEngine::~CAUEngine() noexcept {

}

