#include "../inc/au_config.h"
#include "private/p_au_engine_interface.h"
// Ogg Vorbis
#include "../3rdparty/libvorbis/include/vorbis/codec.h"
#include "../3rdparty/libvorbis/include/vorbis/vorbisfile.h"

#include <cassert>
#include <cstring>
#include <new>


namespace PlayAU {
    // ogg read call back
    static const ov_callbacks OggAudioStreamCallback = {
        // size_t (*read_func)  (void *ptr, size_t size, size_t nmemb, void *datasource);
        [](void* buf, size_t e, size_t c, void* s) noexcept ->size_t {
            const auto stream = reinterpret_cast<XAUStream*>(s);
            return size_t(stream->ReadNext(uint32_t(e*c), buf) / e);
        },
        // int    (*seek_func)  (void *datasource, ogg_int64_t offset, int whence);
        [](void* s, ogg_int64_t offset, int whence) noexcept ->int {
            const auto stream = reinterpret_cast<XAUStream*>(s);
            const auto code = stream->Seek(int32_t(offset), XAUStream::Move(whence));
            return !code;
        },
        // int    (*close_func) (void *datasource);
        nullptr,
        // long   (*tell_func)  (void *datasource);
        [](void* s) noexcept ->long {
            const auto stream = reinterpret_cast<XAUStream*>(s);
            return long(stream->offset);
        },
    };
    /// <summary>
    /// ogg file
    /// </summary>
    struct OggFile : OggVorbis_File {
        // playau obj
        PLAYAU_OBJ;
    };
    /// <summary>
    /// ogg stream
    /// </summary>
    /// <seealso cref="XAUAudioStream" />
    struct CAUOggAudioStream final : XAUAudioStream {
    public:
        // ctor
        CAUOggAudioStream() noexcept;
        // ok
        bool IsOK() const noexcept { return !!m_pOggFile; }
        // dispose
        void Dispose() noexcept override;
        // seek stream in byte, return current position
        bool Seek(int32_t off, Move method = XAUStream::Move_Begin) noexcept override;
        // read stream, return byte count read
        auto ReadNext(uint32_t len, void* buf) noexcept->uint32_t override;
        // move to new position
        void MoveTo(void* target) noexcept override;
    private:
        // ogg file
        OggFile*             m_pOggFile = nullptr;
    };
    /// <summary>
    /// Creates the ogg audio stream.
    /// </summary>
    /// <param name="file">The file.</param>
    /// <param name="buf">The buf.</param>
    /// <returns></returns>
    bool CreateOggAudioStream(XAUStream & file, void* buf) noexcept {
        constexpr size_t sizeof_oggs = sizeof(CAUOggAudioStream);
        constexpr size_t sizeof_bufl = AUDIO_STREAM_BUFLEN;
        static_assert(sizeof_oggs <= sizeof_bufl, "overflow");
        const auto obj = new(buf) CAUOggAudioStream;
        return obj->IsOK();
    }
}


/// <summary>
/// Moves to.
/// </summary>
/// <param name="target">The target.</param>
/// <returns></returns>
void PlayAU::CAUOggAudioStream::MoveTo(void* target) noexcept {
    std::memcpy(target, this, sizeof(*this));
    const auto tar = reinterpret_cast<CAUOggAudioStream*>(target);
    const auto target_stream = tar->FileStream();
    this->FileStream()->MoveTo(target_stream);
    tar->m_pOggFile->datasource = target_stream;
#ifndef NDEBUG
    std::memset(this, 0, sizeof(*this));
#endif
}

/// <summary>
/// Initializes a new instance of the <see cref="CAUOggAudioStream"/> struct.
/// </summary>
PlayAU::CAUOggAudioStream::CAUOggAudioStream() noexcept {
    this->length = 0;
    this->offset = 0;
    OggFile file;
    if (::ov_open_callbacks(this->FileStream(), &file, nullptr, 0, PlayAU::OggAudioStreamCallback) >= 0) {
        //char **ptr = ov_comment(&ovfile, -1)->user_comments;
        vorbis_info *vi = ov_info(&file, -1);
        // 获取声道数
        this->format.channels = vi->channels;
        // 获取采样率
        this->format.samples_per_sec = vi->rate;
        // 样本深度
        this->format.bits_per_sample = sizeof(int16_t) * 8;
        // 编码: PCM
        this->format.fmt_tag = Wave_PCM;
        // 数据大小
        this->length
            = static_cast<uint32_t>(::ov_pcm_total(&file, -1)
            * this->format.channels
            * sizeof(int16_t))
            ;
        m_pOggFile = new(std::nothrow) OggFile{ file };
    }
}

/// <summary>
/// Releases unmanaged and - optionally - managed resources.
/// </summary>
/// <returns></returns>
void PlayAU::CAUOggAudioStream::Dispose() noexcept {
    if (m_pOggFile) {
        ::ov_clear(m_pOggFile);
        delete m_pOggFile;
        m_pOggFile = nullptr;
    }
    this->DisposeFS();
}

/// <summary>
/// Seeks the specified off.
/// </summary>
/// <param name="off">The off.</param>
/// <param name="method">The method.</param>
/// <returns></returns>
bool PlayAU::CAUOggAudioStream::Seek(int32_t off, Move method) noexcept {
    const int32_t block_align = this->format.channels * sizeof(int16_t);
    int32_t offset_block = off / block_align;
    // 相对移动
    switch (method)
    {
    case PlayAU::XAUStream::Move_Current:
        offset_block += static_cast<int32_t>(this->offset / block_align);
        break;
    case PlayAU::XAUStream::Move_End:
        offset_block += static_cast<int32_t>(this->length / block_align);
        break;
    }
    // 绝对定位
    this->offset = static_cast<uint32_t>(offset_block * block_align);
    const auto code = ::ov_pcm_seek(m_pOggFile, offset_block);
    return !code;
}


/// <summary>
/// Reads the next.
/// </summary>
/// <param name="len">The length.</param>
/// <param name="buf">The buf.</param>
/// <returns></returns>
auto PlayAU::CAUOggAudioStream::ReadNext(uint32_t len, void* buf) noexcept -> uint32_t {
#ifndef NDEBUG
    const auto oldbuf = reinterpret_cast<char*>(buf);
    const auto oldlen = size_t(len);
#endif
    uint32_t read = 0;
    // 循环读取数据
    while (len) {
        const auto charbuf = reinterpret_cast<char*>(buf);
#ifndef NDEBUG
        auto bk = charbuf - oldbuf;
#endif
        int bitstream = 0;
        const auto code = ::ov_read(m_pOggFile, charbuf, len, 0, 2, 1, &bitstream);
        // EOF?
        if (!code) break;
        // Error?
        if (code < 0) {
            // TODO: 错误处理
            assert(!"TODO!");
            break;
        }
        // OK!
        else {
            len -= code;
            buf = (reinterpret_cast<char*>(buf) + code);
            read += code;
        }
    }
    this->offset += read;
    return read;
}