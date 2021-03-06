#include "private/p_au_engine_interface.h"
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <cassert>
#include <cstring>


namespace PlayAU {
    /// <summary>
    /// windows file stream
    /// </summary>
    struct CAUWinFileStream final : XAUStream {
        // ok
        bool IsOK() const noexcept { return m_hFile != INVALID_HANDLE_VALUE; }
        // dispose
        void Dispose() noexcept override;
        // seek stream in byte, return current position
        bool Seek(int32_t off, Move method = XAUStream::Move_Begin) noexcept override;
        // read stream, return byte count read
        auto ReadNext(uint32_t len, void* buf) noexcept->uint32_t override;
        // move to new position
        void MoveTo(void* target) noexcept override;
        // ctor
        CAUWinFileStream(const char16_t* filename) noexcept;
    private:
        // file handle
        HANDLE          m_hFile = INVALID_HANDLE_VALUE;
    };
    // create windows file stream
    bool CreateWinFileStream(void* buf, const char16_t file[]) noexcept {
        constexpr size_t buflen = FILE_STREAM_BUFLEN * sizeof(void*);
        static_assert(sizeof(CAUWinFileStream) <= buflen, "overflow");
        const auto obj = new(buf) CAUWinFileStream{ file };
        return obj->IsOK();
    }
}

/// <summary>
/// Moves to.
/// </summary>
/// <param name="target">The target.</param>
/// <returns></returns>
void PlayAU::CAUWinFileStream::MoveTo(void* target) noexcept {
    std::memcpy(target, this, sizeof(*this));
#ifndef NDEBUG
    std::memset(this, 0, sizeof(*this));
#endif
}

/// <summary>
/// Initializes a new instance of the <see cref="CAUWinFileStream"/> struct.
/// </summary>
/// <param name="filename">The filename.</param>
PlayAU::CAUWinFileStream::CAUWinFileStream(const char16_t* filename) noexcept :XAUStream() {
    assert(filename && "bad argument");
    static_assert(sizeof(char16_t) == sizeof(wchar_t), "same!");
    m_hFile = ::CreateFileW(
        reinterpret_cast<const wchar_t*>(filename),
        GENERIC_READ,
        FILE_SHARE_READ,
        nullptr,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );
    if (m_hFile == INVALID_HANDLE_VALUE) return;
    const uint32_t len = ::GetFileSize(m_hFile, nullptr);
    this->length = len;
}



/// <summary>
/// Releases unmanaged and - optionally - managed resources.
/// </summary>
/// <returns></returns>
void PlayAU::CAUWinFileStream::Dispose() noexcept {
    if (m_hFile == INVALID_HANDLE_VALUE) return;
    ::CloseHandle(m_hFile);
    m_hFile = INVALID_HANDLE_VALUE;
    this->length = 0;
    this->offset = 0;
}


/// <summary>
/// Seeks the specified off.
/// </summary>
/// <param name="off">The off.</param>
/// <param name="method">The method.</param>
/// <returns></returns>
bool PlayAU::CAUWinFileStream::Seek(int32_t off, Move method) noexcept {
    assert(this->IsOK());
    const auto rv = ::SetFilePointer(m_hFile, off, nullptr, method);
    this->offset = rv;
    return rv != INVALID_SET_FILE_POINTER;
}

/// <summary>
/// Reads the next.
/// </summary>
/// <param name="len">The length.</param>
/// <param name="buf">The buf.</param>
/// <returns></returns>
auto PlayAU::CAUWinFileStream::ReadNext(uint32_t len, void * buf) noexcept -> uint32_t {
    assert(this->IsOK());
    DWORD read = 0;
    ::ReadFile(m_hFile, buf, len, &read, nullptr);
    this->offset += read;
    return read;
}
