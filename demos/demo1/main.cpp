#include "../../inc/playau.h"
#include "../../inc/au_clip.h"


#include <cstdio>

#pragma comment(lib, "playau.lib")


extern "C" void __stdcall Sleep(uint32_t) noexcept;
extern "C" long __stdcall CoInitialize(void*) noexcept;
extern "C" void __stdcall CoUninitialize() noexcept;

int main() {
    PlayAU::CAUEngine engine;
    ::CoInitialize(nullptr);
    //{
        char16_t buf[1024]; PlayAU::AudioDeviceInfo infos[64];
        PlayAU::API::EnumDevices(buf, infos, 1024, 64);
    //}
    if (engine.Initialize()) {
        const auto clip = engine.CreateClipFromFile(
            PlayAU::Flag_None, 
            u"../../audiofiledemo/Hymn_of_ussr_instrumental.ogg"
        );

        clip->Play();
        for (int i = 0; i != 500; ++i) {
            const auto dur = clip->Tell();
            std::printf("%f\n", dur);
            ::Sleep(500);
            switch (i)
            {
            case 5:
            case 15:
                clip->Stop();
                break;
            case 9:
            case 19:
                clip->Play();
                break;
            }
        }



        std::getchar();
        clip->Pause();
        std::getchar();
        clip->Play();
        std::getchar();
        ::Sleep(1000);
        clip->Destroy();
        engine.Uninitialize();
    }
    ::CoUninitialize();
    return 0;
}