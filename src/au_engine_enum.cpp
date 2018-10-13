#include "../inc/playau.h"
#include <cassert>
#include <cwchar>
#include <cstring>

#include <mmdeviceapi.h>

// PLAYAU: DEFINE_GUID
#define PLAYAU_DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
    const GUID name = { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }

// PLAYAU: DEFINE_PROPERTYKEY
#define PlayAU_DEFINE_PROPERTYKEY(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8, pid) \
    const PROPERTYKEY name = { { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }, pid }

namespace PlayAU {
    // CLSID_MMDeviceEnumerator
    PLAYAU_DEFINE_GUID(CLSID_MMDeviceEnumerator, 0xBCDE0395, 0xE52F, 0x467C,
        0x8E, 0x3D, 0xC4, 0x57, 0x92, 0x91, 0x69, 0x2E);
    // IID_IMMDeviceEnumerator
    PLAYAU_DEFINE_GUID(IID_IMMDeviceEnumerator, 0xA95664D2, 0x9614, 0x4F35,
        0xA7, 0x46, 0xDE, 0x8D, 0xB6, 0x36, 0x17, 0xE6);
    // PKEY_Device_FriendlyName
    PlayAU_DEFINE_PROPERTYKEY(PKEY_Device_FriendlyName, 0xa45c254e, 0xdf1c, 0x4efd,
        0x80, 0x20, 0x67, 0xd1, 0x46, 0xa8, 0x50, 0xe0, 14);
    // PKEY_AudioEndpoint_Path
    PlayAU_DEFINE_PROPERTYKEY(PKEY_AudioEndpoint_Path, 0x9c119480, 0xddc2, 0x4954,
        0xa1, 0x50, 0x5b, 0xd2, 0x40, 0xd4, 0x54, 0xad, 1);
}


/// <summary>
/// Enums the devices.
/// </summary>
/// <param name="buf">The buf.</param>
/// <param name="infos">The infos.</param>
/// <param name="buflen">The buflen.</param>
/// <param name="infolen">The infolen.</param>
/// <returns></returns>
auto PlayAU::API::EnumDevices(
    char16_t buf[], 
    AudioDeviceInfo infos[], 
    uint32_t buflen, 
    uint32_t infolen) noexcept -> uint32_t {
    // 枚举输出
    HRESULT hr = S_OK;
    IMMDeviceEnumerator* enumerator = nullptr;
    IMMDeviceCollection* devices = nullptr;
    UINT device_count = 0;
    uint32_t rv = 0;
    // 获取枚举器
    if (SUCCEEDED(hr)) {
        hr = ::CoCreateInstance(
            CLSID_MMDeviceEnumerator,
            nullptr,
            CLSCTX_ALL,
            IID_IMMDeviceEnumerator,
            reinterpret_cast<void**>(&enumerator)
        );
    }
    // 获取输出设备集合
    if (SUCCEEDED(hr)) {
        hr = enumerator->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &devices);
    }
    // 获取输出设备数量
    if (SUCCEEDED(hr)) {
        hr = devices->GetCount(&device_count);
    }
    // 获取设备信息
    if (SUCCEEDED(hr)) {
        // 钳制
        const uint32_t count
            = device_count > infolen
            ? infolen
            : device_count
            ;
        auto buf_write = buf;
        for (uint32_t i = 0; i != count; ++i) {
            IPropertyStore *propStore = nullptr;
            IMMDevice* device = nullptr;
            PROPVARIANT name, id;
            bool break_here = false;
            ::PropVariantInit(&name);
            ::PropVariantInit(&id);
            // 获取设备
            hr = devices->Item(i, &device);
            // 信息商店
            if (SUCCEEDED(hr)) {
                hr = device->OpenPropertyStore(STGM_READ, &propStore);
            }
            // 路径名称
            if (SUCCEEDED(hr)) {
                hr = propStore->GetValue(PKEY_AudioEndpoint_Path, &id);
            }
            // 友好名称
            if (SUCCEEDED(hr)) {
                hr = propStore->GetValue(PKEY_Device_FriendlyName, &name);
            }
            // 错误中断
            if (FAILED(hr)) break_here = true;
            else {
                // 断言检查
                assert(id.vt == VT_LPWSTR && name.vt == VT_LPWSTR);
                // 输入统一接口
                const uint32_t len1 = std::wcslen(id.pwszVal);
                const uint32_t len2 = std::wcslen(name.pwszVal);
                const uint32_t len = len1 + len2 + 2;
                static_assert(sizeof(id.pwszVal[0]) == sizeof(char16_t), "");
                if (buflen >= len) {
                    std::memcpy(buf_write, id.pwszVal, sizeof(char16_t) * (len1 + 1));
                    std::memcpy(buf_write + len1 + 1, name.pwszVal, sizeof(char16_t) * (len2 + 1));
                    infos[rv].id = buf_write;
                    infos[rv].name = buf_write + len1 + 1;
                    ++rv;
                    buf_write += len;
                    buflen -= len;
                }
                else break_here;
            }
            // 扫尾
            PlayAU::SafeRelease(propStore);
            PlayAU::SafeRelease(device);
            ::PropVariantClear(&name);
            ::PropVariantClear(&id);
            if (break_here) break;
        }
    }
    PlayAU::SafeRelease(enumerator);
    PlayAU::SafeRelease(devices);
    return rv;
}