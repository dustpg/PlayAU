#pragma once
#include "p_XAudio2_base.h"

namespace XAudio2 { namespace Ver2_8 {
    
    // interface list
    struct IXAudio2;
    struct IXAudio2Voice;
    struct IXAudio2SourceVoice;
    struct IXAudio2SubmixVoice;
    struct IXAudio2MasteringVoice;
    struct IXAudio2EngineCallback;
    struct IXAudio2VoiceCallback;
#pragma pack(push, 1)

    // Used in XAUDIO2_VOICE_SENDS below
    struct XAUDIO2_SEND_DESCRIPTOR {
        UINT32 Flags;
        IXAudio2Voice* pOutputVoice;
    };

    // Used in the voice creation functions and in IXAudio2Voice::SetOutputVoices
    struct XAUDIO2_VOICE_SENDS {
        UINT32 SendCount;
        XAUDIO2_SEND_DESCRIPTOR* pSends;
    };
#pragma pack(pop)
    // IXAudio2
    interface IXAudio2 : IUnknown {
        //STDMETHOD(QueryInterface) (REFIID riid,  void** ppvInterface) PURE;
        //STDMETHOD_(ULONG, AddRef) () PURE;
        //STDMETHOD_(ULONG, Release) () PURE;
        STDMETHOD(RegisterForCallbacks) (IXAudio2EngineCallback* pCallback) PURE;
        STDMETHOD_(void, UnregisterForCallbacks) (IXAudio2EngineCallback* pCallback) PURE;

        STDMETHOD(CreateSourceVoice) (IXAudio2SourceVoice** ppSourceVoice,
            const WAVEFORMATEX* pSourceFormat,
            UINT32 Flags = (0),
            float MaxFrequencyRatio = (XAUDIO2_DEFAULT_FREQ_RATIO),
            IXAudio2VoiceCallback* pCallback = (NULL),
            const XAUDIO2_VOICE_SENDS* pSendList = (NULL),
            const XAUDIO2_EFFECT_CHAIN* pEffectChain = (NULL)) PURE;

        STDMETHOD(CreateSubmixVoice) (IXAudio2SubmixVoice** ppSubmixVoice,
            UINT32 InputChannels, UINT32 InputSampleRate,
            UINT32 Flags = (0), UINT32 ProcessingStage = (0),
            const XAUDIO2_VOICE_SENDS* pSendList = (NULL),
            const XAUDIO2_EFFECT_CHAIN* pEffectChain = (NULL)) PURE;

        STDMETHOD(CreateMasteringVoice) (IXAudio2MasteringVoice** ppMasteringVoice,
            UINT32 InputChannels = (XAUDIO2_DEFAULT_CHANNELS),
            UINT32 InputSampleRate = (XAUDIO2_DEFAULT_SAMPLERATE),
            UINT32 Flags = (0), LPCWSTR szDeviceId = (NULL),
            const XAUDIO2_EFFECT_CHAIN* pEffectChain = (NULL),
            AUDIO_STREAM_CATEGORY StreamCategory = (AudioCategory_GameEffects)) PURE;

        STDMETHOD(StartEngine) () PURE;
        STDMETHOD_(void, StopEngine) () PURE;
        STDMETHOD(CommitChanges) (UINT32 OperationSet) PURE;
        STDMETHOD_(void, GetPerformanceData) (XAUDIO2_PERFORMANCE_DATA* pPerfData) PURE;

        STDMETHOD_(void, SetDebugConfiguration) (const XAUDIO2_DEBUG_CONFIGURATION* pDebugConfiguration,
            void* pReserved = (NULL)) PURE;
    };


    // IXAudio2Voice
    interface IXAudio2Voice {
        STDMETHOD_(void, GetVoiceDetails) (XAUDIO2_VOICE_DETAILS* pVoiceDetails) PURE;
        STDMETHOD(SetOutputVoices) (const XAUDIO2_VOICE_SENDS* pSendList) PURE;
        STDMETHOD(SetEffectChain) (const XAUDIO2_EFFECT_CHAIN* pEffectChain) PURE;
        STDMETHOD(EnableEffect) (UINT32 EffectIndex,
            UINT32 OperationSet = (XAUDIO2_COMMIT_NOW)) PURE;
        STDMETHOD(DisableEffect) (UINT32 EffectIndex,
            UINT32 OperationSet = (XAUDIO2_COMMIT_NOW)) PURE;
        STDMETHOD_(void, GetEffectState) (UINT32 EffectIndex, BOOL* pEnabled) PURE;
        STDMETHOD(SetEffectParameters) (UINT32 EffectIndex,
            const void* pParameters,
            UINT32 ParametersByteSize,
            UINT32 OperationSet = (XAUDIO2_COMMIT_NOW)) PURE;
        STDMETHOD(GetEffectParameters) (UINT32 EffectIndex,
            void* pParameters,
            UINT32 ParametersByteSize) PURE;
        STDMETHOD(SetFilterParameters) (const XAUDIO2_FILTER_PARAMETERS* pParameters,
            UINT32 OperationSet = (XAUDIO2_COMMIT_NOW)) PURE;
        STDMETHOD_(void, GetFilterParameters) (XAUDIO2_FILTER_PARAMETERS* pParameters) PURE;
        STDMETHOD(SetOutputFilterParameters) (IXAudio2Voice* pDestinationVoice,
            const XAUDIO2_FILTER_PARAMETERS* pParameters,
            UINT32 OperationSet = (XAUDIO2_COMMIT_NOW)) PURE;
        STDMETHOD_(void, GetOutputFilterParameters) (IXAudio2Voice* pDestinationVoice,
            XAUDIO2_FILTER_PARAMETERS* pParameters) PURE;
        STDMETHOD(SetVolume) (float Volume,
            UINT32 OperationSet = (XAUDIO2_COMMIT_NOW)) PURE;
        STDMETHOD_(void, GetVolume) (float* pVolume) PURE;
        STDMETHOD(SetChannelVolumes) (UINT32 Channels, const float* pVolumes,
            UINT32 OperationSet = (XAUDIO2_COMMIT_NOW)) PURE;
        STDMETHOD_(void, GetChannelVolumes) (UINT32 Channels, float* pVolumes) PURE;
        STDMETHOD(SetOutputMatrix) (IXAudio2Voice* pDestinationVoice,
            UINT32 SourceChannels, UINT32 DestinationChannels,
            const float* pLevelMatrix,
            UINT32 OperationSet = (XAUDIO2_COMMIT_NOW)) PURE;
        STDMETHOD_(void, GetOutputMatrix) (IXAudio2Voice* pDestinationVoice,
            UINT32 SourceChannels, UINT32 DestinationChannels,
            float* pLevelMatrix) PURE;
        STDMETHOD_(void, DestroyVoice) () PURE;
    };

    // IXAudio2SourceVoice

    interface IXAudio2SourceVoice : IXAudio2Voice {
        STDMETHOD(Start) (UINT32 Flags = (0), UINT32 OperationSet = (XAUDIO2_COMMIT_NOW)) PURE;
        STDMETHOD(Stop) (UINT32 Flags = (0), UINT32 OperationSet = (XAUDIO2_COMMIT_NOW)) PURE;
        STDMETHOD(SubmitSourceBuffer) (const XAUDIO2_BUFFER* pBuffer, const XAUDIO2_BUFFER_WMA* pBufferWMA = (NULL)) PURE;
        STDMETHOD(FlushSourceBuffers) () PURE;
        STDMETHOD(Discontinuity) () PURE;
        STDMETHOD(ExitLoop) (UINT32 OperationSet = (XAUDIO2_COMMIT_NOW)) PURE;
        STDMETHOD_(void, GetState) (XAUDIO2_VOICE_STATE* pVoiceState, UINT32 Flags = (0)) PURE;
        STDMETHOD(SetFrequencyRatio) (float Ratio,
            UINT32 OperationSet = (XAUDIO2_COMMIT_NOW)) PURE;
        STDMETHOD_(void, GetFrequencyRatio) (float* pRatio) PURE;
        STDMETHOD(SetSourceSampleRate) (UINT32 NewSourceSampleRate) PURE;
    };

    // IXAudio2SubmixVoice 
    interface IXAudio2SubmixVoice : IXAudio2Voice {

    };

    // IXAudio2MasteringVoice
    interface IXAudio2MasteringVoice : IXAudio2Voice {
        STDMETHOD(GetChannelMask) (DWORD* pChannelmask) PURE;
    };

    interface IXAudio2EngineCallback {
        // Called by XAudio2 just before an audio processing pass begins.
        STDMETHOD_(void, OnProcessingPassStart) () PURE;
        // Called just after an audio processing pass ends.
        STDMETHOD_(void, OnProcessingPassEnd) () PURE;
        // Called in the event of a critical system error which requires XAudio2
        // to be closed down and restarted.  The error code is given in Error.
        STDMETHOD_(void, OnCriticalError) (HRESULT Error) PURE;
    };


    interface IXAudio2VoiceCallback {
        // Called just before this voice's processing pass begins.
        STDMETHOD_(void, OnVoiceProcessingPassStart) (UINT32 BytesRequired) PURE;
        // Called just after this voice's processing pass ends.
        STDMETHOD_(void, OnVoiceProcessingPassEnd) () PURE;
        // Called when this voice has just finished playing a buffer stream
        // (as marked with the XAUDIO2_END_OF_STREAM flag on the last buffer).
        STDMETHOD_(void, OnStreamEnd) () PURE;
        // Called when this voice is about to start processing a new buffer.
        STDMETHOD_(void, OnBufferStart) (void* pBufferContext) PURE;
        // Called when this voice has just finished processing a buffer.
        // The buffer can now be reused or destroyed.
        STDMETHOD_(void, OnBufferEnd) (void* pBufferContext) PURE;
        // Called when this voice has just reached the end position of a loop.
        STDMETHOD_(void, OnLoopEnd) (void* pBufferContext) PURE;
        // Called in the event of a critical error during voice processing,
        // such as a failing xAPO or an error from the hardware XMA decoder.
        // The voice may have to be destroyed and re-created to recover from
        // the error.  The callback arguments report which buffer was being
        // processed when the error occurred, and its HRESULT code.
        STDMETHOD_(void, OnVoiceError) (void* pBufferContext, HRESULT Error) PURE;
    };

}}