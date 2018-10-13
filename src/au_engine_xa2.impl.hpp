
namespace PlayAU {
    /// <summary>
    /// private data for clip
    /// </summary>
    struct CAUAudioClip::Private {
        // get audio stream
        static auto&Engine(CAUAudioClip& clip) noexcept {
            return clip.m_engine; }
        // get audio stream
        static auto AudioStream(CAUAudioClip& clip) noexcept {
            return reinterpret_cast<XAUAudioStream*>(clip.m_asbuffer); }
        // get playing
        static bool&Playing(CAUAudioClip& clip) noexcept {
            return clip.m_playing; }
        // get pausing
        static bool&Pausing(CAUAudioClip& clip) noexcept {
            return clip.m_pausing; }
        // get flag
        static auto Flag(const CAUAudioClip& clip) noexcept {
            return clip.m_flags; }
    };
}

/// <summary>
/// Called when [processing pass start].
/// </summary>
/// <returns></returns>
void PlayAU::CAUXAudio2_8::OnProcessingPassStart() noexcept {

}

/// <summary>
/// Called when [processing pass end].
/// </summary>
/// <returns></returns>
void PlayAU::CAUXAudio2_8::OnProcessingPassEnd() noexcept {
}

/// <summary>
/// Called when [critical error].
/// </summary>
/// <returns></returns>
void PlayAU::CAUXAudio2_8::OnCriticalError(HRESULT code) noexcept {
    // TODO: 分别处理
}

/// <summary>
/// Releases unmanaged and - optionally - managed resources.
/// </summary>
/// <returns></returns>
void PlayAU::CAUXAudio2_8::Dispose() noexcept {
    if (m_pMastering) m_pMastering->DestroyVoice();
    PlayAU::SafeRelease(m_pXAudio2);
    ::FreeLibrary(m_hDllFile);
}

/// <summary>
/// Suspends this instance.
/// </summary>
/// <returns></returns>
void PlayAU::CAUXAudio2_8::Suspend() noexcept {
    m_pXAudio2->StopEngine();
}


/// <summary>
/// Resumes this instance.
/// </summary>
/// <returns></returns>
void PlayAU::CAUXAudio2_8::Resume() noexcept {
    const auto hr = m_pXAudio2->StartEngine();
    // TODO: 错误处理
    assert(SUCCEEDED(hr));
}


/// <summary>
/// private intercace for CAUEngine
/// </summary>
struct PlayAU::CAUEngine::Private {
    // get api
    static auto API(CAUEngine& engine) noexcept {
        return reinterpret_cast<IAUAudioAPI*>(engine.m_buffer);
    }
    // get engine from api
    static auto FromAPI(IAUAudioAPI& api) noexcept {
        const auto addr = reinterpret_cast<char*>(&api);
        const size_t offset = offsetof(CAUEngine, m_buffer);
        const auto obj = reinterpret_cast<CAUEngine*>(addr - offset);
        assert(API(*obj) == &api);
        return obj;
    }
    // get cfg
    static auto Config(CAUEngine& engine) noexcept {
        return engine.m_pConfig;
    }
};


/// <summary>
/// context for XAudio2_8
/// </summary>
struct PlayAU::CAUXAudio2_8::Ctx final : XAudio2::Ver2_8::IXAudio2VoiceCallback {
public:
    // Called just before this voice's processing pass begins.
    void STDMETHODCALLTYPE OnVoiceProcessingPassStart(UINT32 SamplesRequired) noexcept override;
    // Called just after this voice's processing pass ends.
    void STDMETHODCALLTYPE OnVoiceProcessingPassEnd() noexcept override {}
    // Called when this voice has just finished playing a buffer stream
    void STDMETHODCALLTYPE OnStreamEnd() noexcept override;
    // Called when this voice is about to start processing a new buffer.
    void STDMETHODCALLTYPE OnBufferStart(void * pBufferContext) noexcept override;
    // Called when this voice has just finished processing a buffer.
    // The buffer can now be reused or destroyed.
    void STDMETHODCALLTYPE OnBufferEnd(void * pBufferContext) noexcept override;
    // Called when this voice has just reached the end position of a loop.
    void STDMETHODCALLTYPE OnLoopEnd(void * pBufferContext) noexcept override { /*m_bPlaying = false;*/ }
    // Called in the event of a critical error during voice processing,
    // such as a failing xAPO or an error from the hardware XMA decoder.
    // The voice may have to be destroyed and re-created to recover from
    // the error.  The callback arguments report which buffer was being
    // processed when the error occurred, and its HRESULT code.
    void STDMETHODCALLTYPE OnVoiceError(void * pBufferContext, HRESULT Error) noexcept override;
public:
    // ctor
    Ctx() noexcept = default;
    // dtor
    ~Ctx() noexcept = default;
    // dispose
    void Dispose() noexcept;
    // submit next buffer
    void SubmitNext() noexcept;
    // flag
    auto Flag() const noexcept {
        auto& clip = *reinterpret_cast<const CAUAudioClip*>(this);
        return CAUAudioClip::Private::Flag(clip);
    }
    // playing
    bool&Playing() noexcept {
        auto& clip = *reinterpret_cast<CAUAudioClip*>(this);
        return CAUAudioClip::Private::Playing(clip);
    }
    // pauing
    bool&Pausing() noexcept {
        auto& clip = *reinterpret_cast<CAUAudioClip*>(this);
        return CAUAudioClip::Private::Pausing(clip);
    }
    // audio stream
    auto AudioStream() noexcept {
        auto& clip = *reinterpret_cast<CAUAudioClip*>(this);
        return CAUAudioClip::Private::AudioStream(clip);
    }
    // API
    auto&Engine() noexcept {
        auto& clip = *reinterpret_cast<CAUAudioClip*>(this);
        auto& engine = CAUAudioClip::Private::Engine(clip);
        return engine;
    }
public:
    // post submit
    void PostSubmit() {
        auto& engine = this->Engine();
        const auto config = CAUEngine::Private::Config(engine);
        config->CallContext(engine, this, nullptr);
    }
public:
    // source
    XAudio2::Ver2_8::IXAudio2SourceVoice*       source = nullptr;
    // bucket/data data
    Bucket*                                     buffer = nullptr;
    // next bucket id
    uint32_t                                    bucket = 0;
};


/// <summary>
/// Releases unmanaged and - optionally - managed resources.
/// </summary>
/// <returns></returns>
void PlayAU::CAUXAudio2_8::Ctx::Dispose() noexcept {
    // 释放Source
    if (this->source) {
        this->source->DestroyVoice();
        this->source = nullptr;
    }
    // 释放缓存
    if (this->Flag() & Flag_p_Live) {

    }
    else {
        delete buffer;
        this->buffer = nullptr;
    }
}

/// <summary>
/// Calls the context.
/// </summary>
/// <param name="ctx1">The CTX1.</param>
/// <param name="ctx2">The CTX2.</param>
/// <returns></returns>
void PlayAU::CAUXAudio2_8::CallContext(void* ctx1, void* ctx2) noexcept {
    const auto ctx = reinterpret_cast<Ctx*>(ctx1);
    ctx->SubmitNext();
}



/// <summary>
/// Disposes the clip CTX.
/// </summary>
/// <param name="buf">The buf.</param>
/// <returns></returns>
void PlayAU::CAUXAudio2_8::DisposeClipCtx(void* buf) noexcept {
    const auto ctx = reinterpret_cast<CAUXAudio2_8::Ctx*>(buf);
    ctx->Dispose();
}


/// <summary>
/// Tells the clip.
/// </summary>
/// <param name="ctx">The CTX.</param>
/// <returns></returns>
auto PlayAU::CAUXAudio2_8::TellClip(const void* ctx) noexcept -> uint32_t {
    const auto obj = reinterpret_cast<CAUXAudio2_8::Ctx*>(
        const_cast<void*>(ctx)
        );
    const auto src = obj->source;
    assert(src && "bad action");
    XAudio2::XAUDIO2_VOICE_STATE state;
#ifdef Ver2_8
    src->GetState(&state);
#else
    src->GetState(&state, XAudio2::XAUDIO2_VOICE_NOSAMPLESPLAYED);
#endif
    const auto data = reinterpret_cast<uintptr_t>(state.pCurrentBufferContext);
    return static_cast<uint32_t>(data);
}

/// <summary>
/// Plays the clip.
/// </summary>
/// <param name="ctx">The CTX.</param>
/// <returns></returns>
void PlayAU::CAUXAudio2_8::PlayClip(void* ctx) noexcept {
    const auto obj = reinterpret_cast<CAUXAudio2_8::Ctx*>(ctx);
    const auto src = obj->source;
    assert(src && "bad action");
    if (obj->Pausing()) {
        obj->Pausing() = false;
    }
    // 非live
    if (!(obj->Flag() & Flag_p_Live)) {
        XAudio2::XAUDIO2_VOICE_STATE state = { 0 };
#ifdef Ver2_8
        src->GetState(&state);
#else
        src->GetState(&state, XAudio2::XAUDIO2_VOICE_NOSAMPLESPLAYED);
#endif
        const int count = int(BUCKET_COUNT - state.BuffersQueued) - 1;
        for (int i = 0; i < count; ++i)
            obj->SubmitNext();
    }
    const auto hr = src->Start(0);
    // TODO: 错误处理
    assert(SUCCEEDED(hr));
    obj->Playing() = true;
}


/// <summary>
/// Pauses the clip.
/// </summary>
/// <param name="ctx">The CTX.</param>
/// <returns></returns>
void PlayAU::CAUXAudio2_8::PauseClip(void* ctx) noexcept {
    this->stop_clip(ctx);
    const auto obj = reinterpret_cast<CAUXAudio2_8::Ctx*>(ctx);
    obj->Pausing() = true;
}

/// <summary>
/// Stops the clip.
/// </summary>
/// <param name="ctx">The CTX.</param>
/// <returns></returns>
void PlayAU::CAUXAudio2_8::StopClip(void* ctx) noexcept {
    this->stop_clip(ctx);
    const auto obj = reinterpret_cast<CAUXAudio2_8::Ctx*>(ctx);
    const auto src = obj->source;
    assert(src && "bad action");
    src->FlushSourceBuffers();
    obj->AudioStream()->Seek(0, XAUStream::Move_Begin);
}

/// <summary>
/// Stops the clip.
/// </summary>
/// <param name="ctx">The CTX.</param>
/// <returns></returns>
void PlayAU::CAUXAudio2_8::stop_clip(void* ctx) noexcept {
    const auto obj = reinterpret_cast<CAUXAudio2_8::Ctx*>(ctx);
    const auto src = obj->source;
    assert(src && "bad action");
    const auto hr1 = src->Stop();
    obj->Playing() = false;
    // TODO: 错误处理
    assert(SUCCEEDED(hr1));
}

/// <summary>
/// Volumes the clip.
/// </summary>
/// <param name="ctx">The CTX.</param>
/// <param name="set">The set.</param>
/// <returns></returns>
auto PlayAU::CAUXAudio2_8::VolumeClip(void* ctx, float* set) noexcept -> float {
    const auto obj = reinterpret_cast<CAUXAudio2_8::Ctx*>(ctx);
    const auto src = obj->source;
    assert(src && "bad action");
    if (set) {
        src->SetVolume(*set);
        return *set;
    }
    else {
        float vol = 0.f;
        src->GetVolume(&vol);
        return vol;
    }
}

/// <summary>
/// Ratioes the clip.
/// </summary>
/// <param name="ctx">The CTX.</param>
/// <param name="set">The set.</param>
/// <returns></returns>
auto PlayAU::CAUXAudio2_8::RatioClip(void* ctx, float* set) noexcept -> float {
    const auto obj = reinterpret_cast<CAUXAudio2_8::Ctx*>(ctx);
    const auto src = obj->source;
    assert(src && "bad action");
    if (set) {
        src->SetFrequencyRatio(*set);
        return *set;
    }
    else {
        float freq = 0.f;
        src->GetFrequencyRatio(&freq);
        return freq;
    }
}


/// <summary>
/// Seeks the clip.
/// </summary>
/// <param name="ctx">The CTX.</param>
/// <param name="pos">The position.</param>
/// <returns></returns>
void PlayAU::CAUXAudio2_8::SeekClip(void* ctx, uint32_t pos) noexcept {
    const auto obj = reinterpret_cast<CAUXAudio2_8::Ctx*>(ctx);
    obj->AudioStream()->Seek(pos, XAUStream::Move_Begin);
}



/// <summary>
/// Lives the clip buffer.
/// </summary>
/// <param name="ctx">The CTX.</param>
/// <returns></returns>
auto PlayAU::CAUXAudio2_8::LiveClipBuffer(void* ctx) noexcept->uint32_t {
    const auto obj = reinterpret_cast<CAUXAudio2_8::Ctx*>(ctx);
    const auto src = obj->source;
    assert(src && "bad action");
    assert(obj->Flag() & PlayAU::Flag_p_Live);
    XAudio2::XAUDIO2_VOICE_STATE state = { 0 };
#ifdef Ver2_8
    src->GetState(&state);
#else
    src->GetState(&state, XAudio2::XAUDIO2_VOICE_NOSAMPLESPLAYED);
#endif
    const auto aid = reinterpret_cast<uintptr_t>(obj->buffer);
    const auto bid = reinterpret_cast<uintptr_t>(state.pCurrentBufferContext);
    if (!bid) return 0;
    return static_cast<uint32_t>(aid - bid);
}

/// <summary>
/// Lives the clip submit.
/// </summary>
/// <param name="ctx">The CTX.</param>
/// <param name="buf">The buf.</param>
/// <param name="len">The length.</param>
/// <returns></returns>
void PlayAU::CAUXAudio2_8::LiveClipSubmit(void* ctx, void* buf, uint32_t len) noexcept {
    const auto obj = reinterpret_cast<CAUXAudio2_8::Ctx*>(ctx);
    const auto src = obj->source;
    assert(src && "bad action");
    XAudio2::XAUDIO2_BUFFER buffer = { 0 };
    buffer.pAudioData = reinterpret_cast<uint8_t*>(buf);
    buffer.AudioBytes = len;
    ++reinterpret_cast<uintptr_t&>(obj->buffer);
    buffer.pContext = obj->buffer;
    const auto hr = src->SubmitSourceBuffer(&buffer);
    // TODO: 错误处理
    assert(SUCCEEDED(hr));
}

// Group
#include "../inc/au_group.h"

namespace PlayAU {
    /// <summary>
    /// group impl
    /// </summary>
    struct CAUXAudio2_8::Group final : CAUAudioGroup {
        // dtor
        ~Group() noexcept;
        // ctor
        Group(IAUAudioAPI& e) noexcept : CAUAudioGroup(e) {};
        // submix voice
        IXAudio2SubmixVoice*        submix = nullptr;
    };
    /// <summary>
    /// Finalizes an instance of the <see cref="Group"/> class.
    /// </summary>
    /// <returns></returns>
    CAUXAudio2_8::Group::~Group() noexcept {
        if (this->submix) {
            this->submix->DestroyVoice();
            this->submix = nullptr;
        }
    }
}


/// <summary>
/// Creates the group.
/// </summary>
/// <param name="group">The group.</param>
/// <returns></returns>
bool PlayAU::CAUXAudio2_8::CreateGroup(CAUAudioGroup& group) noexcept {
    const auto obj = new(&group) CAUXAudio2_8::Group{ *this };
    static_assert(sizeof(*obj) <= GROUP_BUFLEN_BYTE, "overflow");
    XAUDIO2_VOICE_DETAILS details;
    m_pMastering->GetVoiceDetails(&details);
    const auto hr = m_pXAudio2->CreateSubmixVoice(
        &obj->submix,
        details.InputChannels,
        details.InputSampleRate
    );
    return SUCCEEDED(hr);
}

/// <summary>
/// Disposes the group.
/// </summary>
/// <param name="group">The group.</param>
/// <returns></returns>
void PlayAU::CAUXAudio2_8::DisposeGroup(CAUAudioGroup& group) noexcept {
    auto& obj = static_cast<CAUXAudio2_8::Group&>(group);
    obj.~Group();
}

/// <summary>
/// Volumes the group.
/// </summary>
/// <param name="group">The group.</param>
/// <param name="vol">The vol.</param>
/// <returns></returns>
auto PlayAU::CAUXAudio2_8::VolumeGroup(CAUAudioGroup& group, float* set) noexcept -> float {
    const auto obj = reinterpret_cast<CAUXAudio2_8::Group*>(&group);
    const auto submix = obj->submix;
    assert(submix && "bad action");
    if (set) {
        submix->SetVolume(*set);
        return *set;
    }
    else {
        float vol = 0.f;
        submix->GetVolume(&vol);
        return vol;
    }
}


/// <summary>
/// 提交下一区域缓存
/// </summary>
/// <returns></returns>
void PlayAU::CAUXAudio2_8::Ctx::SubmitNext() noexcept {
    uint8_t* const base = this->buffer->data;
    const auto ptr = base + this->bucket * BUCKET_LENGTH;
    const auto stream = this->AudioStream();
    const auto len = stream->ReadNext(BUCKET_LENGTH, ptr);
    const auto pos = stream->offset;
    const auto all = stream->length;
    ++this->bucket;
    this->bucket = this->bucket % PlayAU::BUCKET_COUNT;
    // 数据有效
    if (!len) return;
    // 提交数据
    XAudio2::XAUDIO2_BUFFER buffer = { 0 };
    if (pos >= all) buffer.Flags = XAudio2::XAUDIO2_END_OF_STREAM;
    buffer.AudioBytes = len;
    buffer.pAudioData = ptr;
    buffer.pContext = reinterpret_cast<void*>(pos-len/2);
    const auto hr = this->source->SubmitSourceBuffer(&buffer, nullptr);
    // TODO: 错误处理
    assert(SUCCEEDED(hr));
}



/// <summary>
/// Called when [voice processing pass start].
/// 音频处理开始
/// </summary>
/// <param name="SamplesRequired">The samples required.</param>
/// <returns></returns>
void PlayAU::CAUXAudio2_8::Ctx::OnVoiceProcessingPassStart(UINT32 SamplesRequired) noexcept {
}

/// <summary>
/// Called when [stream end].
/// 音频流结束
/// </summary>
/// <returns></returns>
void PlayAU::CAUXAudio2_8::Ctx::OnStreamEnd() noexcept {
    this->Playing() = false;
}


/// <summary>
/// Called when [buffer start].
/// </summary>
/// <param name="pBufferContext">The p buffer context.</param>
/// <returns></returns>
void PlayAU::CAUXAudio2_8::Ctx::OnBufferStart(void * pBufferContext) noexcept {
    // this->Playing()
    this->PostSubmit();
}

/// <summary>
/// Called when [buffer end].
/// 缓冲区结束
/// </summary>
/// <param name="pBufferContext">The p buffer context.</param>
/// <returns></returns>
void PlayAU::CAUXAudio2_8::Ctx::OnBufferEnd(void* pBufferContext) noexcept {

}

/// <summary>
/// Called when [voice error].
/// </summary>
/// <param name="pBufferContext">The p buffer context.</param>
/// <param name="Error">The error.</param>
/// <returns></returns>
void PlayAU::CAUXAudio2_8::Ctx::OnVoiceError(void* pBufferContext, HRESULT Error) noexcept {
    assert(!"TODO: ERROR HANDLE");
}




// ---------------------------------




/// <summary>
/// Makes the clip CTX.
/// </summary>
/// <param name="ctx">The CTX.</param>
/// <returns></returns>
bool PlayAU::CAUXAudio2_8::MakeClipCtx(void* buf) noexcept {
    constexpr size_t sizeof_ctx = sizeof(void*) * AUDIO_CTX_BUFLEN;
    static_assert(sizeof(PlayAU::CAUXAudio2_8::Ctx) <= sizeof_ctx, "overflow");
    const auto ctx = new(buf) CAUXAudio2_8::Ctx;
    auto& clip = *reinterpret_cast<CAUAudioClip*>(ctx);
    const auto stream = CAUAudioClip::Private::AudioStream(clip);
    // 创建Source
    XAudio2::WAVEFORMATEX fmt = { 0 };
    fmt.wFormatTag = stream->format.fmt_tag;
    fmt.nChannels = stream->format.channels;
    fmt.nSamplesPerSec = stream->format.samples_per_sec;
    fmt.wBitsPerSample = stream->format.bits_per_sample;
    fmt.nBlockAlign = (fmt.wBitsPerSample >> 3) * fmt.nChannels;
    fmt.nAvgBytesPerSec = fmt.nBlockAlign * fmt.nSamplesPerSec;
    // Live不需要回调
    const auto callback
        = (ctx->Flag() & Flag_p_Live)
        ? nullptr
        : ctx
        ;
    // 创建Source
    HRESULT hr = m_pXAudio2->CreateSourceVoice(
        &ctx->source,
        &fmt,
        0,
        XAudio2::XAUDIO2_DEFAULT_FREQ_RATIO,
        callback
    );
    // 设置分组
    if (SUCCEEDED(hr)) {
        // 存在的话
        if (const auto group = static_cast<Group*>(clip.group)) {
            // 设置输出链
            XAUDIO2_SEND_DESCRIPTOR descriptors[] = {
                { 0, group->submix }
            };
            XAUDIO2_VOICE_SENDS sends = {
                sizeof(descriptors) / sizeof(*descriptors),
                descriptors
            };
            hr = ctx->source->SetOutputVoices(&sends);
        }
    }
    // 申请N个桶
    if (SUCCEEDED(hr)) {
        // live就直接开始
        if (ctx->Flag() & Flag_p_Live) {
            hr = ctx->source->Start(0);
        }
        else {
            if (const auto obj = new(std::nothrow) Bucket) {
                ctx->buffer = obj;
            }
            else hr = E_OUTOFMEMORY;
        }
    }
    return SUCCEEDED(hr);
}