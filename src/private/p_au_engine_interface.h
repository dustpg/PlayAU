#pragma once

// 检查
#ifndef PLAYAU_NOVTABLE
#   if (_MSC_VER >= 1100) && defined(__cplusplus)
#       define PLAYAU_NOVTABLE __declspec(novtable)
#   else
#       define PLAYAU_NOVTABLE
#   endif
#endif

#include <cstdint>
#include "../../inc/au_base.h"

namespace PlayAU {
    // base interface
    struct PLAYAU_NOVTABLE IAUBase {
        // dispose
        virtual void Dispose() noexcept = 0;
    };
    // interface for audio api
    struct PLAYAU_NOVTABLE IAUAudioAPI : IAUBase {
        // suspend
        virtual void Suspend() noexcept = 0;
        // resume
        virtual void Resume() noexcept = 0;
        // make clip context
        virtual bool MakeClipCtx(void*) noexcept = 0;
        // dispose clip context
        virtual void DisposeClipCtx(void*) noexcept = 0;
        // tell clip context
        virtual auto TellClip(const void*) noexcept -> uint32_t = 0;
        // play clip context
        virtual void PlayClip(void*) noexcept = 0;
        // pause clip context
        virtual void PauseClip(void*) noexcept = 0;
        // stop clip context
        virtual void StopClip(void*) noexcept = 0;
        // seek clip in byte
        virtual void SeekClip(void*, uint32_t) noexcept = 0;
        // ratio clip context
        virtual auto RatioClip(void*, float*) noexcept -> float = 0;
        // volume clip context
        virtual auto VolumeClip(void*, float*) noexcept -> float = 0;
    };
    // Stream Interface for read less than 4GB stream
    struct PLAYAU_NOVTABLE IAUStream : IAUBase {
        // method to move
        enum Move : uint32_t { Move_Begin = 0, Move_Current, Move_End };
        // seek stream in byte, return current position
        virtual auto Seek(int32_t off, Move method = IAUStream::Move_Begin) noexcept->uint32_t = 0;
        // read stream, return byte count read
        virtual auto ReadNext(uint32_t len, void* buf) noexcept->uint32_t = 0;
        // move to new positon
        virtual void MoveTo(void* target) noexcept = 0;
        // tell position
        auto Tell() noexcept { return this->Seek(int32_t(0), IAUStream::Move_Current); }
    };
    // interface for audio stream
    struct PLAYAU_NOVTABLE XAUAudioStream : IAUStream {
        // wave format
        WaveFormat          format;
        // total length
        uint32_t            length;
        // file stream buffer
        uintptr_t           fsbuffer[FILE_STREAM_BUFLEN];
        // file stream
        auto FileStream() noexcept { return reinterpret_cast<IAUStream*>(fsbuffer); }
    };
}