#pragma once
// License: MIT  http://opensource.org/licenses/MIT
// Author: dustpg   mailto:dustpg@gmail.com


#include "au_base.h"
#include "au_config.h"

namespace PlayAU {
    // audio api
    struct IAUAudioAPI;
    /// <summary>
    /// Result code
    /// </summary>
    class PLAYAU_API CAUAudioGroup {
    protected:
        // ctor
        CAUAudioGroup(IAUAudioAPI&) noexcept;
        // dtor
        ~CAUAudioGroup() noexcept;
    public:
        // get name
        auto GetName() const noexcept { return m_name; }
        // [nullsafe] get volume
        auto GetVolume() const noexcept ->float;
    public:
        // [nullsafe] get volume
        void SetVolume(float) noexcept;
    private:
        // name of group
        char            m_name[MAX_GROUP_NAME_LENGTH];
        // audio engine
        IAUAudioAPI&    m_api;
    };
}