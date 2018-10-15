#pragma once
// License: MIT  http://opensource.org/licenses/MIT
// Author: dustpg   mailto:dustpg@gmail.com

#include "au_base.h"
#include "au_config.h"
#include <cstdint>

namespace PlayAU {
    // Audio Engine
    class CAUEngine;
    // group
    class CAUAudioGroup;
    // audio stream
    struct XAUAudioStream;
    // private clip data
    class PLAYAU_API CAUAudioClip {
        // friend
        friend CAUEngine;
    public:
        // obj
        PLAYAU_OBJ;
        // private
        struct Private;
        // ctor
        CAUAudioClip(CAUEngine&, ClipFlag, XAUAudioStream&&, CAUAudioGroup*) noexcept;
        // [nullsafe] destroy this
        void Destroy() noexcept;
        // [nullsafe] play this
        void Play() noexcept;
        // [nullsafe] pause this
        void Pause() noexcept;
        // [nullsafe] stop this
        void Stop() noexcept;
        // [nullsafe] seek in sec.
        void Seek(double pos) noexcept;
        // [nullsafe] tell position
        auto Tell() const noexcept ->double;
        // [nullsafe] get duration
        auto Duration() const noexcept ->double;
        // [nullsafe] set volume
        void SetVolume(float v) noexcept;
        // [nullsafe] set frequency ratio
        void SetFrequencyRatio(float f) noexcept;
        // [nullsafe] get volume
        auto GetVolume() const noexcept ->float;
        // [nullsafe] get frequency ratio
        auto GetFrequencyRatio() const noexcept ->float;
    public:
        // [nullsafe] get live buffer left
        auto GetLiveBufferLeft() const noexcept->uint32_t;
        // [nullsafe] submit ref-able live buffer
        void SunmitRefableLiveBuffer(uint8_t*, uint32_t) noexcept;
    public:
        // [nullsafe] set loop
        void SetLoop(bool) noexcept;
    private:
        // context
        uintptr_t                   m_context[AUDIO_CTX_BUFLEN];
        // flag
        ClipFlag        const       m_flags;
        // state: playing
        bool                        m_playing = false;
        // state: pausing
        bool                        m_pausing = false;
    public:
        // group
        CAUAudioGroup*     const    group;
    private:
        // node
        Node                        m_node;
        // audio engine
        CAUEngine&                  m_engine;
        // audio stream
        char                        m_asbuffer[AUDIO_STREAM_BUFLEN];
    private:
        // no copy
        CAUAudioClip(const CAUAudioClip&) noexcept = delete;
        // no move
        CAUAudioClip(CAUAudioClip&&) noexcept = delete;
        // dtor
        ~CAUAudioClip() noexcept;
    };
}