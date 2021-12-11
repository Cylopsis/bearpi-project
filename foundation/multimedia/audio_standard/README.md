# Audio<a name="EN-US_TOPIC_0000001146901937"></a>

  - [Introduction](#introduction)
    - [Basic Concepts](#basic-concepts)
  - [Directory Structure](#directory-structure)
  - [Usage Guidelines](#usage-guidelines)
    - [Audio Playback](#audio-playback)
    - [Audio Recording](#audio-recording)
    - [Audio Management](#audio-management)
  - [Repositories Involved](#repositories-involved)

## Introduction<a name="introduction"></a>
The **audio\_standard** repository is used to implement audio-related features, including audio playback, recording, volume management and device management.

**Figure  1**  Position in the subsystem architecture<a name="fig483116248288"></a>  


![](figures/en-us_image_0000001152315135.png)

### Basic Concepts<a name="basic-concepts"></a>

-   **Sampling**

Sampling is a process to obtain discrete-time signals by extracting samples from analog signals in a continuous time domain at a specific interval.

-   **Sampling rate**

Sampling rate is the number of samples extracted from a continuous signal per second to form a discrete signal. It is measured in Hz. Generally, human hearing range is from 20 Hz to 20 kHz. Common audio sampling rates include 8 kHz, 11.025 kHz, 22.05 kHz, 16 kHz, 37.8 kHz, 44.1 kHz, 48 kHz, and 96 kHz.

-   **Channel**

Channels refer to different spatial positions where independent audio signals are recorded or played. The number of channels is the number of audio sources used during audio recording, or the number of speakers used for audio playback.

-   **Audio frame**

Audio data is in stream form. For the convenience of audio algorithm processing and transmission, it is generally agreed that a data amount in a unit of 2.5 to 60 milliseconds is one audio frame. This unit is called sampling time, and its length is specific to codecs and the application requirements.

-   **PCM**

Pulse code modulation \(PCM\) is a method used to digitally represent sampled analog signals. It converts continuous-time analog signals into discrete-time digital signal samples.

## Directory Structure<a name="directory-structure"></a>

The structure of the repository directory is as follows:

```
/foundation/multimedia/audio_standard  # Audio code
├── frameworks                         # Framework code
│   ├── innerkitsimpl                  # Internal Native API Implementation.
|   |                                    Pulseaudio, libsndfile build configuration and pulseaudio-hdi modules
│   └── kitsimpl                       # External JS API Implementation
├── interfaces                         # Interfaces
│   ├── innerkits                      # Internal Native APIs
│   └── kits                           # External JS APIs
├── sa_profile                         # Service configuration profile
├── services                           # Service code
├── LICENSE                            # License file
└── ohos.build                         # Build file
```

## Usage Guidelines<a name="usage-guidelines"></a>
### Audio Playback<a name="audio-playback"></a>
You can use APIs provided in this repository to convert audio data into audible analog signals, play the audio signals using output devices, and manage playback tasks. The following steps describe how to use  **AudioRenderer**  to develop the audio playback function:
1. Use **Create** API with required stream type to get **AudioRenderer** instance.
    ```
    AudioStreamType streamType = STREAM_MUSIC; // example stream type
    std::unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(streamType);
    ```
2. (Optional) Static APIs **GetSupportedFormats**(), **GetSupportedChannels**(), **GetSupportedEncodingTypes**(), **GetSupportedSamplingRates**() can be used to get the supported values of the params.
3. To Prepare the device, call **SetParams** on the instance.
    ```
    AudioRendererParams rendererParams;
    rendererParams.sampleFormat = SAMPLE_S16LE;
    rendererParams.sampleRate = SAMPLE_RATE_44100;
    rendererParams.channelCount = STEREO;
    rendererParams.encodingType = ENCODING_PCM;

    audioRenderer->SetParams(rendererParams);
    ```
4. (Optional) use audioRenderer->**GetParams**(rendererParams) to validate SetParams
5. Call **audioRenderer->Start()** function on the AudioRenderer instance to start the playback task.
6. Get the buffer length to be written, using **GetBufferSize** API .
    ```
    audioRenderer->GetBufferSize(bufferLen);
    ```
7. Read the audio data to be played from the source(for example, an audio file) and transfer it into the bytes stream. Call the **Write** function repeatedly to write the render data.
    ```
    bytesToWrite = fread(buffer, 1, bufferLen, wavFile);
    while ((bytesWritten < bytesToWrite) && ((bytesToWrite - bytesWritten) > minBytes)) {
        bytesWritten += audioRenderer->Write(buffer + bytesWritten, bytesToWrite - bytesWritten);
        if (bytesWritten < 0)
            break;
    }
    ```
8. Call audioRenderer->**Drain**() to drain the playback stream.

9. Call audioRenderer->**Stop()** function to Stop rendering.
10. After the playback task is complete, call the audioRenderer->**Release**() function on the AudioRenderer instance to release the stream resources.

Provided the basic playback usecase above.

11. Use audioRenderer->**SetVolume(float)** and audioRenderer->**GetVolume()** to set and get Track Volume. Value ranges from 0.0 to 1.0

Please refer [**audio_renderer.h**](https://gitee.com/openharmony/multimedia_audio_standard/blob/master/interfaces/innerkits/native/audiorenderer/include/audio_renderer.h) and [**audio_info.h**](https://gitee.com/openharmony/multimedia_audio_standard/blob/master/interfaces/innerkits/native/audiocommon/include/audio_info.h) for more such useful APIs.


### Audio Recording<a name="audio-recording"></a>
You can use the APIs provided in this repository for your application to record voices using input devices, convert the voices into audio data, and manage recording tasks. The following steps describe how to use **AudioCapturer** to develop the audio recording function:

1. Use **Create** API with required stream type to get **AudioCapturer** instance.
    ```
    AudioStreamType streamType = STREAM_MUSIC;
    std::unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(streamType);
    ```
2. (Optional) Static APIs **GetSupportedFormats**(), **GetSupportedChannels**(), **GetSupportedEncodingTypes**(), **GetSupportedSamplingRates()** can be used to get the supported values of the params.
3. To Prepare the device, call **SetParams** on the instance.
    ```
    AudioCapturerParams capturerParams;
    capturerParams.sampleFormat = SAMPLE_S16LE;
    capturerParams.sampleRate = SAMPLE_RATE_44100;
    capturerParams.channelCount = STEREO;
    capturerParams.encodingType = ENCODING_PCM;

    audioCapturer->SetParams(capturerParams);
    ```
4. (Optional) use audioCapturer->**GetParams**(capturerParams) to validate SetParams()
5. Call audioCapturer->**Start**() function on the AudioCapturer instance to start the recording task.
6. Get the buffer length to be read, using **GetBufferSize** API. 
    ```
    audioCapturer->GetBufferSize(bufferLen);
    ```
7. Read the captured audio data and convert it to a byte stream. Call the read function repeatedly to read data untill you want to stop recording
    ```
    // set isBlocking = true/false for blocking/non-blocking read
    bytesRead = audioCapturer->Read(*buffer, bufferLen, isBlocking);
    while (numBuffersToCapture) {
        bytesRead = audioCapturer->Read(*buffer, bufferLen, isBlockingRead);
        if (bytesRead < 0) {
            break;
        } else if (bytesRead > 0) {
            fwrite(buffer, size, bytesRead, recFile); // example shows writes the recored data into a file
            numBuffersToCapture--;
        }
    }
    ```
8. (Optional) Call audioCapturer->**Flush**() to flush the capture buffer of this stream.
9. Call the audioCapturer->**Stop**() function on the AudioCapturer instance to stop the recording.
10. After the recording task is complete, call the audioCapturer->**Release**() function on the AudioCapturer instance to release the stream resources.

Provided the basic recording usecase above. Please refer [**audio_capturer.h**](https://gitee.com/openharmony/multimedia_audio_standard/blob/master/interfaces/innerkits/native/audiocapturer/include/audio_capturer.h) and [**audio_info.h**](https://gitee.com/openharmony/multimedia_audio_standard/blob/master/interfaces/innerkits/native/audiocommon/include/audio_info.h) for more APIs.

### Audio Management<a name="audio-management"></a>
You can use the APIs provided in [**audio_system_manager.h**](https://gitee.com/openharmony/multimedia_audio_standard/blob/master/interfaces/innerkits/native/audiomanager/include/audio_system_manager.h) to control volume and device.
1. Use **GetInstance** API to get **AudioSystemManager** instance.
    ```
    AudioSystemManager *audioSystemMgr = AudioSystemManager::GetInstance();
    ```
#### Volume Control
2. Use **GetMaxVolume** and  **GetMinVolume** APIs to query the Maximum & Minimum volume level allowed for the stream. Use this volume range to set the volume.
    ```
    AudioSystemManager::AudioVolumeType streamType = AudioSystemManager::AudioVolumeType::STREAM_MUSIC;
    int32_t maxVol = audioSystemMgr->GetMaxVolume(streamType);
    int32_t minVol = audioSystemMgr->GetMinVolume(streamType);
    ```
3. Use **SetVolume** and **GetVolume** APIs to set and get the volume level of the stream.
    ```
    int32_t result = audioSystemMgr->SetVolume(streamType, 10);
    int32_t vol = audioSystemMgr->GetVolume(streamType);
    ```
4. Use **SetMute** and **IsStreamMute** APIs to set and get the mute status of the stream.
    ```
    int32_t result = audioSystemMgr->SetMute(streamType, true);
    bool isMute = audioSystemMgr->IsStreamMute(streamType);
5. Use **SetRingerMode** and **GetRingerMode** APIs to set and get ringer modes. Refer **AudioRingerMode** enum in [**audio_info.h**](https://gitee.com/openharmony/multimedia_audio_standard/blob/master/interfaces/innerkits/native/audiocommon/include/audio_info.h) for supported ringer modes.
    ```
    int32_t result = audioSystemMgr->SetRingerMode(RINGER_MODE_SILENT);
    AudioRingerMode ringMode = audioSystemMgr->GetRingerMode();
    ```
6. Use **SetMicrophoneMute** and **IsMicrophoneMute** APIs to mute/unmute the mic and to check if mic muted.
    ```
    int32_t result = audioSystemMgr->SetMicrophoneMute(true);
    bool isMicMute = audioSystemMgr->IsMicrophoneMute();
    ```
#### Device control
7. Use **GetDevices**, **deviceType_** and **deviceRole_** APIs to get audio I/O devices information. For DeviceFlag, DeviceType and DeviceRole enums refer [**audio_info.h**](https://gitee.com/openharmony/multimedia_audio_standard/blob/master/interfaces/innerkits/native/audiocommon/include/audio_info.h).
    ```
    DeviceFlag deviceFlag = OUTPUT_DEVICES_FLAG;
    vector<sptr<AudioDeviceDescriptor>> audioDeviceDescriptors
        = audioSystemMgr->GetDevices(deviceFlag);
    sptr<AudioDeviceDescriptor> audioDeviceDescriptor = audioDeviceDescriptors[0];
    cout << audioDeviceDescriptor->deviceType_;
    cout << audioDeviceDescriptor->deviceRole_;
    ```
8. Use **SetDeviceActive** and **IsDeviceActive** APIs to Actiavte/Deactivate the device and to check if the device is active.
     ```
    ActiveDeviceType deviceType = SPEAKER;
    int32_t result = audioSystemMgr->SetDeviceActive(deviceType, true);
    bool isDevActive = audioSystemMgr->IsDeviceActive(deviceType);
    ```
9. Other useful APIs such as **IsStreamActive**, **SetAudioParameter** and **GetAudioParameter** are also provided. Please refer [**audio_system_manager.h**](https://gitee.com/openharmony/multimedia_audio_standard/blob/master/interfaces/innerkits/native/audiomanager/include/audio_system_manager.h) for more details

#### JavaScript Usage:
JavaScript apps can use the APIs provided by audio manager to control the volume and the device.\
Please refer [**audio-management.md**](https://gitee.com/openharmony/docs/blob/master/en/application-dev/js-reference/audio-management.md) for JavaScript usage of audio volume and device management.

## Repositories Involved<a name="repositories-involved"></a>

[multimedia\_audio\_standard](https://gitee.com/openharmony/multimedia_audio_standard)
