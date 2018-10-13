#include "../inc/playau.h"
#include "../inc/au_group.h"
#include "private/p_au_engine_interface.h"

#include <cstring>

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
    static auto API(CAUEngine& engine) noexcept {
        return reinterpret_cast<IAUAudioAPI*>(engine.m_buffer); }
    // get api
    static auto Group(CAUEngine& engine) noexcept {
        return engine.m_group; }
};

namespace PlayAU {
    /// <summary>
    /// Disposes the groups.
    /// </summary>
    /// <param name="engine">The engine.</param>
    /// <returns></returns>
    void DisposeGroups(CAUEngine& engine) noexcept {
        const auto api = CAUEngine::Private::API(engine);
        const auto base = CAUEngine::Private::Group(engine);
        for (uint32_t i = 0; i != MAX_GROUP_COUNT; ++i) {
            const auto ptr = base + GROUP_BUFLEN_BYTE * i;
            const auto obj = reinterpret_cast<CAUAudioGroup*>(ptr);
            if (!obj->GetName()[0]) break;
            api->DisposeGroup(*obj);
        }
    }
}


/// <summary>
/// Finds the group.
/// </summary>
/// <param name="name">The name.</param>
/// <returns></returns>
auto PlayAU::CAUEngine::FindGroup(const char name[]) noexcept -> CAUAudioGroup* {
    if (!name || !name[0]) return nullptr;
    const auto base = m_group;
    for (uint32_t i = 0; i != MAX_GROUP_COUNT; ++i) {
        const auto ptr = base + GROUP_BUFLEN_BYTE * i;
        const auto obj = reinterpret_cast<CAUAudioGroup*>(ptr);
        const auto onm = obj->GetName();
        if (!onm[0]) break;
        if (!std::strcmp(onm, name)) return obj;
    }
    return nullptr;
}

/// <summary>
/// Creates the empty group.
/// </summary>
/// <param name="name">The name.</param>
/// <returns></returns>
auto PlayAU::CAUEngine::CreateEmptyGroup(const char name[]) noexcept -> CAUAudioGroup* {
    if (!name || !name[0]) return nullptr;
    static_assert(GROUP_BUFLEN_BYTE % sizeof(void*) == 0, "aligned");
    const auto base = m_group;
    CAUAudioGroup* rv = nullptr;
    for (uint32_t i = 0; i != MAX_GROUP_COUNT; ++i) {
        const auto ptr = base + GROUP_BUFLEN_BYTE * i;
        const auto obj = reinterpret_cast<CAUAudioGroup*>(ptr);
        if (*obj->GetName()) continue;
        if (const auto ok = Private::API(*this)->CreateGroup(*obj)) {
            const size_t len = MAX_GROUP_NAME_LENGTH - 1;
            std::strncpy(const_cast<char*>(obj->GetName()), name, len);
            rv = obj;
        }
        break;
    }
    return rv;
}


/// <summary>
/// Initializes a new instance of the <see cref="CAUAudioGroup" /> class.
/// </summary>
/// <param name="api">The API.</param>
PlayAU::CAUAudioGroup::CAUAudioGroup(IAUAudioAPI& api) noexcept : m_api(api) {

}

/// <summary>
/// Finalizes an instance of the <see cref="CAUAudioGroup"/> class.
/// </summary>
/// <returns></returns>
PlayAU::CAUAudioGroup::~CAUAudioGroup() noexcept {

}

/// <summary>
/// Gets the volume.
/// </summary>
/// <returns></returns>
auto PlayAU::CAUAudioGroup::GetVolume() const noexcept -> float {
    PLAYAU_NULL_RETURN(0.f);
    const auto this_ptr = const_cast<CAUAudioGroup*>(this);
    return m_api.VolumeGroup(*this_ptr, nullptr);
}

/// <summary>
/// Sets the volume.
/// </summary>
/// <param name="vol">The vol.</param>
/// <returns></returns>
void PlayAU::CAUAudioGroup::SetVolume(float vol) noexcept {
    PLAYAU_NULL_RETURN((void)0);
    m_api.VolumeGroup(*this, &vol);
}