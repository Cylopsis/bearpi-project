# 音频组件<a name="ZH-CN_TOPIC_0000001146901937"></a>

-   [简介](#section119mcpsimp)
    -   [基本概念](#section122mcpsimp)

-   [目录](#section179mcpsimp)
-   [使用说明](#section112738505318)
    -   [音频播放](#section1147510562812)
    -   [音频录制](#section295162052813)
    -   [音频管理](#section645572311287)

-   [相关仓](#section340mcpsimp)

## 简介<a name="section119mcpsimp"></a>

音频组件用于实现音频相关的功能，包括音频播放，录制，音量管理和设备管理。

**图 1**  音频组件架构图<a name="fig483116248288"></a>  


![](figures/zh-cn_image_0000001152315135.png)

### 基本概念<a name="section122mcpsimp"></a>

-   **采样**

采样是指将连续时域上的模拟信号按照一定的时间间隔采样，获取到离散时域上离散信号的过程。

-   **采样率**

采样率为每秒从连续信号中提取并组成离散信号的采样次数，单位用赫兹（Hz）来表示。通常人耳能听到频率范围大约在20Hz～20kHz之间的声音。常用的音频采样频率有：8kHz、11.025kHz、22.05kHz、16kHz、37.8kHz、44.1kHz、48kHz、96kHz、192kHz等。

-   **声道**

声道是指声音在录制或播放时在不同空间位置采集或回放的相互独立的音频信号，所以声道数也就是声音录制时的音源数量或回放时相应的扬声器数量。

-   **音频帧**

音频数据是流式的，本身没有明确的一帧帧的概念，在实际的应用中，为了音频算法处理/传输的方便，一般约定俗成取2.5ms\~60ms为单位的数据量为一帧音频。这个时间被称之为“采样时间”，其长度没有特别的标准，它是根据编解码器和具体应用的需求来决定的。

-   **PCM**

PCM（Pulse Code Modulation），即脉冲编码调制，是一种将模拟信号数字化的方法，是将时间连续、取值连续的模拟信号转换成时间离散、抽样值离散的数字信号的过程。

## 目录<a name="section179mcpsimp"></a>

仓目录结构如下：

```
/foundation/multimedia/audio_standard  # 音频组件业务代码
├── frameworks                         # 框架代码
│   ├── innerkitsimpl                  # 内部接口实现
│   └── kitsimpl                       # 外部接口实现
├── interfaces                         # 接口代码
│   ├── innerkits                      # 内部接口
│   └── kits                           # 外部接口
├── sa_profile                         # 服务配置文件
├── services                           # 服务代码
├── LICENSE                            # 证书文件
└── ohos.build                         # 编译文件
```

## 使用说明<a name="section112738505318"></a>

### 音频播放<a name="section1147510562812"></a>

可以使用此仓库内提供的接口将音频数据转换为音频模拟信号，使用输出设备播放音频信号，以及管理音频播放任务。以下步骤描述了如何使用 **AudioRenderer** 开发音频播放功能：

1.  使用 **Create** 接口和所需流类型来获取 **AudioRenderer** 实例。

    ```
    AudioStreamType streamType = STREAM_MUSIC; // 流类型示例
    std::unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(streamType);
    ```

2.  （可选）静态接口 **GetSupportedFormats**(), **GetSupportedChannels**(), **GetSupportedEncodingTypes**(), **GetSupportedSamplingRates**() 可用于获取支持的参数。
3.  准备设备，调用实例的 **SetParams** 。

    ```
    AudioRendererParams rendererParams;
    rendererParams.sampleFormat = SAMPLE_S16LE;
    rendererParams.sampleRate = SAMPLE_RATE_44100;
    rendererParams.channelCount = STEREO;
    rendererParams.encodingType = ENCODING_PCM;
     
    audioRenderer->SetParams(rendererParams);
    ```

4.  （可选）使用 audioRenderer->**GetParams**(rendererParams) 来验证 SetParams。
5.  AudioRenderer 实例调用 audioRenderer->**Start**() 函数来启动播放任务。
6.  使用 **GetBufferSize** 接口获取要写入的缓冲区长度。

    ```
    audioRenderer->GetBufferSize(bufferLen);
    ```

7.  从源（例如音频文件）读取要播放的音频数据并将其传输到字节流中。重复调用Write函数写入渲染数据。

    ```
    bytesToWrite = fread(buffer, 1, bufferLen, wavFile);
    while ((bytesWritten < bytesToWrite) && ((bytesToWrite - bytesWritten) > minBytes)) {
        bytesWritten += audioRenderer->Write(buffer + bytesWritten, bytesToWrite - bytesWritten);
        if (bytesWritten < 0)
            break;
    }
    ```

8.  调用audioRenderer->**Drain**()来清空播放流。
9.  调用audioRenderer->**Stop**()来停止输出。
10. 播放任务完成后，调用AudioRenderer实例的audioRenderer->**Release**()函数来释放资源。

以上提供了基本音频播放使用场景。


11. 使用 audioRenderer->**SetVolume(float)** 和 audioRenderer->**GetVolume()** 来设置和获取当前音频流音量, 可选范围为 0.0 到 1.0。

提供上述基本音频播放使用范例。更多接口说明请参考[**audio_renderer.h**](https://gitee.com/openharmony/multimedia_audio_standard/blob/master/interfaces/innerkits/native/audiorenderer/include/audio_renderer.h) 和 [**audio_info.h**](https://gitee.com/openharmony/multimedia_audio_standard/blob/master/interfaces/innerkits/native/audiocommon/include/audio_info.h)。

### 音频录制<a name="section295162052813"></a>

可以使用此仓库内提供的接口，让应用程序可以完成使用输入设备进行声音录制，将语音转换为音频数据，并管理录制的任务。以下步骤描述了如何使用 **AudioCapturer** 开发音频录制功能：

1.  使用Create接口和所需流类型来获取 **AudioCapturer** 实例。

    ```
    AudioStreamType streamType = STREAM_MUSIC;
    std::unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(streamType);
    ```

2.  （可选）静态接口 **GetSupportedFormats**(), **GetSupportedChannels**(), **GetSupportedEncodingTypes**(), **GetSupportedSamplingRates**() 可用于获取支持的参数。
3.  准备设备，调用实例的 **SetParams** 。

    ```
    AudioCapturerParams capturerParams;
    capturerParams.sampleFormat = SAMPLE_S16LE;
    capturerParams.sampleRate = SAMPLE_RATE_44100;
    capturerParams.channelCount = STEREO;
    capturerParams.encodingType = ENCODING_PCM;

    audioCapturer->SetParams(capturerParams);
    ```

4.  （可选）使用 audioCapturer->**GetParams**(capturerParams) 来验证 SetParams()。
5.  AudioCapturer 实例调用 AudioCapturer->**Start**() 函数来启动录音任务。
6.  使用 **GetBufferSize** 接口获取要写入的缓冲区长度。

    ```
    audioCapturer->GetBufferSize(bufferLen);
    ```

7.  读取录制的音频数据并将其转换为字节流。重复调用read函数读取数据直到主动停止。

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

8.  （可选）audioCapturer->**Flush**() 来清空录音流缓冲区。
9.  AudioCapturer 实例调用 audioCapturer->**Stop**() 函数停止录音。
10. 录音任务完成后，调用 AudioCapturer 实例的 audioCapturer->**Release**() 函数释放资源。

提供上述基本音频录制使用范例。更多API请参考[**audio_capturer.h**](https://gitee.com/openharmony/multimedia_audio_standard/blob/master/interfaces/innerkits/native/audiocapturer/include/audio_capturer.h)和[**audio_info.h**](https://gitee.com/openharmony/multimedia_audio_standard/blob/master/interfaces/innerkits/native/audiocommon/include/audio_info.h)。

### 音频管理<a name="section645572311287"></a>
可以使用 [**audio_system_manager.h**](https://gitee.com/openharmony/multimedia_audio_standard/blob/master/interfaces/innerkits/native/audiomanager/include/audio_system_manager.h) 内的接口来控制音量和设备。
1. 使用 **GetInstance** 接口获取 **AudioSystemManager** 实例.
    ```
    AudioSystemManager *audioSystemMgr = AudioSystemManager::GetInstance();
    ```
#### 音量控制
2. 使用 **GetMaxVolume** 和  **GetMinVolume** 接口去查询音频流支持的最大和最小音量等级，在此范围内设置音量。
    ```
    AudioSystemManager::AudioVolumeType streamType = AudioSystemManager::AudioVolumeType::STREAM_MUSIC;
    int32_t maxVol = audioSystemMgr->GetMaxVolume(streamType);
    int32_t minVol = audioSystemMgr->GetMinVolume(streamType);
    ```
3. 使用 **SetVolume** 和 **GetVolume** 接口来设置和获取指定音频流的音量等级。
    ```
    int32_t result = audioSystemMgr->SetVolume(streamType, 10);
    int32_t vol = audioSystemMgr->GetVolume(streamType);
    ```
4. 使用 **SetMute** 和 **IsStreamMute** 接口来设置和获取指定音频流的静音状态。
    ```
    int32_t result = audioSystemMgr->SetMute(streamType, true);
    bool isMute = audioSystemMgr->IsStreamMute(streamType);
5. 使用 **SetRingerMode** 和 **GetRingerMode** 接口来设置和获取铃声模式。参考在 [**audio_info.h**](https://gitee.com/openharmony/multimedia_audio_standard/blob/master/interfaces/innerkits/native/audiocommon/include/audio_info.h)  定义的 **AudioRingerMode** 枚举来获取支持的铃声模式。
    ```
    int32_t result = audioSystemMgr->SetRingerMode(RINGER_MODE_SILENT);
    AudioRingerMode ringMode = audioSystemMgr->GetRingerMode();
    ```
6. 使用 **SetMicrophoneMute** 和 **IsMicrophoneMute** 接口来设置和获取麦克风的静音状态。
    ```
    int32_t result = audioSystemMgr->SetMicrophoneMute(true);
    bool isMicMute = audioSystemMgr->IsMicrophoneMute();
    ```
#### 设备控制
7. 使用 **GetDevices**, **deviceType_** 和 **deviceRole_** 接口来获取音频输入输出设备信息。 参考 [**audio_info.h**](https://gitee.com/openharmony/multimedia_audio_standard/blob/master/interfaces/innerkits/native/audiocommon/include/audio_info.h) 内定义的DeviceFlag, DeviceType 和 DeviceRole 枚举。
    ```
    DeviceFlag deviceFlag = OUTPUT_DEVICES_FLAG;
    vector<sptr<AudioDeviceDescriptor>> audioDeviceDescriptors
        = audioSystemMgr->GetDevices(deviceFlag);
    sptr<AudioDeviceDescriptor> audioDeviceDescriptor = audioDeviceDescriptors[0];
    cout << audioDeviceDescriptor->deviceType_;
    cout << audioDeviceDescriptor->deviceRole_;
    ```
8. 使用 **SetDeviceActive** 和 **IsDeviceActive** 接口去激活/去激活音频设备和获取音频设备激活状态。
     ```
    ActiveDeviceType deviceType = SPEAKER;
    int32_t result = audioSystemMgr->SetDeviceActive(deviceType, true);
    bool isDevActive = audioSystemMgr->IsDeviceActive(deviceType);
    ```
9. 提供其他用途的接口如 **IsStreamActive**, **SetAudioParameter** and **GetAudioParameter**, 详细请参考 [**audio_system_manager.h**](https://gitee.com/openharmony/multimedia_audio_standard/blob/master/interfaces/innerkits/native/audiomanager/include/audio_system_manager.h) 

#### JavaScript 用法:
JavaScript应用可以使用系统提供的音频管理接口，来控制音量和设备。\
请参考 [**音频管理.md**](https://gitee.com/openharmony/docs/blob/master/zh-cn/application-dev/js-reference/音频管理.md) 来获取音量和设备管理相关JavaScript接口的用法。

## 相关仓<a name="section340mcpsimp"></a>

[multimedia\_audio\_standard](https://gitee.com/openharmony/multimedia_audio_standard)

