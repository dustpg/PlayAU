#define WIN32_LEAN_AND_MEAN
#include "../inc/playau.h"
#include "../inc/au_clip.h"
#include <Windows.h>
#include "private/p_XAudio2_8.h"
#include "private/p_au_engine_interface.h"

#include <cassert>
#include <cstring>
#include <cwchar>
#include <new>


namespace PlayAU {
    /// <summary>
    /// XAudio ver 2.8
    /// </summary>
    struct CAUXAudio2_8 final: IAUAudioAPI, 
        XAudio2::Ver2_8::IXAudio2EngineCallback {
        // XAudio2::Ver2_8::IXAudio2MasteringVoice
        using XAudio2__Ver2_8__IMastering = XAudio2::Ver2_8::IXAudio2MasteringVoice;
        // dtor
        ~CAUXAudio2_8() noexcept = default;
        // ctor
        CAUXAudio2_8(HMODULE h) noexcept : m_hDllFile(h) {}
        // init
        auto Init(IAUConfigure&) noexcept->Result;
    public:
        // OnProcessingPassStart
        void STDMETHODCALLTYPE OnProcessingPassStart() noexcept override;
        // OnProcessingPassEnd
        void STDMETHODCALLTYPE OnProcessingPassEnd() noexcept override;
        // OnCriticalError
        void STDMETHODCALLTYPE OnCriticalError(HRESULT code) noexcept override;
    public:
        // Ctx
        struct Ctx;
        // group
        struct Group;
        // Submix
        using IXAudio2SubmixVoice = XAudio2::Ver2_8::IXAudio2SubmixVoice;
        // using
        using XAUDIO2_VOICE_DETAILS = XAudio2::XAUDIO2_VOICE_DETAILS;
        // using
        using XAUDIO2_SEND_DESCRIPTOR = XAudio2::Ver2_8::XAUDIO2_SEND_DESCRIPTOR;
        // using
        using XAUDIO2_VOICE_SENDS = XAudio2::Ver2_8::XAUDIO2_VOICE_SENDS;
    public:
        // dispose
        void Dispose() noexcept override;
        // suspend
        void Suspend() noexcept override;
        // resume
        void Resume() noexcept override;
        // call context
        void CallContext(void* ctx1, void* ctx2) noexcept override;
        // make clip context
        bool MakeClipCtx(void*) noexcept override;
        // dispose clip context
        void DisposeClipCtx(void*) noexcept override;
        // tell clip context
        auto TellClip(const void*) noexcept ->uint32_t override;
        // play clip
        void PlayClip(void*) noexcept override;
        // pause clip context
        void PauseClip(void*) noexcept override;
        // stop clip
        void StopClip(void*) noexcept override;
        // seek clip in byte
        void SeekClip(void*, uint32_t) noexcept override;
        // ratio clip context
        auto RatioClip(void*, float*) noexcept -> float override;
        // volume clip context
        auto VolumeClip(void*, float*) noexcept -> float override;
        // live: buffer left
        auto LiveClipBuffer(void*) noexcept->uint32_t override;
        // live: buffer submit
        void LiveClipSubmit(void*, void*, uint32_t) noexcept override;
        // create group
        bool CreateGroup(CAUAudioGroup&) noexcept override;
        // dispose group
        void DisposeGroup(CAUAudioGroup&) noexcept override;
        // volume group
        auto VolumeGroup(CAUAudioGroup&, float*) noexcept -> float override;
    private:
        // stop
        void stop_clip(void*) noexcept;
    private:
        // XAudio2
        XAudio2::Ver2_8::IXAudio2*  m_pXAudio2 = nullptr;
        // Mastering
        XAudio2__Ver2_8__IMastering*m_pMastering = nullptr;
        // dll handle
        HMODULE                     m_hDllFile;
    };
    /// <summary>
    /// Initializes the interface x audio2 7.
    /// </summary>
    /// <param name="buf">The buf.</param>
    /// <returns></returns>
    auto InitInterfaceXAudio2_8(void* buf, IAUConfigure& config) noexcept ->Result {
        constexpr uint32_t sizeof_buf = AUDIO_API_BUFLEN * sizeof(void*);
        static_assert(sizeof(CAUXAudio2_8) <= sizeof_buf, "overflow");
        // 查找 XAudio2_8.dll
#ifndef NDEBUG
        const auto handle1 = ::LoadLibraryA("XAudio2_9D.dll");
        const auto handle 
            = handle1 
            ? handle1
            : ::LoadLibraryA("XAudio2_8.dll")
            ;
#else
        // 查找 XAudio2_8.dll
        const auto handle = ::LoadLibraryA("XAudio2_8.dll");
#endif
        if (!handle) return { Result::RE_NOINTERFACE };
        // 初始化API
        const auto api = new(buf) CAUXAudio2_8{ handle };
        const auto hr = api->Init(config);
        // 初始化失败
        if (!hr) {
            api->Dispose();
#ifndef NDEBUG
            std::memset(buf, 0, sizeof_buf);
#endif
        }
        return hr;
    }
}



/// <summary>
/// Initializes this instance.
/// </summary>
/// <param name="config">The configuration.</param>
/// <returns></returns>
auto PlayAU::CAUXAudio2_8::Init(IAUConfigure& config) noexcept -> Result {
    Result hr = { Result::RS_OK };
    union {
        FARPROC proc;
        HRESULT(WINAPI*XAudio2Create) (
            XAudio2::Ver2_8::IXAudio2**,
            UINT32,
            XAudio2::XAUDIO2_PROCESSOR
            ) noexcept = nullptr;
    } data;
    data.proc = ::GetProcAddress(m_hDllFile, "XAudio2Create");

    // 初始化接口
    hr.code = data.XAudio2Create(
        &m_pXAudio2,
        0,
        XAudio2::XAUDIO2_DEFAULT_PROCESSOR
    );
#ifndef NDEBUG
    // 添加调试信息
    if (hr) {
        XAudio2::XAUDIO2_DEBUG_CONFIGURATION dbg_config = { 0 };
        dbg_config.TraceMask 
            = XAudio2::XAUDIO2_LOG_WARNINGS
            | XAudio2::XAUDIO2_LOG_DETAIL
            ;
        dbg_config.BreakMask = XAudio2::XAUDIO2_LOG_WARNINGS;
        dbg_config.LogFunctionName = true;
        dbg_config.LogFileline = true;
        dbg_config.LogThreadID = true;
        dbg_config.LogTiming = true;
        m_pXAudio2->SetDebugConfiguration(&dbg_config);
    }
#endif
    // 注册回调
    if (hr) {
        XAudio2::Ver2_8::IXAudio2EngineCallback* const call = this;
        hr.code = m_pXAudio2->RegisterForCallbacks(call);
    }
    // 枚举设备
    if (hr) {
        char16_t buf[256]; 
        // 挑选设备
        const auto id = config.PickDevice(buf);
        // 创建 Mastering
        hr.code = m_pXAudio2->CreateMasteringVoice(
            &m_pMastering,
            XAudio2::XAUDIO2_DEFAULT_CHANNELS,
            XAudio2::XAUDIO2_DEFAULT_SAMPLERATE,
            0,
            reinterpret_cast<const wchar_t*>(id),
            nullptr
        );
    }
    return hr;
}


// IMPL
#include "au_engine_xa2.impl.hpp"

