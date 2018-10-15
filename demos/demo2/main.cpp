#define _CRT_SECURE_NO_WARNINGS
#include "../../inc/playau.h"
#include "../../inc/au_clip.h"
#include "../../inc/au_group.h"

#include <cstdio>
#include <Windows.h>

#pragma comment(lib, "playau.lib")
#pragma comment(lib, "libFLAC_static_win32d.lib")

void window(HWND*) noexcept;

enum : UINT {
    POST_CONTEXT = WM_USER + 10
};


namespace PlayAU {
    // demo2 config
    struct CAUDemo2Config final : IAUConfigure {
        // hwnd
        HWND            hwnd = nullptr;
        // pick the device
        auto PickDevice(char16_t buf[256]) noexcept -> const char16_t* override {
            return nullptr;
        }
        // call context
        void CallContext(CAUEngine& engine, void* ctx1, void* ctx2) noexcept {
            ::PostMessageW(hwnd, POST_CONTEXT, (WPARAM)ctx1, (LPARAM)ctx2);
        }
    };
}

PlayAU::CAUEngine* g_engine = nullptr;

int main() noexcept  {
    // DPIAware
    ::SetProcessDPIAware();
    PlayAU::CAUDemo2Config config;
    PlayAU::CAUEngine engine; g_engine = &engine;
    ::CoInitialize(nullptr);
    //{
        char16_t buf[1024]; PlayAU::AudioDeviceInfo infos[64];
        PlayAU::API::EnumDevices(buf, infos, 1024, 64);
    //}
    if (engine.Initialize(&config)) {
        //void test(); test();

        if (const auto file = std::fopen("../../../Doc/Castle_in_the_Sky.dat", "rb")) {

            const auto clip = engine.CreateLiveClip(
                { 44100, 32, 2, PlayAU::Wave_IEEEFloat }
            );
            constexpr uint32_t len = 1024 * 8;
            float buffer[len * 4];
            int index = 0;
            bool flag = true;
            while (flag) {
                ::Sleep(20);
                if (clip->GetLiveBufferLeft()) continue;
                const auto buf = buffer + index * len;
                flag = std::fread(buf, sizeof(float), len, file) == len;
                index = (index + 1) & 3;
                clip->SunmitRefableLiveBuffer(reinterpret_cast<uint8_t*>(buf), sizeof(float) * len);
            }
            std::fclose(file);
            std::getchar();
            clip->Destroy();
        }

        const auto clip = engine.CreateClipFromFile(
            PlayAU::Flag_None,
            //PlayAU::Flag_AutoDestroyOnEnd,
            //u"../../audiofiledemo/Hymn_of_ussr_instrumental.ogg"
            //u"../../../Doc/Sakura.ogg"
            u"../../../Doc/Castle_in_the_Sky.ogg"
            //u"../../../Doc/Castle_in_the_Sky.flac"
            //u"../../../Doc/ghost.flac"
            ,
            "BGM"
        );
        const auto dur = clip->Duration();
        clip->Seek(dur-10);
        int bk = 9;

        clip->Play();
        clip->SetLoop(true);

        clip->SetFrequencyRatio(1.25f);

        const auto group = engine.FindGroup("BGM");

        window(&config.hwnd);

        // engine.Uninitialize will release
        //clip->Destroy();

        engine.Uninitialize();
    }
    ::CoUninitialize();
    return 0;
}

enum { WINDOW_WIDTH = 1280, WINDOW_HEIGHT = 720 };
static const wchar_t WINDOW_TITLE[] = L"PlayAU Draw";
//static bool doit = true;

LRESULT CALLBACK ThisWndProc(HWND, UINT, WPARAM, LPARAM) noexcept;
void DoRender(uint32_t sync) noexcept;
bool InitD3D(HWND) noexcept;
void ClearD3D() noexcept;

void window(HWND* out) noexcept {
    // 注册窗口
    WNDCLASSEXW wcex = { sizeof(WNDCLASSEXW) };
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = ThisWndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = sizeof(LONG_PTR);
    wcex.hInstance = ::GetModuleHandleW(nullptr);
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = nullptr;
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = L"DemoWindowClass";
    wcex.hIcon = nullptr;
    ::RegisterClassExW(&wcex);
    // 计算窗口大小
    RECT window_rect = { 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT };
    DWORD window_style = WS_OVERLAPPEDWINDOW;
    AdjustWindowRect(&window_rect, window_style, FALSE);
    window_rect.right -= window_rect.left;
    window_rect.bottom -= window_rect.top;
    window_rect.left = (::GetSystemMetrics(SM_CXFULLSCREEN) - window_rect.right) / 2;
    window_rect.top = (::GetSystemMetrics(SM_CYFULLSCREEN) - window_rect.bottom) / 2;
    // 创建窗口
    const auto hwnd = ::CreateWindowExW(
        0,
        wcex.lpszClassName, WINDOW_TITLE, window_style,
        window_rect.left, window_rect.top, window_rect.right, window_rect.bottom,
        0, 0, ::GetModuleHandleW(nullptr), nullptr
    );
    if (!hwnd) return;
    *out = hwnd;
    ::ShowWindow(hwnd, SW_NORMAL);
    ::UpdateWindow(hwnd);
    if (::InitD3D(hwnd)) {
        MSG msg = { 0 };
        while (msg.message != WM_QUIT) {
            // 获取消息
            if (::PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
                ::TranslateMessage(&msg);
                ::DispatchMessageW(&msg);
            }
            else DoRender(1);
        }
    }
    ::ClearD3D();
    return;
}

void DoRender(uint32_t sync) noexcept {
    ::Sleep(20);
}

bool InitD3D(HWND) noexcept {

    return true;
}

void ClearD3D() noexcept {

}


LRESULT CALLBACK ThisWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept {
    switch (msg)
    {
    case WM_CLOSE:
        ::DestroyWindow(hwnd);
        return 0;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    case POST_CONTEXT:
        g_engine->CallContext((void*)wParam, (void*)lParam);
        return 0;
    }
    return ::DefWindowProcW(hwnd, msg, wParam, lParam);
}