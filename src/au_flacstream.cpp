#include "../inc/au_config.h"
#ifdef PLAYAU_FLAG_FLAC_SUPPORT
#include "private/p_au_engine_interface.h"
// Flac
#define FLAC__NO_DLL
#include "../3rdparty/libflac/include/FLAC/stream_decoder.h"

#include <cassert>
#include <cstring>
#include <new>


namespace PlayAU {
    /// <summary>
    /// flac stream
    /// </summary>
    /// <seealso cref="XAUAudioStream" />
    struct CAUFlacAudioStream final : XAUAudioStream {
    public:
        // ctor
        CAUFlacAudioStream() noexcept;
        // Ctor failed
        bool CtorFailed() const noexcept { return !m_pDecoder; }
        // init
        bool Init() noexcept;
        // dispose
        void Dispose() noexcept override;
        // seek stream in byte, return current position
        bool Seek(int32_t off, Move method = XAUStream::Move_Begin) noexcept override;
        // read stream, return byte count read
        auto ReadNext(uint32_t len, void* buf) noexcept->uint32_t override;
        // move to new position
        void MoveTo(void* target) noexcept override;
        // code
        //template<typename T> void Decode(const FLAC__int32* const buffer[], uint32_t count, uint32_t ch) noexcept;
        // decode  8bit
        //void Decode08(const FLAC__int32* const buffer[], uint32_t count, uint32_t ch) noexcept;
        // decode 16bit
        void Decode16(const FLAC__int32* const buffer[], uint32_t count, uint32_t ch) noexcept;
        // decode 32bit
        //void Decode32(const FLAC__int32* const buffer[], uint32_t count, uint32_t ch) noexcept;
    private:
        // flac decoder
        FLAC__StreamDecoder*    m_pDecoder = ::FLAC__stream_decoder_new();
        // write pointer
        uint8_t*                m_pWrite = nullptr;
        // write left
        uint32_t                m_left = 0;
        // rigth shift count
        uint32_t                m_shift = 0;
    };
    // replace flac data
    template<typename T> static inline 
    bool ReplaceFlacCtx(FLAC__StreamDecoder* decoder, T a, T b) noexcept {
        const auto ptr = reinterpret_cast<T*>(decoder->private_);
        for (int i = 0; i != 100; ++i) {
            if (ptr[i] == a) {
                ptr[i] = b;
                return true;
            }
        }
        assert(!"failed");
        return false;
    }
    /// <summary>
    /// Creates the flac audio stream.
    /// </summary>
    /// <param name="file">The file.</param>
    /// <param name="buf">The buf.</param>
    /// <returns></returns>
    bool CreateFlacAudioStream(XAUStream & file, void* buf) noexcept {
        constexpr size_t sizeof_flac = sizeof(CAUFlacAudioStream);
        constexpr size_t sizeof_bufl = AUDIO_STREAM_BUFLEN;
        static_assert(sizeof_flac <= sizeof_bufl, "overflow");
        const auto obj = new(buf) CAUFlacAudioStream;
        if (obj->CtorFailed()) return false;
        return obj->Init();
    }
}


/// <summary>
/// Initializes a new instance of the <see cref="CAUFlacAudioStream"/> struct.
/// </summary>
PlayAU::CAUFlacAudioStream::CAUFlacAudioStream() noexcept {
    this->length = 0;
    this->offset = 0;
}

/// <summary>
/// Initializes this instance.
/// </summary>
/// <returns></returns>
bool PlayAU::CAUFlacAudioStream::Init() noexcept {
    // 读取FLAC流
    const auto read_flac = [](
        const FLAC__StreamDecoder *decoder,
        FLAC__byte buffer[],
        size_t *bytes, 
        void *client_data
        ) noexcept ->FLAC__StreamDecoderReadStatus {
        const auto stream = reinterpret_cast<CAUFlacAudioStream*>(client_data);
        const auto fs = stream->FileStream();
        const auto len = fs->ReadNext(*bytes, buffer);
        *bytes = len;
        return fs->offset < fs->length
            ? FLAC__STREAM_DECODER_READ_STATUS_CONTINUE
            : FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM
            ;
    };
    // 定位FLAC流
    const auto seek_flac = [](
        const FLAC__StreamDecoder *decoder,
        FLAC__uint64 absolute_byte_offset, 
        void *client_data
        ) noexcept ->FLAC__StreamDecoderSeekStatus {
        const auto stream = reinterpret_cast<CAUFlacAudioStream*>(client_data);
        const auto offset = static_cast<int32_t>(absolute_byte_offset);
        const auto ok = stream->FileStream()->Seek(offset, XAUStream::Move_Begin);
        return ok 
            ? FLAC__STREAM_DECODER_SEEK_STATUS_OK
            : FLAC__STREAM_DECODER_SEEK_STATUS_ERROR
            ;
    };
    // 判定Flac流
    const auto tell_flac = [](
        const FLAC__StreamDecoder *decoder,
        FLAC__uint64* absolute_byte_offset,
        void *client_data
        ) noexcept ->FLAC__StreamDecoderTellStatus {
        const auto stream = reinterpret_cast<CAUFlacAudioStream*>(client_data);
        *absolute_byte_offset = stream->FileStream()->offset;
        return FLAC__STREAM_DECODER_TELL_STATUS_OK;
    };
    // 记长Flac流
    const auto len_flac = [](
        const FLAC__StreamDecoder *decoder,
        FLAC__uint64* stream_length,
        void *client_data
        ) noexcept ->FLAC__StreamDecoderLengthStatus {
        const auto stream = reinterpret_cast<CAUFlacAudioStream*>(client_data);
        const auto fs = stream->FileStream();
        *stream_length = fs->length;
        return FLAC__STREAM_DECODER_LENGTH_STATUS_OK;
    };
    // EOF
    const auto eof_flac = [](
        const FLAC__StreamDecoder *decoder,
        void *client_data
        ) noexcept ->FLAC__bool {
        const auto stream = reinterpret_cast<CAUFlacAudioStream*>(client_data);
        const auto fs = stream->FileStream();
        return fs->offset >= fs->length;
    };
    // 写入流
    const auto write_flac = [](
        const FLAC__StreamDecoder *decoder, 
        const FLAC__Frame *frame, 
        const FLAC__int32 * const buffer[], 
        void *client_data
    ) noexcept ->FLAC__StreamDecoderWriteStatus {
        const auto stream = reinterpret_cast<CAUFlacAudioStream*>(client_data);
        // 不支持
        stream->m_pWrite = nullptr;
        stream->m_left = 0;
        return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
    };
    // 元数据
    const auto meta_flac = [](
        const FLAC__StreamDecoder *decoder, 
        const FLAC__StreamMetadata *metadata, 
        void *client_data) noexcept {
        const auto stream = reinterpret_cast<CAUFlacAudioStream*>(client_data);
        switch (metadata->type)
        {
        case FLAC__METADATA_TYPE_STREAMINFO:
            stream->format.channels = metadata->data.stream_info.channels;
            stream->format.samples_per_sec = metadata->data.stream_info.sample_rate;

            // 取整到对齐字节
            stream->format.bits_per_sample = metadata->data.stream_info.bits_per_sample;
            stream->format.bits_per_sample += 7;
            stream->format.bits_per_sample &= ~(uint16_t)7;

            stream->m_shift 
                = stream->format.bits_per_sample 
                - metadata->data.stream_info.bits_per_sample
                ;

            stream->format.fmt_tag = Wave_PCM;
            // 数据大小
            stream->length = static_cast<uint32_t>(
                metadata->data.stream_info.total_samples
                * metadata->data.stream_info.channels
                * sizeof(int16_t))
                ;
            break;
        }
    };
    // 流错误
    const auto error_flac = [](
        const FLAC__StreamDecoder *decoder, 
        FLAC__StreamDecoderErrorStatus status, 
        void *client_data
        ) noexcept ->void {
    };

    // 初始化
    const auto status = ::FLAC__stream_decoder_init_stream(
        m_pDecoder,
        read_flac,
        seek_flac,
        tell_flac,
        len_flac,
        eof_flac,
        write_flac,
        meta_flac,
        error_flac,
        this
    );
    // 错误
    if (status != FLAC__STREAM_DECODER_INIT_STATUS_OK) return false;
    // 先解码一帧
    const auto ok = ::FLAC__stream_decoder_process_single(m_pDecoder);


    // 写入流 - 16bit
    const auto write_flac16 = [](
        const FLAC__StreamDecoder *decoder,
        const FLAC__Frame *frame,
        const FLAC__int32 * const buffer[],
        void *client_data
        ) noexcept ->FLAC__StreamDecoderWriteStatus {
        const auto count = frame->header.blocksize;
        const auto stream = reinterpret_cast<CAUFlacAudioStream*>(client_data);
        stream->Decode16(buffer, count, frame->header.channels);
        return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
    };
    // 16BIT
    switch (this->format.bits_per_sample >> 3)
    {
        FLAC__StreamDecoderWriteCallback a, b;

    //case 1:
        // 8bit
        //a = write_flac;
        //b = write_flac16;
        //PlayAU::ReplaceFlacCtx(m_pDecoder, a, b);
        //break;
    case 2:
        // 16bit
        a = write_flac;
        b = write_flac16;
        PlayAU::ReplaceFlacCtx(m_pDecoder, a, b);
        break;
    //case 3:
        // 24bit
        //a = write_flac;
        //b = write_flac16;
        //PlayAU::ReplaceFlacCtx(m_pDecoder, a, b);
        //break;
    //case 4:
        // 32bit
        //a = write_flac;
        //b = write_flac16;
        //PlayAU::ReplaceFlacCtx(m_pDecoder, a, b);
        //break;
    }

    return ok;
}


/// <summary>
/// Decode16s the specified buffer.
/// </summary>
/// <param name="buffer">The buffer.</param>
/// <param name="count">The count.</param>
/// <returns></returns>
void PlayAU::CAUFlacAudioStream::Decode16(const FLAC__int32* const buffer[], const uint32_t count, const uint32_t ch) noexcept {
    // 声道一般很少, 长度却很多
    using sample_t = int16_t;
    const auto output = reinterpret_cast<sample_t*>(m_pWrite);

    // 计算长度
    const uint32_t length = count * ch * sizeof(sample_t);
    this->offset += length;
    // TODO: SEEK 后可能包含部分块

    // 剩余缓存足够?
    if (m_left >= length) {
        m_pWrite += length;
        m_left -= length;
        // 计算
        for (uint32_t i = 0; i != ch; ++i) {
            const auto this_chn = buffer[i];
            for (uint32_t j = 0; j != count; ++j) {
                output[j * ch + i] = static_cast<sample_t>(this_chn[j]);
            }
        }
    }
    else m_left = 0;

}

/// <summary>
/// Moves to.
/// </summary>
/// <param name="target">The target.</param>
/// <returns></returns>
void PlayAU::CAUFlacAudioStream::MoveTo(void* target) noexcept {
    std::memcpy(target, this, sizeof(*this));
    const auto tar = reinterpret_cast<CAUFlacAudioStream*>(target);
    const auto target_stream = tar->FileStream();
    const auto this_stream = this->FileStream();
    this_stream->MoveTo(target_stream);
    // 搜索上下文
    void* const this_ptr = this;
    const auto ok = PlayAU::ReplaceFlacCtx(m_pDecoder, this_ptr, target);
    assert(ok && "failed");
#ifndef NDEBUG
    std::memset(this, 0, sizeof(*this));
#endif
}

/// <summary>
/// Releases unmanaged and - optionally - managed resources.
/// </summary>
/// <returns></returns>
void PlayAU::CAUFlacAudioStream::Dispose() noexcept {
    if (m_pDecoder) {
        ::FLAC__stream_decoder_delete(m_pDecoder);
        m_pDecoder = nullptr;
    }
    this->DisposeFS();
}

/// <summary>
/// Seeks the specified off.
/// </summary>
/// <param name="off">The off.</param>
/// <param name="method">The method.</param>
/// <returns></returns>
bool PlayAU::CAUFlacAudioStream::Seek(int32_t off, Move method) noexcept {
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
    const auto ok = ::FLAC__stream_decoder_seek_absolute(m_pDecoder, offset_block);
    assert(ok && "failed");
    return ok;
}


/// <summary>
/// Reads the next.
/// </summary>
/// <param name="len">The length.</param>
/// <param name="buf">The buf.</param>
/// <returns></returns>
auto PlayAU::CAUFlacAudioStream::ReadNext(uint32_t len, void* buf) noexcept -> uint32_t {
    m_left = len;
    m_pWrite = reinterpret_cast<uint8_t*>(buf);
    while (m_left) {
        const auto ok = ::FLAC__stream_decoder_process_single(m_pDecoder);
        if (!ok) break;
        const auto fs = this->FileStream();
        if (fs->offset >= fs->length) break;
    }
    const auto left = m_left;
    m_left = 0;
    m_pWrite = nullptr;
    return len - left;
}
#endif