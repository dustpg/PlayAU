#include "../inc/au_config.h"
#include "private/p_au_engine_interface.h"
#ifdef PLAYAU_FLAG_MP3_SUPPORT
// Mp3 
#define MINIMP3_IMPLEMENTATION
#include "../3rdparty/minimp3/minimp3.h"

#include <cassert>
#include <cstring>
#include <new>


namespace PlayAU {
    // minimp3
    struct MiniMp3 : mp3dec_t {
        // object
        PLAYAU_OBJ;
    };
    /// <summary>
    /// mp3 stream
    /// </summary>
    /// <seealso cref="XAUAudioStream" />
    struct CAUMp3AudioStream final : XAUAudioStream {
    public:
        // ctor
        CAUMp3AudioStream() noexcept;
        // ok
        //bool IsOK() const noexcept { return !!m_pMp3File; }
        // dispose
        void Dispose() noexcept override;
        // seek stream in byte, return current position
        bool Seek(int32_t off, Move method = XAUStream::Move_Begin) noexcept override;
        // read stream, return byte count read
        auto ReadNext(uint32_t len, void* buf) noexcept->uint32_t override;
        // move to new position
        void MoveTo(void* target) noexcept override;
    private:
        // mp3 object
        MiniMp3*                m_pMp3Obj = new(std::nothrow) MiniMp3;
    };
    /// <summary>
    /// Creates the mp3 audio stream.
    /// </summary>
    /// <param name="file">The file.</param>
    /// <param name="buf">The buf.</param>
    /// <returns></returns>
    bool CreateMp3AudioStream(XAUStream & file, void* buf) noexcept {
        constexpr size_t sizeof_mp3s = sizeof(CAUMp3AudioStream);
        constexpr size_t sizeof_bufl = AUDIO_STREAM_BUFLEN;
        static_assert(sizeof_mp3s <= sizeof_bufl, "overflow");
        const auto obj = new(buf) CAUMp3AudioStream;
        mp3dec_t mp3;
        //return obj->IsOK();
        return true;
    }
}


/// <summary>
/// Moves to.
/// </summary>
/// <param name="target">The target.</param>
/// <returns></returns>
void PlayAU::CAUMp3AudioStream::MoveTo(void* target) noexcept {
    std::memcpy(target, this, sizeof(*this));
    const auto tar = reinterpret_cast<CAUMp3AudioStream*>(target);
    const auto target_stream = tar->FileStream();
    this->FileStream()->MoveTo(target_stream);
#ifndef NDEBUG
    std::memset(this, 0, sizeof(*this));
#endif
}

/// <summary>
/// Initializes a new instance of the <see cref="CAUMp3AudioStream"/> struct.
/// </summary>
PlayAU::CAUMp3AudioStream::CAUMp3AudioStream() noexcept {
    if (!m_pMp3Obj) return;
    ::mp3dec_init(m_pMp3Obj);
}

/// <summary>
/// Releases unmanaged and - optionally - managed resources.
/// </summary>
/// <returns></returns>
void PlayAU::CAUMp3AudioStream::Dispose() noexcept {
    this->DisposeFS();
}

/// <summary>
/// Seeks the specified off.
/// </summary>
/// <param name="off">The off.</param>
/// <param name="method">The method.</param>
/// <returns></returns>
bool PlayAU::CAUMp3AudioStream::Seek(int32_t off, Move method) noexcept {

    return true;
}


/// <summary>
/// Reads the next.
/// </summary>
/// <param name="len">The length.</param>
/// <param name="buf">The buf.</param>
/// <returns></returns>
auto PlayAU::CAUMp3AudioStream::ReadNext(uint32_t len, void* buf) noexcept -> uint32_t {
    return 0;
}
#endif