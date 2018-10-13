#define WIN32_LEAN_AND_MEAN
#include "../inc/playau.h"
#include "../inc/au_clip.h"
#include <Windows.h>
#include "private/p_XAudio2_7.h"
#include "private/p_au_engine_interface.h"

#include <cassert>
#include <cstring>
#include <cwchar>
#include <new>


#define XAUDIO2_DEBUG_ENGINE            0x0001


// PLAYAU: DEFINE_GUID
#define PLAYAU_DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
    const GUID name = { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }

namespace PlayAU {
    // CLSID_XAudio2
    PLAYAU_DEFINE_GUID(CLSID_XAudio2, 0x5a508685, 0xa254, 0x4fba, 0x9b, 0x82, 0x9a, 0x24, 0xb0, 0x03, 0x06, 0xaf);
    // CLSID_XAudio2Debug
    PLAYAU_DEFINE_GUID(CLSID_XAudio2Debug, 0xdb05ea35, 0x0329, 0x4d4b, 0xa5, 0x3a, 0x6d, 0xea, 0xd0, 0x3d, 0x38, 0x52);
    // IID_IXAudio2
    PLAYAU_DEFINE_GUID(IID_IXAudio2, 0x8bcf1f58, 0x9fe7, 0x4583, 0x8a, 0xc6, 0xe2, 0xad, 0xc4, 0x65, 0xc8, 0xbb);
    /// <summary>
    /// XAudio ver 2.7
    /// </summary>
    struct CAUXAudio2_7 final: IAUAudioAPI,
        XAudio2::Ver2_7::IXAudio2EngineCallback {
        // XAudio2::Ver2_7::IXAudio2MasteringVoice
        using XAudio2__Ver2_7__IMastering = XAudio2::Ver2_7::IXAudio2MasteringVoice;
        // dtor
        ~CAUXAudio2_7() noexcept = default;
        // ctor
        CAUXAudio2_7(HMODULE h) noexcept : m_hDllFile(h) {}
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
        using IXAudio2SubmixVoice = XAudio2::Ver2_7::IXAudio2SubmixVoice;
        // using
        using XAUDIO2_VOICE_DETAILS = XAudio2::Ver2_7::XAUDIO2_VOICE_DETAILS;
        // using
        using XAUDIO2_SEND_DESCRIPTOR = XAudio2::Ver2_7::XAUDIO2_SEND_DESCRIPTOR;
        // using
        using XAUDIO2_VOICE_SENDS = XAudio2::Ver2_7::XAUDIO2_VOICE_SENDS;
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
        XAudio2::Ver2_7::IXAudio2*  m_pXAudio2 = nullptr;
        // Mastering
        XAudio2__Ver2_7__IMastering*m_pMastering = nullptr;
        // dll handle
        HMODULE                     m_hDllFile;
    };
    /// <summary>
    /// Initializes the interface x audio2 7.
    /// </summary>
    /// <param name="buf">The buf.</param>
    /// <returns></returns>
    auto InitInterfaceXAudio2_7(void* buf, IAUConfigure& config) noexcept ->Result {
        constexpr uint32_t sizeof_buf = AUDIO_API_BUFLEN * sizeof(void*);
        static_assert(sizeof(CAUXAudio2_7) <= sizeof_buf, "overflow");
        // 查找 XAudio2_7.dll
        const auto handle = ::LoadLibraryA("XAudio2_7.dll");
        if (!handle) return { Result::RE_NOINTERFACE };
        // 初始化API
        const auto api = new(buf) CAUXAudio2_7{ handle };
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
auto PlayAU::CAUXAudio2_7::Init(IAUConfigure& config) noexcept -> Result {
    Result hr = { Result::RS_OK };
    // 初始化接口
    hr.code = ::CoCreateInstance(
        //CLSID_XAudio2Debug,
        CLSID_XAudio2,
        nullptr,
        CLSCTX_INPROC_SERVER,
        IID_IXAudio2,
        reinterpret_cast<void**>(&m_pXAudio2)
    );
    // 初始化
    if (hr) {
        const uint32_t flag
#ifndef NDEBUG
            = XAUDIO2_DEBUG_ENGINE
#else
            = 0
#endif
            ;
        hr.code = m_pXAudio2->Initialize(0, XAudio2::XAUDIO2_DEFAULT_PROCESSOR);
    }
    // 注册回调
    if (hr) {
        XAudio2::Ver2_7::IXAudio2EngineCallback* const call = this;
        hr.code = m_pXAudio2->RegisterForCallbacks(call);
    }
    // 枚举设备
    uint32_t device_index = 0;
    if (hr) {
        char16_t buf[256]; 
        // 挑选设备
        if (const auto id = config.PickDevice(buf)) {
            // 获取设备数量
            const auto count = [this]() noexcept -> uint32_t {
                uint32_t count = 0;
                const auto code = m_pXAudio2->GetDeviceCount(&count);
                return SUCCEEDED(code) ? count : 0;
            }();
            // 遍历设备
            for (uint32_t i = 0; i != count; ++i) {
                XAudio2::Ver2_7::XAUDIO2_DEVICE_DETAILS details;
                const auto code = m_pXAudio2->GetDeviceDetails(i, &details);
                if (SUCCEEDED(code)) {
                    static_assert(sizeof(wchar_t) == sizeof(char16_t), "same!");
                    const auto src = reinterpret_cast<const wchar_t*>(id);
                    if (std::wcsstr(src, details.DeviceID)) {
                        device_index = i;
                        break;
                    }
                }
            }
        }
    }
    // 创建 Mastering
    if (hr) {
        hr.code = m_pXAudio2->CreateMasteringVoice(
            &m_pMastering,
            XAudio2::XAUDIO2_DEFAULT_CHANNELS,
            XAudio2::XAUDIO2_DEFAULT_SAMPLERATE,
            0,
            device_index,
            nullptr
        );
    }
    return hr;
}


// IMPL
#define CAUXAudio2_8 CAUXAudio2_7
#define Ver2_8 Ver2_7
#include "au_engine_xa2.impl.hpp"
