#pragma once
// License: MIT  http://opensource.org/licenses/MIT
// Author: dustpg   mailto:dustpg@gmail.com

#include "au_base.h"
#include "au_config.h"
#include <cstdint>

namespace PlayAU {
    // Audio Engine
    class CAUEngine;
    // audio stream
    struct XAUAudioStream;
    // private clip data
    class PLAYAU_API CAUAudioClip {
        // friend
        friend CAUEngine;
        // obj
        PLAYAU_OBJ;
    public:
        // private
        struct Private;
        // ctor
        CAUAudioClip(CAUEngine&, ClipFlag, XAUAudioStream&&) noexcept;
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
        // set volume
        void SetVolume(float v) noexcept;
        // set frequency ratio
        void SetFrequencyRatio(float f) noexcept;
        // get volume
        auto GetVolume() const noexcept ->float;
        // get frequency ratio
        auto GetFrequencyRatio() const noexcept ->float;
    private:
        // context
        uintptr_t                   m_context[AUDIO_CTX_BUFLEN];
        // flag
        ClipFlag        const       m_flags;
        // state: playing
        bool                        m_playing = false;
        // state: pausing
        bool                        m_pausing = false;
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