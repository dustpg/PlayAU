#pragma once

// 检查
#ifndef PLAYAU_NOVTABLE
#if defined(_MSC_VER) && defined(__cplusplus)
#define PLAYAU_NOVTABLE __declspec(novtable)
#else
#define PLAYAU_NOVTABLE
#endif
#endif

#include <cstdint>
#include "../../inc/au_base.h"

// readonly, write by self
#define pconst

namespace PlayAU {
    // group
    class CAUAudioGroup;
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
        // call context
        virtual void CallContext(void* ctx1, void* ctx2) noexcept = 0;
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
        // live: buffer left
        virtual auto LiveClipBuffer(void*) noexcept->uint32_t = 0;
        // live: buffer submit
        virtual void LiveClipSubmit(void*, void*, uint32_t) noexcept = 0;
        // create group
        virtual bool CreateGroup(CAUAudioGroup&) noexcept = 0;
        // dispose group
        virtual void DisposeGroup(CAUAudioGroup&) noexcept = 0;
        // volume group
        virtual auto VolumeGroup(CAUAudioGroup&, float*) noexcept -> float = 0;
    };
    // Stream Interface for read less than 4GB stream
    struct PLAYAU_NOVTABLE XAUStream : IAUBase {
        // method to move
        enum Move : uint32_t { Move_Begin = 0, Move_Current, Move_End };
        // read stream, return byte count read
        virtual auto ReadNext(uint32_t len, void* buf) noexcept->uint32_t = 0;
        // seek stream in byte, if successful, return true
        virtual bool Seek(int32_t off, Move method = XAUStream::Move_Begin) noexcept = 0;
        // move this to new positon (&&)
        virtual void MoveTo(void* target) noexcept = 0;
        // total length
        uint32_t     pconst length;
        // curret offset, EOF/EOS if greater or eql to @length
        uint32_t     pconst offset;
    };
    // interface for audio stream
    struct PLAYAU_NOVTABLE XAUAudioStream : XAUStream {
        // wave format
        WaveFormat   pconst format;
        // file stream buffer
        uintptr_t    pconst fsbuffer[FILE_STREAM_BUFLEN];
        // file stream
        auto FileStream() noexcept { return reinterpret_cast<XAUStream*>(fsbuffer); }
        // dispose file stream
        void DisposeFS() noexcept { FileStream()->Dispose(); }
    };
}