#include "private/p_au_engine_interface.h"
#include "../inc/au_clip.h"
#include "../inc/au_engine.h"
#include <cassert>
#include <cstring>
#include <utility>

#ifdef PLAYAU_FLAG_NULL_THISPTR_SAFE
#define PLAYAU_NULL_RETURN(x) if (!this) return x;
#else 
#define PLAYAU_NULL_RETURN(x)
#endif

/// <summary>
/// private intercace for CAUEngine
/// </summary>
struct PlayAU::CAUEngine::Private {
    // get api
    static IAUAudioAPI* API(CAUEngine& engine) noexcept {
        return reinterpret_cast<IAUAudioAPI*>(engine.m_buffer);
    }
    // 添加片段
    static void AddClip(CAUEngine& engine, Node& node) noexcept {
        // 尾节点前面的节点指向新的节点
        engine.m_tail.prev->next = &node;
        // 新的节点执行尾节点之前的节点
        node.prev = engine.m_tail.prev;
        // 新的节点后面是尾节点
        node.next = &engine.m_tail;
        // 尾节点前面是新节点
        engine.m_tail.prev = &node;
    }
    // 添加片段
    static void RemoveClip(CAUEngine& engine, Node& node) noexcept {
        // 链接前后节点
        node.prev->next = node.next;
        node.next->prev = node.prev;
    }
};


/// <summary>
/// private intercace for CAUEngine
/// </summary>
struct PlayAU::CAUAudioClip::Private {
    // get ctx
    static auto Ctx(CAUAudioClip& clip) noexcept {
        return clip.m_context;
    }
    // get audio stream
    static auto AS(const CAUAudioClip& clip) noexcept {
        return reinterpret_cast<const XAUAudioStream*>(clip.m_asbuffer);
    }
    // get audio stream
    static auto AS(CAUAudioClip& clip) noexcept {
        return reinterpret_cast<XAUAudioStream*>(clip.m_asbuffer);
    }
    // from node
    static auto FromNode(Node& node) noexcept {
        const auto ptr = reinterpret_cast<char*>(&node);
        const auto offset = offsetof(CAUAudioClip, m_node);
        const auto obj = reinterpret_cast<CAUAudioClip*>(ptr - offset);
        return obj;
    }
};

/// <summary>
/// Prevents a default instance of the <see cref="CAUAudioClip" /> class from being created.
/// </summary>
/// <param name="engine">The engine.</param>
/// <param name="flag">The flag.</param>
/// <param name="stream">The stream.</param>
/// <param name="group">The group.</param>
PlayAU::CAUAudioClip::CAUAudioClip(
    CAUEngine& engine, 
    ClipFlag flag,
    XAUAudioStream&& stream,
    CAUAudioGroup* group0
) noexcept : m_engine(engine), m_flags(flag), group(group0) {
    constexpr size_t offset_ctx = offsetof(CAUAudioClip, m_context);
    static_assert(offset_ctx == 0, "must be 0");
    // 添加
    CAUEngine::Private::AddClip(engine, m_node);
    // 移动数据
    if (&stream) stream.MoveTo(m_asbuffer);
}

/// <summary>
/// Finalizes an instance of the <see cref="CAUAudioClip"/> class.
/// </summary>
/// <returns></returns>
PlayAU::CAUAudioClip::~CAUAudioClip() noexcept {
    // 释放节点
    CAUEngine::Private::RemoveClip(m_engine, m_node);
    // 释放上下文环境
    const auto api = CAUEngine::Private::API(m_engine);
    api->DisposeClipCtx(m_context);
    // 释放音频流
    if (!(m_flags & Flag_p_Live))
        Private::AS(*this)->Dispose();
}

namespace PlayAU {
    // create file stream
    bool CreateWinFileStream(void* buf, const char16_t file[]) noexcept;
    // create ogg audio stream
    bool CreateOggAudioStream(XAUStream& file, void*buf) noexcept;
    // create flac audio stream
    bool CreateFlacAudioStream(XAUStream& file, void*buf) noexcept;
    // create clip
    auto CreateClip(
        CAUEngine&, 
        ClipFlag, 
        XAUAudioStream&&, 
        const WaveFormat* fmt,
        const char* group
    ) noexcept->CAUAudioClip*;
    /// <summary>
    /// Disposes the clip via.
    /// </summary>
    /// <param name="node">The node.</param>
    /// <returns></returns>
    void DisposeClipVia(Node& node) noexcept {
        CAUAudioClip::Private::FromNode(node)->Destroy();
    }
}

/// <summary>
/// Creates this instance.
/// </summary>
/// <param name="engine">The engine.</param>
/// <param name="flags">The flags.</param>
/// <param name="stream">The stream.</param>
/// <returns></returns>
auto PlayAU::CreateClip(
    CAUEngine& engine, 
    ClipFlag flags, 
    XAUAudioStream&& stream,
    const WaveFormat* fmt,
    const char* group
) noexcept -> CAUAudioClip* {
    // 获取分组
    const auto group_obj = [&engine, group]() noexcept {
        const auto obj = engine.FindGroup(group);
        if (obj) return obj;
        return engine.CreateEmptyGroup(group);
    }();
    // 创建对象
    const auto obj = new(std::nothrow) CAUAudioClip{ 
        engine, 
        flags, 
        std::move(stream),
        group_obj
    };
    //alignas(CAUAudioClip) static char buf[sizeof(CAUAudioClip)];
    if (!obj) return nullptr;

    // 提供了格式
    if (fmt) CAUAudioClip::Private::AS(*obj)->format = *fmt;
    // 创建上下文环境
    const auto api = CAUEngine::Private::API(engine);
    const auto ctxok = api->MakeClipCtx(CAUAudioClip::Private::Ctx(*obj));
    if (ctxok) return obj;
#ifndef NDEBUG
    std::printf("Make clip context: failed\n");
#endif
    obj->Destroy();
    return nullptr;
}


namespace PlayAU {
    /// <summary>
    /// Creates the audio stream from file stream.
    /// </summary>
    /// <param name="file">The file.</param>
    /// <param name="buf">The buf.</param>
    bool CreateAudioStreamFromFileStream(XAUStream& file, void* asbuf) {
        alignas(uint32_t) char buf[AUDIO_HEADER_PEEK_LENGTH]; std::memset(buf, 0, sizeof(buf));
        // 先读取一定字节
        file.ReadNext(AUDIO_HEADER_PEEK_LENGTH, buf);
        file.Seek(0, XAUStream::Move_Begin);
        // 最开始4字节
        const auto bufh = reinterpret_cast<uint32_t*>(buf)[0];
        // OGG
        const auto ogg_header = []() noexcept ->uint32_t {
            union { char buf[4]; uint32_t u32; } tmp;
            tmp.buf[0] = 'O'; tmp.buf[1] = 'g'; tmp.buf[2] = 'g'; tmp.buf[3] = 'S';
            return tmp.u32;
        }();
        // OGG
        const auto flac_header = []() noexcept ->uint32_t {
            union { char buf[4]; uint32_t u32; } tmp;
            tmp.buf[0] = 'f'; tmp.buf[1] = 'L'; tmp.buf[2] = 'a'; tmp.buf[3] = 'C';
            return tmp.u32;
        }();
        // 分类讨论
        if (bufh == ogg_header) {
            return PlayAU::CreateOggAudioStream(file, asbuf);
        }
#ifdef PLAYAU_FLAG_FLAC_SUPPORT
        else if (bufh == flac_header) {
            return PlayAU::CreateFlacAudioStream(file, asbuf);
        }
#endif
        return false;
    }
}

/// <summary>
/// Creates the clip from file.
/// </summary>
/// <param name="flag">The flag.</param>
/// <param name="file">The file.</param>
/// <param name="group">The group.</param>
/// <returns></returns>
auto PlayAU::CAUEngine::CreateClipFromFile(
    ClipFlag flag, 
    const char16_t file[], 
    const char*group) noexcept -> Clip {
    alignas(void*) char asbuf[AUDIO_STREAM_BUFLEN];
    const auto fsbuf = reinterpret_cast<XAUAudioStream*>(asbuf)->fsbuffer;
    // 创建文件流
    const auto fileok = PlayAU::CreateWinFileStream(fsbuf, file);
    // 文件? 不存在
    if (!fileok) return nullptr;
    XAUStream& filestream = *(reinterpret_cast<XAUStream*>(fsbuf));
    // 利用文件流创建音频流
    const auto audiook = PlayAU::CreateAudioStreamFromFileStream(filestream, asbuf);
    // 音频? 不存在
    if (!audiook) return nullptr;
    XAUAudioStream& audiostream = *(reinterpret_cast<XAUAudioStream*>(asbuf));
    // 创建音频片段
    return this->CreateClipFromAudio(flag, std::move(audiostream), group);
}


/// <summary>
/// Creates the clip from stream.
/// </summary>
/// <param name="flag">The flag.</param>
/// <param name="stream">The stream.</param>
/// <param name="group">The group.</param>
/// <returns></returns>
auto PlayAU::CAUEngine::CreateClipFromStream(
    ClipFlag flag, 
    XAUStream& stream, 
    const char*group) noexcept -> Clip {
    alignas(void*) char asbuf[AUDIO_STREAM_BUFLEN];
    const auto fsbuf = reinterpret_cast<XAUAudioStream*>(asbuf)->fsbuffer;
    // 移动文件流
    stream.MoveTo(fsbuf);
    XAUStream& filestream = *(reinterpret_cast<XAUStream*>(fsbuf));
    // 利用文件流创建音频流
    const auto audiook = PlayAU::CreateAudioStreamFromFileStream(filestream, asbuf);
    // 音频? 不存在
    if (!audiook) return nullptr;
    XAUAudioStream& audiostream = *(reinterpret_cast<XAUAudioStream*>(asbuf));
    // 创建音频片段
    return this->CreateClipFromAudio(flag, std::move(audiostream), group);
}

/// <summary>
/// Creates the clip from audio stream.
/// </summary>
/// <param name="flag">The flag.</param>
/// <param name="stream">The stream.</param>
/// <param name="group">The group.</param>
/// <returns></returns>
auto PlayAU::CAUEngine::CreateClipFromAudio(
    ClipFlag flag, 
    XAUAudioStream&& stream, 
    const char*group) noexcept -> Clip {
    // 去掉私有标志位
    const auto f = static_cast<ClipFlag>(flag & Flag_Public);
    return CreateClip(*this, f, std::move(stream), nullptr, group);
}

/// <summary>
/// Creates the live clip.
/// </summary>
/// <param name="fmt">The FMT.</param>
/// <param name="group">The group.</param>
/// <returns></returns>
auto PlayAU::CAUEngine::CreateLiveClip(
    const WaveFormat& fmt, 
    const char* group) noexcept ->Clip {
    XAUAudioStream* stream = nullptr;
    return CreateClip(*this, Flag_p_Live, std::move(*stream), &fmt, group);
}


/// <summary>
/// Destroys this instance.
/// </summary>
/// <returns></returns>
void PlayAU::CAUAudioClip::Destroy() noexcept {
    delete this;
}

/// <summary>
/// Plays this instance.
/// </summary>
/// <returns></returns>
void PlayAU::CAUAudioClip::Play() noexcept {
    PLAYAU_NULL_RETURN((void)0);
    const auto api = CAUEngine::Private::API(m_engine);
    api->PlayClip(m_context);
}

/// <summary>
/// Pauses this instance.
/// </summary>
/// <returns></returns>
void PlayAU::CAUAudioClip::Pause() noexcept {
    PLAYAU_NULL_RETURN((void)0);
    const auto api = CAUEngine::Private::API(m_engine);
    api->PauseClip(m_context);
}

/// <summary>
/// Stops this instance.
/// </summary>
/// <returns></returns>
void PlayAU::CAUAudioClip::Stop() noexcept {
    PLAYAU_NULL_RETURN((void)0);
    const auto api = CAUEngine::Private::API(m_engine);
    api->StopClip(m_context);
}

/// <summary>
/// Durations this instance.
/// </summary>
/// <returns></returns>
auto PlayAU::CAUAudioClip::Duration() const noexcept -> double {
    PLAYAU_NULL_RETURN(0.0);
    // 计算
    const auto stream = Private::AS(*this);
    const double l = static_cast<double>(stream->length);
    const double n = (stream->format.bits_per_sample >> 3)
        * stream->format.channels
        * stream->format.samples_per_sec
        ;;
    // 计算时间
    return l / n;
}

/// <summary>
/// Seeks the specified position.
/// </summary>
/// <param name="pos">The position.</param>
/// <returns></returns>
void PlayAU::CAUAudioClip::Seek(double pos) noexcept {
    PLAYAU_NULL_RETURN((void)0);
    const auto stream = Private::AS(*this);
    const double spsec = stream->format.samples_per_sec;
    const auto pos_in_sample 
        = static_cast<uint32_t>(pos * spsec)
        * (stream->format.bits_per_sample >> 3)
        * stream->format.channels
        ;
    const auto api = CAUEngine::Private::API(m_engine);
    api->SeekClip(m_context, pos_in_sample);
}

/// <summary>
/// Tells this instance.
/// </summary>
/// <returns></returns>
auto PlayAU::CAUAudioClip::Tell() const noexcept -> double {
    PLAYAU_NULL_RETURN(0.0);
    const auto stream = Private::AS(*this);
    const auto api = CAUEngine::Private::API(m_engine);
    const auto count = api->TellClip(m_context);
    const double l = static_cast<double>(count);
    const double n 
        = stream->format.samples_per_sec
        * (stream->format.bits_per_sample >> 3)
        * stream->format.channels
        ;
    // 计算时间
    return l / n;
}


/// <summary>
/// Sets the volume.
/// </summary>
/// <param name="v">The v.</param>
/// <returns></returns>
void PlayAU::CAUAudioClip::SetVolume(float v) noexcept {
    PLAYAU_NULL_RETURN((void)0);
    const auto api = CAUEngine::Private::API(m_engine);
    api->VolumeClip(m_context, &v);
}

/// <summary>
/// Sets the frequency ratio.
/// </summary>
/// <param name="f">The f.</param>
/// <returns></returns>
void PlayAU::CAUAudioClip::SetFrequencyRatio(float f) noexcept {
    PLAYAU_NULL_RETURN((void)0);
    const auto api = CAUEngine::Private::API(m_engine);
    api->RatioClip(m_context, &f);
}

/// <summary>
/// Gets the volume.
/// </summary>
/// <returns></returns>
auto PlayAU::CAUAudioClip::GetVolume() const noexcept ->float {
    PLAYAU_NULL_RETURN(0.f);
    const auto api = CAUEngine::Private::API(m_engine);
    const auto ctx = const_cast<uintptr_t*>(m_context);
    return api->VolumeClip(ctx, nullptr);
}

/// <summary>
/// Gets the frequency ratio.
/// </summary>
/// <returns></returns>
auto PlayAU::CAUAudioClip::GetFrequencyRatio() const noexcept ->float {
    PLAYAU_NULL_RETURN(0.f);
    const auto api = CAUEngine::Private::API(m_engine);
    const auto ctx = const_cast<uintptr_t*>(m_context);
    return api->RatioClip(ctx, nullptr);
}


/// <summary>
/// Gets the live buffer left.
/// </summary>
/// <returns></returns>
auto PlayAU::CAUAudioClip::GetLiveBufferLeft() const noexcept -> uint32_t {
    PLAYAU_NULL_RETURN(0);
    const auto api = CAUEngine::Private::API(m_engine);
    return api->LiveClipBuffer(const_cast<uintptr_t*>(m_context));
}

/// <summary>
/// Sunmits the refable live buffer.
/// </summary>
/// <param name="data">The data.</param>
/// <param name="len">The length.</param>
/// <returns></returns>
void PlayAU::CAUAudioClip::SunmitRefableLiveBuffer(uint8_t* data, uint32_t len) noexcept {
    PLAYAU_NULL_RETURN((void)0);
    const auto api = CAUEngine::Private::API(m_engine);
    api->LiveClipSubmit(m_context, data, len);
}


#include <atomic>

/// <summary>
/// Sets the loop.
/// </summary>
/// <param name="is">if set to <c>true</c> [is].</param>
/// <returns></returns>
void PlayAU::CAUAudioClip::SetLoop(bool is) noexcept {
    PLAYAU_NULL_RETURN((void)0);
    // TODO: 线程安全?
    auto& flag = const_cast<ClipFlag&>(m_flags);
    static_assert(sizeof(flag) == sizeof(uint32_t), "same!");
    auto& flag_uint = reinterpret_cast<uint32_t&>(flag);
    if (is) flag_uint |= Flag_LoopInfinite;
    else flag_uint &= ~Flag_LoopInfinite;
}