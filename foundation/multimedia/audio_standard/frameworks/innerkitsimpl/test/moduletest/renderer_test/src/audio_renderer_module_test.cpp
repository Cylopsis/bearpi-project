/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "audio_renderer_module_test.h"

#include "audio_errors.h"
#include "audio_info.h"
#include "audio_renderer.h"

using namespace std;
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
namespace {
    const string AUDIORENDER_TEST_FILE_PATH = "/data/test_44100_2.wav";
    const int32_t VALUE_ZERO = 0;
    // Writing only 500 buffers of data for test
    const int32_t WRITE_BUFFERS_COUNT = 500;
    constexpr int32_t PAUSE_BUFFER_POSITION = 400000;
    constexpr int32_t PAUSE_RENDER_TIME_SECONDS = 1;
} // namespace

void AudioRendererModuleTest::SetUpTestCase(void) {}
void AudioRendererModuleTest::TearDownTestCase(void) {}
void AudioRendererModuleTest::SetUp(void) {}
void AudioRendererModuleTest::TearDown(void) {}

int32_t AudioRendererModuleTest::InitializeRenderer(unique_ptr<AudioRenderer> &audioRenderer)
{
    AudioRendererParams rendererParams;
    rendererParams.sampleFormat = SAMPLE_S16LE;
    rendererParams.sampleRate = SAMPLE_RATE_44100;
    rendererParams.channelCount = STEREO;
    rendererParams.encodingType = ENCODING_PCM;

    return audioRenderer->SetParams(rendererParams);
}

/**
* @tc.name  : Test GetSupportedFormats API
* @tc.number: Audio_Renderer_GetSupportedFormats_001
* @tc.desc  : Test GetSupportedFormats interface. Returns supported Formats on success.
*/
HWTEST(AudioRendererModuleTest, Audio_Renderer_GetSupportedFormats_001, TestSize.Level1)
{
    vector<AudioSampleFormat> supportedFormatList = AudioRenderer::GetSupportedFormats();
    EXPECT_EQ(AUDIO_SUPPORTED_FORMATS.size(), supportedFormatList.size());
}

/**
* @tc.name  : Test GetSupportedChannels API
* @tc.number: Audio_Renderer_GetSupportedChannels_001
* @tc.desc  : Test GetSupportedChannels interface. Returns supported Channels on success.
*/
HWTEST(AudioRendererModuleTest, Audio_Renderer_GetSupportedChannels_001, TestSize.Level1)
{
    vector<AudioChannel> supportedChannelList = AudioRenderer::GetSupportedChannels();
    EXPECT_EQ(AUDIO_SUPPORTED_CHANNELS.size(), supportedChannelList.size());
}

/**
* @tc.name  : Test GetSupportedEncodingTypes API
* @tc.number: Audio_Renderer_GetSupportedEncodingTypes_001
* @tc.desc  : Test GetSupportedEncodingTypes interface. Returns supported Encoding types on success.
*/
HWTEST(AudioRendererModuleTest, Audio_Renderer_GetSupportedEncodingTypes_001, TestSize.Level1)
{
    vector<AudioEncodingType> supportedEncodingTypes
                                        = AudioRenderer::GetSupportedEncodingTypes();
    EXPECT_EQ(AUDIO_SUPPORTED_ENCODING_TYPES.size(), supportedEncodingTypes.size());
}

/**
* @tc.name  : Test GetSupportedSamplingRates API
* @tc.number: Audio_Renderer_GetSupportedSamplingRates_001
* @tc.desc  : Test GetSupportedSamplingRates interface. Returns supported Sampling rates on success.
*/
HWTEST(AudioRendererModuleTest, Audio_Renderer_GetSupportedSamplingRates_001, TestSize.Level1)
{
    vector<AudioSamplingRate> supportedSamplingRates = AudioRenderer::GetSupportedSamplingRates();
    EXPECT_EQ(AUDIO_SUPPORTED_SAMPLING_RATES.size(), supportedSamplingRates.size());
}

/**
* @tc.name  : Test Create API via legal input.
* @tc.number: Audio_Renderer_Create_001
* @tc.desc  : Test Create interface with STREAM_MUSIC. Returns audioRenderer instance, if create is successful.
*/
HWTEST(AudioRendererModuleTest, Audio_Renderer_Create_001, TestSize.Level1)
{
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    EXPECT_NE(nullptr, audioRenderer);
}

/**
* @tc.name  : Test Create API via legal input.
* @tc.number: Audio_Renderer_Create_002
* @tc.desc  : Test Create interface with STREAM_RING. Returns audioRenderer instance, if create is successful.
*/
HWTEST(AudioRendererModuleTest, Audio_Renderer_Create_002, TestSize.Level1)
{
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_RING);
    EXPECT_NE(nullptr, audioRenderer);
}

/**
* @tc.name  : Test Create API via legal input.
* @tc.number: Audio_Renderer_Create_003
* @tc.desc  : Test Create interface with STREAM_VOICE_CALL. Returns audioRenderer instance if create is successful.
*             Note: instance will be created but functional support for STREAM_VOICE_CALL not available yet.
*/
HWTEST(AudioRendererModuleTest, Audio_Renderer_Create_003, TestSize.Level1)
{
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_VOICE_CALL);
    EXPECT_NE(nullptr, audioRenderer);
}

/**
* @tc.name  : Test Create API via legal input.
* @tc.number: Audio_Renderer_Create_004
* @tc.desc  : Test Create interface with STREAM_SYSTEM. Returns audioRenderer instance, if create is successful.
*             Note: instance will be created but functional support for STREAM_SYSTEM not available yet.
*/
HWTEST(AudioRendererModuleTest, Audio_Renderer_Create_004, TestSize.Level1)
{
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_SYSTEM);
    EXPECT_NE(nullptr, audioRenderer);
}

/**
* @tc.name  : Test Create API via legal input.
* @tc.number: Audio_Renderer_Create_005
* @tc.desc  : Test Create interface with STREAM_BLUETOOTH_SCO. Returns audioRenderer instance, if create is successful.
*             Note: instance will be created but functional support for STREAM_BLUETOOTH_SCO not available yet
*/
HWTEST(AudioRendererModuleTest, Audio_Renderer_Create_005, TestSize.Level1)
{
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_BLUETOOTH_SCO);
    EXPECT_NE(nullptr, audioRenderer);
}

/**
* @tc.name  : Test Create API via legal input.
* @tc.number: Audio_Renderer_Create_006
* @tc.desc  : Test Create interface with STREAM_ALARM. Returns audioRenderer instance, if create is successful.
*             Note: instance will be created but functional support for STREAM_ALARM not available yet.
*/
HWTEST(AudioRendererModuleTest, Audio_Renderer_Create_006, TestSize.Level1)
{
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_ALARM);
    EXPECT_NE(nullptr, audioRenderer);
}

/**
* @tc.name  : Test Create API via legal input.
* @tc.number: Audio_Renderer_Create_007
* @tc.desc  : Test Create interface with STREAM_NOTIFICATION. Returns audioRenderer instance, if create is successful.
*             Note: instance will be created but functional support for STREAM_NOTIFICATION not available yet.
*/
HWTEST(AudioRendererModuleTest, Audio_Renderer_Create_007, TestSize.Level1)
{
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_NOTIFICATION);
    EXPECT_NE(nullptr, audioRenderer);
}

/**
* @tc.name  : Test SetParams API via legal input
* @tc.number: Audio_Renderer_SetParams_001
* @tc.desc  : Test SetParams interface. Returns 0 {SUCCESS}, if the setting is successful.
*             rendererParams.sampleFormat = SAMPLE_S16LE;
*             rendererParams.sampleRate = SAMPLE_RATE_44100;
*             rendererParams.channelCount = STEREO;
*             rendererParams.encodingType = ENCODING_PCM;
*/
HWTEST(AudioRendererModuleTest, Audio_Renderer_SetParams_001, TestSize.Level1)
{
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    AudioRendererParams rendererParams;
    rendererParams.sampleFormat = SAMPLE_S16LE;
    rendererParams.sampleRate = SAMPLE_RATE_44100;
    rendererParams.channelCount = STEREO;
    rendererParams.encodingType = ENCODING_PCM;

    int32_t ret = audioRenderer->SetParams(rendererParams);
    EXPECT_EQ(SUCCESS, ret);
    audioRenderer->Release();
}

/**
* @tc.name  : Test SetParams API via legal input.
* @tc.number: Audio_Renderer_SetParams_002
* @tc.desc  : Test SetParams interface. Returns 0 {SUCCESS}, if the setting is successful.
*             rendererParams.sampleFormat = SAMPLE_S16LE;
*             rendererParams.sampleRate = SAMPLE_RATE_8000;
*             rendererParams.channelCount = MONO;
*             rendererParams.encodingType = ENCODING_PCM;
*/
HWTEST(AudioRendererModuleTest, Audio_Renderer_SetParams_002, TestSize.Level1)
{
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    AudioRendererParams rendererParams;
    rendererParams.sampleFormat = SAMPLE_S16LE;
    rendererParams.sampleRate = SAMPLE_RATE_8000;
    rendererParams.channelCount = MONO;
    rendererParams.encodingType = ENCODING_PCM;

    int32_t ret = audioRenderer->SetParams(rendererParams);
    EXPECT_EQ(SUCCESS, ret);
    audioRenderer->Release();
}

/**
* @tc.name  : Test SetParams API via legal input.
* @tc.number: Audio_Renderer_SetParams_003
* @tc.desc  : Test SetParams interface. Returns 0 {SUCCESS}, if the setting is successful.
*             rendererParams.sampleFormat = SAMPLE_S16LE;
*             rendererParams.sampleRate = SAMPLE_RATE_11025;
*             rendererParams.channelCount = STEREO;
*             rendererParams.encodingType = ENCODING_PCM;
*/
HWTEST(AudioRendererModuleTest, Audio_Renderer_SetParams_003, TestSize.Level1)
{
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    AudioRendererParams rendererParams;
    rendererParams.sampleFormat = SAMPLE_S16LE;
    rendererParams.sampleRate = SAMPLE_RATE_11025;
    rendererParams.channelCount = STEREO;
    rendererParams.encodingType = ENCODING_PCM;

    int32_t ret = audioRenderer->SetParams(rendererParams);
    EXPECT_EQ(SUCCESS, ret);
    audioRenderer->Release();
}

/**
* @tc.name  : Test SetParams API via legal input.
* @tc.number: Audio_Renderer_SetParams_004
* @tc.desc  : Test SetParams interface. Returns 0 {SUCCESS}, if the setting is successful.
*             rendererParams.sampleFormat = SAMPLE_S16LE;
*             rendererParams.sampleRate = SAMPLE_RATE_22050;
*             rendererParams.channelCount = MONO;
*             rendererParams.encodingType = ENCODING_PCM;
*/
HWTEST(AudioRendererModuleTest, Audio_Renderer_SetParams_004, TestSize.Level1)
{
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    AudioRendererParams rendererParams;
    rendererParams.sampleFormat = SAMPLE_S16LE;
    rendererParams.sampleRate = SAMPLE_RATE_22050;
    rendererParams.channelCount = MONO;
    rendererParams.encodingType = ENCODING_PCM;

    int32_t ret = audioRenderer->SetParams(rendererParams);
    EXPECT_EQ(SUCCESS, ret);
    audioRenderer->Release();
}

/**
* @tc.name  : Test SetParams API via legal input.
* @tc.number: Audio_Renderer_SetParams_005
* @tc.desc  : Test SetParams interface. Returns 0 {SUCCESS}, if the setting is successful.
*             rendererParams.sampleFormat = SAMPLE_S16LE;
*             rendererParams.sampleRate = SAMPLE_RATE_96000;
*             rendererParams.channelCount = MONO;
*             rendererParams.encodingType = ENCODING_PCM;
*/
HWTEST(AudioRendererModuleTest, Audio_Renderer_SetParams_005, TestSize.Level1)
{
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    AudioRendererParams rendererParams;
    rendererParams.sampleFormat = SAMPLE_S16LE;
    rendererParams.sampleRate = SAMPLE_RATE_96000;
    rendererParams.channelCount = MONO;
    rendererParams.encodingType = ENCODING_PCM;

    int32_t ret = audioRenderer->SetParams(rendererParams);
    EXPECT_EQ(SUCCESS, ret);
}

/**
* @tc.name  : Test SetParams API via legal input.
* @tc.number: Audio_Renderer_SetParams_006
* @tc.desc  : Test SetParams interface. Returns 0 {SUCCESS}, if the setting is successful.
*             rendererParams.sampleFormat = SAMPLE_S24LE;
*             rendererParams.sampleRate = SAMPLE_RATE_64000;
*             rendererParams.channelCount = MONO;
*             rendererParams.encodingType = ENCODING_PCM;
*/
HWTEST(AudioRendererModuleTest, Audio_Renderer_SetParams_006, TestSize.Level1)
{
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    AudioRendererParams rendererParams;
    rendererParams.sampleFormat = SAMPLE_S24LE;
    rendererParams.sampleRate = SAMPLE_RATE_64000;
    rendererParams.channelCount = MONO;
    rendererParams.encodingType = ENCODING_PCM;

    int32_t ret = audioRenderer->SetParams(rendererParams);
    EXPECT_EQ(SUCCESS, ret);
}

/**
* @tc.name  : Test SetParams API via illegal input.
* @tc.number: Audio_Renderer_SetParams_007
* @tc.desc  : Test SetParams interface. Returns 0 {SUCCESS}, if the setting is successful.
*             rendererParams.sampleFormat = SAMPLE_S16LE;
*             rendererParams.sampleRate = SAMPLE_RATE_16000;
*             rendererParams.channelCount = STEREO;
*             rendererParams.encodingType = ENCODING_PCM;
*/
HWTEST(AudioRendererModuleTest, Audio_Renderer_SetParams_007, TestSize.Level1)
{
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    AudioRendererParams rendererParams;
    rendererParams.sampleFormat = SAMPLE_S16LE;
    rendererParams.sampleRate = SAMPLE_RATE_16000;
    rendererParams.channelCount = STEREO;
    rendererParams.encodingType = ENCODING_PCM;

    int32_t ret = audioRenderer->SetParams(rendererParams);
    EXPECT_EQ(SUCCESS, ret);
    audioRenderer->Release();
}

/**
* @tc.name  : Test GetParams API via legal input.
* @tc.number: Audio_Renderer_GetParams_001
* @tc.desc  : Test GetParams interface. Returns 0 {SUCCESS}, if the getting is successful.
*/
HWTEST(AudioRendererModuleTest, Audio_Renderer_GetParams_001, TestSize.Level1)
{
    int32_t ret = -1;
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    AudioRendererParams rendererParams;
    rendererParams.sampleFormat = SAMPLE_S16LE;
    rendererParams.sampleRate = SAMPLE_RATE_44100;
    rendererParams.channelCount = STEREO;
    rendererParams.encodingType = ENCODING_PCM;
    ret = audioRenderer->SetParams(rendererParams);
    EXPECT_EQ(SUCCESS, ret);

    AudioRendererParams getRendererParams;
    ret = audioRenderer->GetParams(getRendererParams);
    EXPECT_EQ(SUCCESS, ret);
    EXPECT_EQ(rendererParams.sampleFormat, getRendererParams.sampleFormat);
    EXPECT_EQ(rendererParams.sampleRate, getRendererParams.sampleRate);
    EXPECT_EQ(rendererParams.channelCount, getRendererParams.channelCount);
    EXPECT_EQ(rendererParams.encodingType, getRendererParams.encodingType);

    audioRenderer->Release();
}

/**
* @tc.name  : Test GetParams API via legal state, RENDERER_RUNNING: GetParams after Start.
* @tc.number: Audio_Renderer_GetParams_002
* @tc.desc  : Test GetParams interface. Returns 0 {SUCCESS} if the getting is successful.
*/
HWTEST(AudioRendererModuleTest, Audio_Renderer_GetParams_002, TestSize.Level1)
{
    int32_t ret = -1;
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    AudioRendererParams rendererParams;
    rendererParams.sampleFormat = SAMPLE_S16LE;
    rendererParams.sampleRate = SAMPLE_RATE_44100;
    rendererParams.channelCount = MONO;
    rendererParams.encodingType = ENCODING_PCM;
    ret = audioRenderer->SetParams(rendererParams);
    EXPECT_EQ(SUCCESS, ret);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    AudioRendererParams getRendererParams;
    ret = audioRenderer->GetParams(getRendererParams);
    EXPECT_EQ(SUCCESS, ret);

    audioRenderer->Release();
}

/**
* @tc.name  : Test GetParams API via illegal state, RENDERER_NEW: Call GetParams without SetParams.
* @tc.number: Audio_Renderer_GetParams_003
* @tc.desc  : Test GetParams interface. Returns error code, if the renderer state is RENDERER_NEW.
*/
HWTEST(AudioRendererModuleTest, Audio_Renderer_GetParams_003, TestSize.Level1)
{
    int32_t ret = -1;
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    AudioRendererParams rendererParams;
    rendererParams.sampleFormat = SAMPLE_S16LE;
    rendererParams.sampleRate = SAMPLE_RATE_44100;
    rendererParams.channelCount = MONO;
    rendererParams.encodingType = ENCODING_PCM;

    AudioRendererParams getRendererParams;
    ret = audioRenderer->GetParams(getRendererParams);
    EXPECT_EQ(ERR_OPERATION_FAILED, ret);
}

/**
* @tc.name  : Test GetParams API via illegal state, RENDERER_RELEASED: Call GetParams after Release.
* @tc.number: Audio_Renderer_GetParams_004
* @tc.desc  : Test GetParams interface. Returns error code, if the renderer state is RENDERER_RELEASED.
*/
HWTEST(AudioRendererModuleTest, Audio_Renderer_GetParams_004, TestSize.Level1)
{
    int32_t ret = -1;
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    ret = AudioRendererModuleTest::InitializeRenderer(audioRenderer);
    EXPECT_EQ(SUCCESS, ret);

    bool isReleased = audioRenderer->Release();
    EXPECT_EQ(true, isReleased);

    AudioRendererParams getRendererParams;
    ret = audioRenderer->GetParams(getRendererParams);
    EXPECT_EQ(ERR_OPERATION_FAILED, ret);
}

/**
* @tc.name  : Test GetParams API via legal state, RENDERER_STOPPED: GetParams after Stop.
* @tc.number: Audio_Renderer_GetParams_005
* @tc.desc  : Test GetParams interface. Returns 0 {SUCCESS}, if the getting is successful.
*/
HWTEST(AudioRendererModuleTest, Audio_Renderer_GetParams_005, TestSize.Level1)
{
    int32_t ret = -1;
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    ret = AudioRendererModuleTest::InitializeRenderer(audioRenderer);
    EXPECT_EQ(SUCCESS, ret);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    bool isStopped = audioRenderer->Stop();
    EXPECT_EQ(true, isStopped);

    AudioRendererParams getRendererParams;
    ret = audioRenderer->GetParams(getRendererParams);
    EXPECT_EQ(SUCCESS, ret);

    audioRenderer->Release();
}

/**
* @tc.name  : Test GetBufferSize API via legal input.
* @tc.number: Audio_Renderer_GetBufferSize_001
* @tc.desc  : Test GetBufferSize interface. Returns 0 {SUCCESS}, if the getting is successful.
*/
HWTEST(AudioRendererModuleTest, Audio_Renderer_GetBufferSize_001, TestSize.Level1)
{
    int32_t ret = -1;
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    ret = AudioRendererModuleTest::InitializeRenderer(audioRenderer);
    EXPECT_EQ(SUCCESS, ret);

    size_t bufferLen;
    ret = audioRenderer->GetBufferSize(bufferLen);
    EXPECT_EQ(SUCCESS, ret);

    audioRenderer->Release();
}

/**
* @tc.name  : Test GetBufferSize API via illegal state, RENDERER_NEW: without initializing the renderer.
* @tc.number: Audio_Renderer_GetBufferSize_002
* @tc.desc  : Test GetBufferSize interface. Returns error code, if the renderer state is RENDERER_NEW.
*/
HWTEST(AudioRendererModuleTest, Audio_Renderer_GetBufferSize_002, TestSize.Level1)
{
    int32_t ret = -1;
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    size_t bufferLen;
    ret = audioRenderer->GetBufferSize(bufferLen);
    EXPECT_EQ(ERR_OPERATION_FAILED, ret);
}

/**
* @tc.name  : Test GetBufferSize API via illegal state, RENDERER_RELEASED: call Release before GetBufferSize
* @tc.number: Audio_Renderer_GetBufferSize_003
* @tc.desc  : Test GetBufferSize interface. Returns error code, if the renderer state is RENDERER_RELEASED.
*/
HWTEST(AudioRendererModuleTest, Audio_Renderer_GetBufferSize_003, TestSize.Level1)
{
    int32_t ret = -1;
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    ret = AudioRendererModuleTest::InitializeRenderer(audioRenderer);
    EXPECT_EQ(SUCCESS, ret);

    bool isReleased = audioRenderer->Release();
    EXPECT_EQ(true, isReleased);

    size_t bufferLen;
    ret = audioRenderer->GetBufferSize(bufferLen);
    EXPECT_EQ(ERR_OPERATION_FAILED, ret);
}

/**
* @tc.name  : Test GetBufferSize API via legal state, RENDERER_STOPPED: call Stop before GetBufferSize
* @tc.number: Audio_Renderer_GetBufferSize_004
* @tc.desc  : Test GetBufferSize interface. Returns 0 {SUCCESS}, if the getting is successful.
*/
HWTEST(AudioRendererModuleTest, Audio_Renderer_GetBufferSize_004, TestSize.Level1)
{
    int32_t ret = -1;
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    ret = AudioRendererModuleTest::InitializeRenderer(audioRenderer);
    EXPECT_EQ(SUCCESS, ret);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    bool isStopped = audioRenderer->Stop();
    EXPECT_EQ(true, isStopped);

    size_t bufferLen;
    ret = audioRenderer->GetBufferSize(bufferLen);
    EXPECT_EQ(SUCCESS, ret);

    audioRenderer->Release();
}

/**
* @tc.name  : Test GetBufferSize API via legal state, RENDERER_RUNNING: call Start before GetBufferSize
* @tc.number: Audio_Renderer_GetBufferSize_005
* @tc.desc  : test GetBufferSize interface. Returns 0 {SUCCESS}, if the getting is successful.
*/
HWTEST(AudioRendererModuleTest, Audio_Renderer_GetBufferSize_005, TestSize.Level1)
{
    int32_t ret = -1;
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    ret = AudioRendererModuleTest::InitializeRenderer(audioRenderer);
    EXPECT_EQ(SUCCESS, ret);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    size_t bufferLen;
    ret = audioRenderer->GetBufferSize(bufferLen);
    EXPECT_EQ(SUCCESS, ret);

    audioRenderer->Release();
}

/**
* @tc.name  : Test GetFrameCount API via legal input.
* @tc.number: Audio_Renderer_GetFrameCount_001
* @tc.desc  : test GetFrameCount interface, Returns 0 {SUCCESS}, if the getting is successful.
*/
HWTEST(AudioRendererModuleTest, Audio_Renderer_GetFrameCount_001, TestSize.Level1)
{
    int32_t ret = -1;
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    ret = AudioRendererModuleTest::InitializeRenderer(audioRenderer);
    EXPECT_EQ(SUCCESS, ret);

    uint32_t frameCount;
    ret = audioRenderer->GetFrameCount(frameCount);
    EXPECT_EQ(SUCCESS, ret);

    audioRenderer->Release();
}

/**
* @tc.name  : Test GetFrameCount API via illegal state, RENDERER_NEW: without initialiing the renderer.
* @tc.number: Audio_Renderer_GetFrameCount_002
* @tc.desc  : Test GetFrameCount interface. Returns error code, if the renderer state is RENDERER_NEW.
*/
HWTEST(AudioRendererModuleTest, Audio_Renderer_GetFrameCount_002, TestSize.Level1)
{
    int32_t ret = -1;
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    uint32_t frameCount;
    ret = audioRenderer->GetFrameCount(frameCount);
    EXPECT_EQ(ERR_OPERATION_FAILED, ret);
}

/**
* @tc.name  : Test GetFrameCount API via legal state, RENDERER_RUNNING: call Start before GetFrameCount.
* @tc.number: Audio_Renderer_GetFrameCount_003
* @tc.desc  : Test GetFrameCount interface. Returns 0 {SUCCESS}, if the getting is successful.
*/
HWTEST(AudioRendererModuleTest, Audio_Renderer_GetFrameCount_003, TestSize.Level1)
{
    int32_t ret = -1;
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    ret = AudioRendererModuleTest::InitializeRenderer(audioRenderer);
    EXPECT_EQ(SUCCESS, ret);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    uint32_t frameCount;
    ret = audioRenderer->GetFrameCount(frameCount);
    EXPECT_EQ(SUCCESS, ret);

    audioRenderer->Release();
}

/**
* @tc.name  : Test GetFrameCount API via legal state, RENDERER_STOPPED: call Stop before GetFrameCount
* @tc.number: Audio_Renderer_GetFrameCount_004
* @tc.desc  : Test GetFrameCount interface. Returns 0 {SUCCESS}, if the getting is successful.
*/
HWTEST(AudioRendererModuleTest, Audio_Renderer_GetFrameCount_004, TestSize.Level1)
{
    int32_t ret = -1;
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    ret = AudioRendererModuleTest::InitializeRenderer(audioRenderer);
    EXPECT_EQ(SUCCESS, ret);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    bool isStopped = audioRenderer->Stop();
    EXPECT_EQ(true, isStopped);

    uint32_t frameCount;
    ret = audioRenderer->GetFrameCount(frameCount);
    EXPECT_EQ(SUCCESS, ret);

    audioRenderer->Release();
}

/**
* @tc.name  : Test GetFrameCount API via illegal state, RENDERER_RELEASED: call Release before GetFrameCount
* @tc.number: Audio_Renderer_GetFrameCount_005
* @tc.desc  : Test GetFrameCount interface.  Returns error code, if the state is RENDERER_RELEASED.
*/
HWTEST(AudioRendererModuleTest, Audio_Renderer_GetFrameCount_005, TestSize.Level1)
{
    int32_t ret = -1;
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    ret = AudioRendererModuleTest::InitializeRenderer(audioRenderer);
    EXPECT_EQ(SUCCESS, ret);

    bool isReleased = audioRenderer->Release();
    EXPECT_EQ(true, isReleased);

    uint32_t frameCount;
    ret = audioRenderer->GetFrameCount(frameCount);
    EXPECT_EQ(ERR_OPERATION_FAILED, ret);
}

/**
* @tc.name  : Test SetVolume
* @tc.number: Audio_Renderer_SetVolume_001
* @tc.desc  : Test SetVolume interface, Returns 0 {SUCCESS}, if the track volume is set.
*/
HWTEST(AudioRendererModuleTest, Audio_Renderer_SetVolume_001, TestSize.Level1)
{
    int32_t ret = -1;
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    ret = AudioRendererModuleTest::InitializeRenderer(audioRenderer);
    EXPECT_EQ(SUCCESS, ret);

    ret = audioRenderer->SetVolume(0.5);
    EXPECT_EQ(SUCCESS, ret);

    bool isReleased = audioRenderer->Release();
    EXPECT_EQ(true, isReleased);
}

/**
* @tc.name  : Test SetVolume
* @tc.number: Audio_Renderer_SetVolume_002
* @tc.desc  : Test SetVolume interface for minimum and maximum volumes.
*/
HWTEST(AudioRendererModuleTest, Audio_Renderer_SetVolume_002, TestSize.Level1)
{
    int32_t ret = -1;
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    ret = AudioRendererModuleTest::InitializeRenderer(audioRenderer);
    EXPECT_EQ(SUCCESS, ret);

    ret = audioRenderer->SetVolume(0);
    EXPECT_EQ(SUCCESS, ret);

    ret = audioRenderer->SetVolume(1.0);
    EXPECT_EQ(SUCCESS, ret);

    bool isReleased = audioRenderer->Release();
    EXPECT_EQ(true, isReleased);
}

/**
* @tc.name  : Test SetVolume
* @tc.number: Audio_Renderer_SetVolume_003
* @tc.desc  : Test SetVolume interface for out of range values.
*/
HWTEST(AudioRendererModuleTest, Audio_Renderer_SetVolume_003, TestSize.Level1)
{
    int32_t ret = -1;
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    ret = AudioRendererModuleTest::InitializeRenderer(audioRenderer);
    EXPECT_EQ(SUCCESS, ret);

    ret = audioRenderer->SetVolume(-0.5);
    EXPECT_NE(SUCCESS, ret);

    ret = audioRenderer->SetVolume(1.5);
    EXPECT_NE(SUCCESS, ret);

    bool isReleased = audioRenderer->Release();
    EXPECT_EQ(true, isReleased);
}

/**
* @tc.name  : Test GetVolume
* @tc.number: Audio_Renderer_GetVolume_001
* @tc.desc  : Test GetVolume interface to get the default value.
*/
HWTEST(AudioRendererModuleTest, Audio_Renderer_GetVolume_001, TestSize.Level1)
{
    int32_t ret = -1;
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    ret = AudioRendererModuleTest::InitializeRenderer(audioRenderer);
    EXPECT_EQ(SUCCESS, ret);

    float volume = audioRenderer->GetVolume();
    EXPECT_EQ(1.0, volume);

    bool isReleased = audioRenderer->Release();
    EXPECT_EQ(true, isReleased);
}

/**
* @tc.name  : Test GetVolume
* @tc.number: Audio_Renderer_GetVolume_002
* @tc.desc  : Test GetVolume interface after set volume call.
*/
HWTEST(AudioRendererModuleTest, Audio_Renderer_GetVolume_002, TestSize.Level1)
{
    int32_t ret = -1;
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    ret = AudioRendererModuleTest::InitializeRenderer(audioRenderer);
    EXPECT_EQ(SUCCESS, ret);

    ret = audioRenderer->SetVolume(0.5);
    EXPECT_EQ(SUCCESS, ret);

    float volume = audioRenderer->GetVolume();
    EXPECT_EQ(0.5, volume);

    bool isReleased = audioRenderer->Release();
    EXPECT_EQ(true, isReleased);
}

/**
* @tc.name  : Test GetVolume
* @tc.number: Audio_Renderer_GetVolume_003
* @tc.desc  : Test GetVolume interface after set volume fails.
*/
HWTEST(AudioRendererModuleTest, Audio_Renderer_GetVolume_003, TestSize.Level1)
{
    int32_t ret = -1;
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    ret = AudioRendererModuleTest::InitializeRenderer(audioRenderer);
    EXPECT_EQ(SUCCESS, ret);

    ret = audioRenderer->SetVolume(0.5);
    EXPECT_EQ(SUCCESS, ret);

    ret = audioRenderer->SetVolume(1.5);
    EXPECT_NE(SUCCESS, ret);

    float volume = audioRenderer->GetVolume();
    EXPECT_EQ(0.5, volume);

    bool isReleased = audioRenderer->Release();
    EXPECT_EQ(true, isReleased);
}

/**
* @tc.name  : Test Start API via legal state, RENDERER_PREPARED.
* @tc.number: Audio_Renderer_Start_001
* @tc.desc  : Test Start interface. Returns true if start is successful.
*/
HWTEST(AudioRendererModuleTest, Audio_Renderer_Start_001, TestSize.Level1)
{
    int32_t ret = -1;
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    ret = AudioRendererModuleTest::InitializeRenderer(audioRenderer);
    EXPECT_EQ(SUCCESS, ret);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    audioRenderer->Release();
}

/**
* @tc.name  : Test Start API via illegal state, RENDERER_NEW: without initializing the renderer.
* @tc.number: Audio_Renderer_Start_002
* @tc.desc  : Test Start interface. Returns false, if the renderer state is RENDERER_NEW.
*/
HWTEST(AudioRendererModuleTest, Audio_Renderer_Start_002, TestSize.Level1)
{
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(false, isStarted);
}

/**
* @tc.name  : Test Start API via illegal state, RENDERER_RELEASED: call Start after Release
* @tc.number: Audio_Renderer_Start_003
* @tc.desc  : Test Start interface. Returns false, if the renderer state is RENDERER_RELEASED.
*/
HWTEST(AudioRendererModuleTest, Audio_Renderer_Start_003, TestSize.Level1)
{
    int32_t ret = -1;
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    ret = AudioRendererModuleTest::InitializeRenderer(audioRenderer);
    EXPECT_EQ(SUCCESS, ret);

    bool isReleased = audioRenderer->Release();
    EXPECT_EQ(true, isReleased);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(false, isStarted);

    audioRenderer->Release();
}

/**
* @tc.name  : Test Start API via legal state, RENDERER_STOPPED: Start Stop and then Start again
* @tc.number: Audio_Renderer_Start_004
* @tc.desc  : Test Start interface. Returns true, if the start is successful.
*/
HWTEST(AudioRendererModuleTest, Audio_Renderer_Start_004, TestSize.Level1)
{
    int32_t ret = -1;
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    ret = AudioRendererModuleTest::InitializeRenderer(audioRenderer);
    EXPECT_EQ(SUCCESS, ret);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    bool isStopped = audioRenderer->Stop();
    EXPECT_EQ(true, isStopped);

    isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    audioRenderer->Release();
}

/**
* @tc.name  : Test Start API via illegal state, RENDERER_RUNNING : call Start repeatedly
* @tc.number: Audio_Renderer_Start_005
* @tc.desc  : Test Start interface. Returns false, if the renderer state is RENDERER_RUNNING.
*/
HWTEST(AudioRendererModuleTest, Audio_Renderer_Start_005, TestSize.Level1)
{
    int32_t ret = -1;
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    ret = AudioRendererModuleTest::InitializeRenderer(audioRenderer);
    EXPECT_EQ(SUCCESS, ret);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    isStarted = audioRenderer->Start();
    EXPECT_EQ(false, isStarted);

    audioRenderer->Release();
}

/**
* @tc.name  : Test Write API.
* @tc.number: Audio_Renderer_Write_001
* @tc.desc  : Test Write interface. Returns number of bytes written, if the write is successful.
*/

HWTEST(AudioRendererModuleTest, Audio_Renderer_Write_001, TestSize.Level1)
{
    int32_t ret = -1;
    FILE *wavFile = fopen(AUDIORENDER_TEST_FILE_PATH.c_str(), "rb");
    ASSERT_NE(nullptr, wavFile);

    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    ret = AudioRendererModuleTest::InitializeRenderer(audioRenderer);
    EXPECT_EQ(SUCCESS, ret);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    size_t bufferLen;
    ret = audioRenderer->GetBufferSize(bufferLen);
    EXPECT_EQ(SUCCESS, ret);

    uint8_t *buffer = (uint8_t *) malloc(bufferLen);
    ASSERT_NE(nullptr, buffer);

    size_t bytesToWrite = 0;
    int32_t bytesWritten = 0;
    size_t minBytes = 4;
    int32_t numBuffersToRender = WRITE_BUFFERS_COUNT;

    while (numBuffersToRender) {
        bytesToWrite = fread(buffer, 1, bufferLen, wavFile);
        bytesWritten = 0;
        while ((static_cast<size_t>(bytesWritten) < bytesToWrite) &&
            ((static_cast<size_t>(bytesToWrite) - bytesWritten) > minBytes)) {
            bytesWritten += audioRenderer->Write(buffer + static_cast<size_t>(bytesWritten),
                                                 bytesToWrite - static_cast<size_t>(bytesWritten));
            EXPECT_GE(bytesWritten, VALUE_ZERO);
            if (bytesWritten < 0) {
                break;
            }
        }
        numBuffersToRender--;
    }

    audioRenderer->Drain();
    audioRenderer->Stop();
    audioRenderer->Release();

    free(buffer);
    fclose(wavFile);
}

/**
* @tc.name  : Test Write API via illegl state, RENDERER_NEW : without Initializing the renderer.
* @tc.number: Audio_Renderer_Write_002
* @tc.desc  : Test Write interface. Returns error code, if the renderer state is RENDERER_NEW.
*           : bufferLen is invalid here, firstly bufferLen is validated in Write. So it returns ERR_INVALID_PARAM.
*/
HWTEST(AudioRendererModuleTest, Audio_Renderer_Write_002, TestSize.Level1)
{
    int32_t ret = -1;
    FILE *wavFile = fopen(AUDIORENDER_TEST_FILE_PATH.c_str(), "rb");
    ASSERT_NE(nullptr, wavFile);

    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(false, isStarted);

    size_t bufferLen;
    ret = audioRenderer->GetBufferSize(bufferLen);
    EXPECT_EQ(ERR_OPERATION_FAILED, ret);

    uint8_t *buffer = (uint8_t *) malloc(bufferLen);
    ASSERT_NE(nullptr, buffer);

    size_t bytesToWrite = fread(buffer, 1, bufferLen, wavFile);
    int32_t bytesWritten = audioRenderer->Write(buffer, bytesToWrite);
    EXPECT_EQ(ERR_INVALID_PARAM, bytesWritten);

    free(buffer);
    fclose(wavFile);
}

/**
* @tc.name  : Test Write API via illegl state, RENDERER_PREPARED : Write without Start.
* @tc.number: Audio_Renderer_Write_003
* @tc.desc  : Test Write interface. Returns error code, if the renderer state is not RENDERER_RUNNING.
*/
HWTEST(AudioRendererModuleTest, Audio_Renderer_Write_003, TestSize.Level1)
{
    int32_t ret = -1;
    FILE *wavFile = fopen(AUDIORENDER_TEST_FILE_PATH.c_str(), "rb");
    ASSERT_NE(nullptr, wavFile);

    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    ret = AudioRendererModuleTest::InitializeRenderer(audioRenderer);
    EXPECT_EQ(SUCCESS, ret);

    size_t bufferLen;
    ret = audioRenderer->GetBufferSize(bufferLen);
    EXPECT_EQ(SUCCESS, ret);

    uint8_t *buffer = (uint8_t *) malloc(bufferLen);
    ASSERT_NE(nullptr, buffer);

    size_t bytesToWrite = fread(buffer, 1, bufferLen, wavFile);
    int32_t bytesWritten = audioRenderer->Write(buffer, bytesToWrite);
    EXPECT_EQ(ERR_ILLEGAL_STATE, bytesWritten);

    audioRenderer->Release();

    free(buffer);
    fclose(wavFile);
}

/**
* @tc.name  : Test Write API via illegal input, bufferLength = 0.
* @tc.number: Audio_Renderer_Write_004
* @tc.desc  : Test Write interface. Returns error code, if the bufferLength <= 0.
*/
HWTEST(AudioRendererModuleTest, Audio_Renderer_Write_004, TestSize.Level1)
{
    int32_t ret = -1;
    FILE *wavFile = fopen(AUDIORENDER_TEST_FILE_PATH.c_str(), "rb");
    ASSERT_NE(nullptr, wavFile);

    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    ret = AudioRendererModuleTest::InitializeRenderer(audioRenderer);
    EXPECT_EQ(SUCCESS, ret);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    size_t bufferLen = 0;

    uint8_t *buffer = (uint8_t *) malloc(bufferLen);
    ASSERT_NE(nullptr, buffer);

    size_t bytesToWrite = fread(buffer, 1, bufferLen, wavFile);
    int32_t bytesWritten = audioRenderer->Write(buffer, bytesToWrite);
    EXPECT_EQ(ERR_INVALID_PARAM, bytesWritten);

    audioRenderer->Stop();
    audioRenderer->Release();

    free(buffer);
    fclose(wavFile);
}

/**
* @tc.name  : Test Write API via illegal input, buffer = nullptr.
* @tc.number: Audio_Renderer_Write_005
* @tc.desc  : Test Write interface. Returns error code, if the buffer = nullptr.
*/
HWTEST(AudioRendererModuleTest, Audio_Renderer_Write_005, TestSize.Level1)
{
    int32_t ret = -1;
    FILE *wavFile = fopen(AUDIORENDER_TEST_FILE_PATH.c_str(), "rb");
    ASSERT_NE(nullptr, wavFile);

    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    ret = AudioRendererModuleTest::InitializeRenderer(audioRenderer);
    EXPECT_EQ(SUCCESS, ret);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    size_t bufferLen;
    ret = audioRenderer->GetBufferSize(bufferLen);
    EXPECT_EQ(SUCCESS, ret);

    uint8_t *buffer = (uint8_t *) malloc(bufferLen);
    ASSERT_NE(nullptr, buffer);

    size_t bytesToWrite = fread(buffer, 1, bufferLen, wavFile);
    uint8_t *buffer_null = nullptr;
    int32_t bytesWritten = audioRenderer->Write(buffer_null, bytesToWrite);
    EXPECT_EQ(ERR_INVALID_PARAM, bytesWritten);

    audioRenderer->Stop();
    audioRenderer->Release();

    free(buffer);
    fclose(wavFile);
}

/**
* @tc.name  : Test Write API via illegal state, RENDERER_STOPPED: Write after Stop
* @tc.number: Audio_Renderer_Write_006
* @tc.desc  : Test Write interface. Returns error code, if the renderer state is not RENDERER_RUNNING
*/
HWTEST(AudioRendererModuleTest, Audio_Renderer_Write_006, TestSize.Level1)
{
    int32_t ret = -1;
    FILE *wavFile = fopen(AUDIORENDER_TEST_FILE_PATH.c_str(), "rb");
    ASSERT_NE(nullptr, wavFile);

    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    ret = AudioRendererModuleTest::InitializeRenderer(audioRenderer);
    EXPECT_EQ(SUCCESS, ret);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    size_t bufferLen;
    ret = audioRenderer->GetBufferSize(bufferLen);
    EXPECT_EQ(SUCCESS, ret);

    uint8_t *buffer = (uint8_t *) malloc(bufferLen);
    ASSERT_NE(nullptr, buffer);

    bool isStopped = audioRenderer->Stop();
    EXPECT_EQ(true, isStopped);

    size_t bytesToWrite = fread(buffer, 1, bufferLen, wavFile);
    int32_t bytesWritten = audioRenderer->Write(buffer, bytesToWrite);
    EXPECT_EQ(ERR_ILLEGAL_STATE, bytesWritten);

    audioRenderer->Release();

    free(buffer);
    fclose(wavFile);
}

/**
* @tc.name  : Test Write API via illegal state, RENDERER_RELEASED: Write after Release
* @tc.number: Audio_Renderer_Write_007
* @tc.desc  : Test Write interface. Returns error code, if the renderer state is not RENDERER_RUNNING
*/
HWTEST(AudioRendererModuleTest, Audio_Renderer_Write_007, TestSize.Level1)
{
    int32_t ret = -1;
    FILE *wavFile = fopen(AUDIORENDER_TEST_FILE_PATH.c_str(), "rb");
    ASSERT_NE(nullptr, wavFile);

    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    ret = AudioRendererModuleTest::InitializeRenderer(audioRenderer);
    EXPECT_EQ(SUCCESS, ret);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    size_t bufferLen;
    ret = audioRenderer->GetBufferSize(bufferLen);
    EXPECT_EQ(SUCCESS, ret);

    uint8_t *buffer = (uint8_t *) malloc(bufferLen);
    ASSERT_NE(nullptr, buffer);

    bool isReleased = audioRenderer->Release();
    EXPECT_EQ(true, isReleased);

    size_t bytesToWrite = fread(buffer, 1, bufferLen, wavFile);
    int32_t bytesWritten = audioRenderer->Write(buffer, bytesToWrite);
    EXPECT_EQ(ERR_ILLEGAL_STATE, bytesWritten);

    free(buffer);
    fclose(wavFile);
}

/**
* @tc.name  : Test Write API.
* @tc.number: Audio_Renderer_Write_008
* @tc.desc  : Test Write interface after pause and resume. Returns number of bytes written, if the write is successful.
*/

HWTEST(AudioRendererModuleTest, Audio_Renderer_Write_008, TestSize.Level1)
{
    int32_t ret = -1;
    FILE *wavFile = fopen(AUDIORENDER_TEST_FILE_PATH.c_str(), "rb");
    ASSERT_NE(nullptr, wavFile);

    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    ret = AudioRendererModuleTest::InitializeRenderer(audioRenderer);
    EXPECT_EQ(SUCCESS, ret);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    size_t bufferLen;
    ret = audioRenderer->GetBufferSize(bufferLen);
    EXPECT_EQ(SUCCESS, ret);

    uint8_t *buffer = (uint8_t *) malloc(bufferLen);
    ASSERT_NE(nullptr, buffer);

    size_t bytesToWrite = 0;
    int32_t bytesWritten = 0;
    size_t minBytes = 4;
    int32_t numBuffersToRender = WRITE_BUFFERS_COUNT;
    bool pauseTested = false;

    while (numBuffersToRender) {
        bytesToWrite = fread(buffer, 1, bufferLen, wavFile);
        bytesWritten = 0;
        uint64_t currFilePos = ftell(wavFile);
        if (!pauseTested && (currFilePos > PAUSE_BUFFER_POSITION) && audioRenderer->Pause()) {
            pauseTested = true;
            sleep(PAUSE_RENDER_TIME_SECONDS);
            isStarted = audioRenderer->Start();
            EXPECT_EQ(true, isStarted);

            ret = audioRenderer->SetVolume(0.5);
            EXPECT_EQ(SUCCESS, ret);
            float volume = audioRenderer->GetVolume();
            EXPECT_EQ(0.5, volume);
        }

        while ((static_cast<size_t>(bytesWritten) < bytesToWrite) &&
            ((static_cast<size_t>(bytesToWrite) - bytesWritten) > minBytes)) {
            bytesWritten += audioRenderer->Write(buffer + static_cast<size_t>(bytesWritten),
                                                 bytesToWrite - static_cast<size_t>(bytesWritten));
            EXPECT_GE(bytesWritten, VALUE_ZERO);
            if (bytesWritten < 0) {
                break;
            }
        }
        numBuffersToRender--;
    }

    audioRenderer->Drain();
    audioRenderer->Stop();
    audioRenderer->Release();

    free(buffer);
    fclose(wavFile);
}

/**
* @tc.name  : Test GetAudioTime API via legal input.
* @tc.number: Audio_Renderer_GetAudioTime_001
* @tc.desc  : Test GetAudioTime interface. Returns true, if the getting is successful.
*/
HWTEST(AudioRendererModuleTest, Audio_Renderer_GetAudioTime_001, TestSize.Level1)
{
    int32_t ret = -1;
    FILE *wavFile = fopen(AUDIORENDER_TEST_FILE_PATH.c_str(), "rb");
    ASSERT_NE(nullptr, wavFile);

    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    ret = AudioRendererModuleTest::InitializeRenderer(audioRenderer);
    EXPECT_EQ(SUCCESS, ret);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    size_t bufferLen;
    ret = audioRenderer->GetBufferSize(bufferLen);
    EXPECT_EQ(SUCCESS, ret);

    uint8_t *buffer = (uint8_t *) malloc(bufferLen);
    ASSERT_NE(nullptr, buffer);

    size_t bytesToWrite = fread(buffer, 1, bufferLen, wavFile);
    int32_t bytesWritten = audioRenderer->Write(buffer, bytesToWrite);
    EXPECT_GE(bytesWritten, VALUE_ZERO);

    Timestamp timeStamp;
    bool getAudioTime = audioRenderer->GetAudioTime(timeStamp, Timestamp::Timestampbase::MONOTONIC);
    EXPECT_EQ(true, getAudioTime);
    EXPECT_GE(timeStamp.time.tv_sec, (const long)VALUE_ZERO);
    EXPECT_GE(timeStamp.time.tv_nsec, (const long)VALUE_ZERO);

    audioRenderer->Drain();
    audioRenderer->Stop();
    audioRenderer->Release();

    free(buffer);
    fclose(wavFile);
}

/**
* @tc.name  : Test GetAudioTime API via illegal state, RENDERER_NEW: GetAudioTime without initializing the renderer.
* @tc.number: Audio_Renderer_GetAudioTime_002
* @tc.desc  : Test GetAudioTime interface. Returns false, if the renderer state is RENDERER_NEW
*/
HWTEST(AudioRendererModuleTest, Audio_Renderer_GetAudioTime_002, TestSize.Level1)
{
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    Timestamp timeStamp;
    bool getAudioTime = audioRenderer->GetAudioTime(timeStamp, Timestamp::Timestampbase::MONOTONIC);
    EXPECT_EQ(false, getAudioTime);
}

/**
* @tc.name  : Test GetAudioTime API via legal state, RENDERER_RUNNING.
* @tc.number: Audio_Renderer_GetAudioTime_003
* @tc.desc  : test GetAudioTime interface. Returns true, if the getting is successful.
*/
HWTEST(AudioRendererModuleTest, Audio_Renderer_GetAudioTime_003, TestSize.Level1)
{
    int32_t ret = -1;
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    ret = AudioRendererModuleTest::InitializeRenderer(audioRenderer);
    EXPECT_EQ(SUCCESS, ret);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    Timestamp timeStamp;
    bool getAudioTime = audioRenderer->GetAudioTime(timeStamp, Timestamp::Timestampbase::MONOTONIC);
    EXPECT_EQ(true, getAudioTime);

    audioRenderer->Release();
}

/**
* @tc.name  : Test GetAudioTime API via legal state, RENDERER_STOPPED.
* @tc.number: Audio_Renderer_GetAudioTime_004
* @tc.desc  : Test GetAudioTime interface. Returns true, if the getting is successful.
*/
HWTEST(AudioRendererModuleTest, Audio_Renderer_GetAudioTime_004, TestSize.Level1)
{
    int32_t ret = -1;
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    ret = AudioRendererModuleTest::InitializeRenderer(audioRenderer);
    EXPECT_EQ(SUCCESS, ret);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    bool isStopped = audioRenderer->Stop();
    EXPECT_EQ(true, isStopped);

    Timestamp timeStamp;
    bool getAudioTime = audioRenderer->GetAudioTime(timeStamp, Timestamp::Timestampbase::MONOTONIC);
    EXPECT_EQ(true, getAudioTime);

    audioRenderer->Release();
}

/**
* @tc.name  : Test GetAudioTime API via illegal state, RENDERER_RELEASED: GetAudioTime after Release.
* @tc.number: Audio_Renderer_GetAudioTime_005
* @tc.desc  : Test GetAudioTime interface. Returns false, if the renderer state is RENDERER_RELEASED
*/
HWTEST(AudioRendererModuleTest, Audio_Renderer_GetAudioTime_005, TestSize.Level1)
{
    int32_t ret = -1;
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    ret = AudioRendererModuleTest::InitializeRenderer(audioRenderer);
    EXPECT_EQ(SUCCESS, ret);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    bool isStopped = audioRenderer->Stop();
    EXPECT_EQ(true, isStopped);

    bool isReleased = audioRenderer->Release();
    EXPECT_EQ(true, isReleased);

    Timestamp timeStamp;
    bool getAudioTime = audioRenderer->GetAudioTime(timeStamp, Timestamp::Timestampbase::MONOTONIC);
    EXPECT_EQ(false, getAudioTime);
}

/**
* @tc.name  : Test Drain API.
* @tc.number: Audio_Renderer_Drain_001
* @tc.desc  : Test Drain interface. Returns true, if the flush is successful.
*/
HWTEST(AudioRendererModuleTest, Audio_Renderer_Drain_001, TestSize.Level1)
{
    int32_t ret = -1;
    FILE *wavFile = fopen(AUDIORENDER_TEST_FILE_PATH.c_str(), "rb");
    ASSERT_NE(nullptr, wavFile);

    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    ret = AudioRendererModuleTest::InitializeRenderer(audioRenderer);
    EXPECT_EQ(SUCCESS, ret);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    size_t bufferLen;
    ret = audioRenderer->GetBufferSize(bufferLen);
    EXPECT_EQ(SUCCESS, ret);

    uint8_t *buffer = (uint8_t *) malloc(bufferLen);
    ASSERT_NE(nullptr, buffer);

    size_t bytesToWrite = fread(buffer, 1, bufferLen, wavFile);
    int32_t bytesWritten = audioRenderer->Write(buffer, bytesToWrite);
    EXPECT_GE(bytesWritten, VALUE_ZERO);

    bool isDrained = audioRenderer->Drain();
    EXPECT_EQ(true, isDrained);

    audioRenderer->Stop();
    audioRenderer->Release();

    free(buffer);
    fclose(wavFile);
}

/**
* @tc.name  : Test Drain API via illegal state, RENDERER_NEW: Without initializing the renderer.
* @tc.number: Audio_Renderer_Drain_002
* @tc.desc  : Test Drain interface. Returns false, if the renderer state is not RENDERER_RUNNING.
*/
HWTEST(AudioRendererModuleTest, Audio_Renderer_Drain_002, TestSize.Level1)
{
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    bool isDrained = audioRenderer->Drain();
    EXPECT_EQ(false, isDrained);

    audioRenderer->Release();
}

/**
* @tc.name  : Test Drain API via illegal state, RENDERER_PREPARED: Without Start.
* @tc.number: Audio_Renderer_Drain_003
* @tc.desc  : Test Drain interface. Returns false, if the renderer state is not RENDERER_RUNNING.
*/
HWTEST(AudioRendererModuleTest, Audio_Renderer_Drain_003, TestSize.Level1)
{
    int32_t ret = -1;
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    ret = AudioRendererModuleTest::InitializeRenderer(audioRenderer);
    EXPECT_EQ(SUCCESS, ret);

    bool isDrained = audioRenderer->Drain();
    EXPECT_EQ(false, isDrained);

    audioRenderer->Release();
}

/**
* @tc.name  : Test Drain API via illegal state, RENDERER_STOPPED: call Stop before Drain.
* @tc.number: Audio_Renderer_Drain_004
* @tc.desc  : Test Drain interface. Returns false, if the renderer state is not RENDERER_RUNNING.
*/
HWTEST(AudioRendererModuleTest, Audio_Renderer_Drain_004, TestSize.Level1)
{
    int32_t ret = -1;
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    ret = AudioRendererModuleTest::InitializeRenderer(audioRenderer);
    EXPECT_EQ(SUCCESS, ret);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    bool isStopped = audioRenderer->Stop();
    EXPECT_EQ(true, isStopped);

    bool isDrained = audioRenderer->Drain();
    EXPECT_EQ(false, isDrained);

    audioRenderer->Release();
}

/**
* @tc.name  : Test Drain API via illegal state, RENDERER_RELEASED: call Release before Drain.
* @tc.number: Audio_Renderer_Drain_005
* @tc.desc  : Test Drain interface. Returns false, if the renderer state is not RENDERER_RUNNING.
*/
HWTEST(AudioRendererModuleTest, Audio_Renderer_Drain_005, TestSize.Level1)
{
    int32_t ret = -1;
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    ret = AudioRendererModuleTest::InitializeRenderer(audioRenderer);
    EXPECT_EQ(SUCCESS, ret);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    bool isReleased = audioRenderer->Release();
    EXPECT_EQ(true, isReleased);

    bool isDrained = audioRenderer->Drain();
    EXPECT_EQ(false, isDrained);

    audioRenderer->Release();
}

/**
* @tc.name  : Test Flush API.
* @tc.number: Audio_Renderer_Flush_001
* @tc.desc  : Test Flush interface. Returns true, if the flush is successful.
*/
HWTEST(AudioRendererModuleTest, Audio_Renderer_Flush_001, TestSize.Level1)
{
    int32_t ret = -1;
    FILE *wavFile = fopen(AUDIORENDER_TEST_FILE_PATH.c_str(), "rb");
    ASSERT_NE(nullptr, wavFile);

    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    ret = AudioRendererModuleTest::InitializeRenderer(audioRenderer);
    EXPECT_EQ(SUCCESS, ret);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    size_t bufferLen;
    ret = audioRenderer->GetBufferSize(bufferLen);
    EXPECT_EQ(SUCCESS, ret);

    uint8_t *buffer = (uint8_t *) malloc(bufferLen);
    ASSERT_NE(nullptr, buffer);

    size_t bytesToWrite = fread(buffer, 1, bufferLen, wavFile);
    int32_t bytesWritten = audioRenderer->Write(buffer, bytesToWrite);
    EXPECT_GE(bytesWritten, VALUE_ZERO);

    bool isFlushed = audioRenderer->Flush();
    EXPECT_EQ(true, isFlushed);

    audioRenderer->Release();

    free(buffer);
    fclose(wavFile);
}

/**
* @tc.name  : Test Flush API.
* @tc.number: Audio_Renderer_Flush_002
* @tc.desc  : Test Flush interface after Pause call. Returns true, if the flush is successful.
*/
HWTEST(AudioRendererModuleTest, Audio_Renderer_Flush_002, TestSize.Level1)
{
    int32_t ret = -1;
    FILE *wavFile = fopen(AUDIORENDER_TEST_FILE_PATH.c_str(), "rb");
    ASSERT_NE(nullptr, wavFile);

    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    ret = AudioRendererModuleTest::InitializeRenderer(audioRenderer);
    EXPECT_EQ(SUCCESS, ret);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    size_t bufferLen;
    ret = audioRenderer->GetBufferSize(bufferLen);
    EXPECT_EQ(SUCCESS, ret);

    uint8_t *buffer = (uint8_t *) malloc(bufferLen);
    ASSERT_NE(nullptr, buffer);

    size_t bytesToWrite = fread(buffer, 1, bufferLen, wavFile);
    int32_t bytesWritten = audioRenderer->Write(buffer, bytesToWrite);
    EXPECT_GE(bytesWritten, VALUE_ZERO);

    audioRenderer->Pause();

    bool isFlushed = audioRenderer->Flush();
    EXPECT_EQ(true, isFlushed);

    audioRenderer->Release();

    free(buffer);
    fclose(wavFile);
}

/**
* @tc.name  : Test Flush API via illegal state, RENDERER_NEW: Without initializing the renderer.
* @tc.number: Audio_Renderer_Flush_003
* @tc.desc  : Test Flush interface. Returns false, if the renderer state is not RENDERER_RUNNING or RENDERER_PAUSED.
*/
HWTEST(AudioRendererModuleTest, Audio_Renderer_Flush_003, TestSize.Level1)
{
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    bool isFlushed = audioRenderer->Flush();
    EXPECT_EQ(false, isFlushed);
}

/**
* @tc.name  : Test Flush API via illegal state, RENDERER_PREPARED: Without Start.
* @tc.number: Audio_Renderer_Flush_004
* @tc.desc  : Test Flush interface. Returns false, if the renderer state is not RENDERER_RUNNING or RENDERER_PAUSED.
*/
HWTEST(AudioRendererModuleTest, Audio_Renderer_Flush_004, TestSize.Level1)
{
    int32_t ret = -1;
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    ret = AudioRendererModuleTest::InitializeRenderer(audioRenderer);
    EXPECT_EQ(SUCCESS, ret);

    bool isFlushed = audioRenderer->Flush();
    EXPECT_EQ(false, isFlushed);

    audioRenderer->Release();
}

/**
* @tc.name  : Test Flush API via illegal state, RENDERER_STOPPED: call Stop before Flush.
* @tc.number: Audio_Renderer_Flush_005
* @tc.desc  : Test Flush interface. Returns false, if the renderer state is not RENDERER_RUNNING or RENDERER_PAUSED.
*/
HWTEST(AudioRendererModuleTest, Audio_Renderer_Flush_005, TestSize.Level1)
{
    int32_t ret = -1;
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    ret = AudioRendererModuleTest::InitializeRenderer(audioRenderer);
    EXPECT_EQ(SUCCESS, ret);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    bool isStopped = audioRenderer->Stop();
    EXPECT_EQ(true, isStopped);

    bool isFlushed = audioRenderer->Flush();
    EXPECT_EQ(false, isFlushed);

    audioRenderer->Release();
}

/**
* @tc.name  : Test Flush API via illegal state, RENDERER_RELEASED: call Release before Flush.
* @tc.number: Audio_Renderer_Flush_006
* @tc.desc  : Test Flush interface. Returns false, if the renderer state is not RENDERER_RUNNING.
*/
HWTEST(AudioRendererModuleTest, Audio_Renderer_Flush_006, TestSize.Level1)
{
    int32_t ret = -1;
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    ret = AudioRendererModuleTest::InitializeRenderer(audioRenderer);
    EXPECT_EQ(SUCCESS, ret);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    bool isReleased = audioRenderer->Release();
    EXPECT_EQ(true, isReleased);

    bool isFlushed = audioRenderer->Flush();
    EXPECT_EQ(false, isFlushed);
}

/**
* @tc.name  : Test Pause API.
* @tc.number: Audio_Renderer_Pause_001
* @tc.desc  : Test Pause interface. Returns true, if the pause is successful.
*/
HWTEST(AudioRendererModuleTest, Audio_Renderer_Pause_001, TestSize.Level1)
{
    int32_t ret = -1;
    FILE *wavFile = fopen(AUDIORENDER_TEST_FILE_PATH.c_str(), "rb");
    ASSERT_NE(nullptr, wavFile);

    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    ret = AudioRendererModuleTest::InitializeRenderer(audioRenderer);
    EXPECT_EQ(SUCCESS, ret);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    size_t bufferLen;
    ret = audioRenderer->GetBufferSize(bufferLen);
    EXPECT_EQ(SUCCESS, ret);

    uint8_t *buffer = (uint8_t *) malloc(bufferLen);
    ASSERT_NE(nullptr, buffer);

    size_t bytesToWrite = fread(buffer, 1, bufferLen, wavFile);
    int32_t bytesWritten = audioRenderer->Write(buffer, bytesToWrite);
    EXPECT_GE(bytesWritten, VALUE_ZERO);

    audioRenderer->Drain();

    bool isPaused = audioRenderer->Pause();
    EXPECT_EQ(true, isPaused);

    audioRenderer->Release();

    free(buffer);
    fclose(wavFile);
}

/**
* @tc.name  : Test Pause API via illegal state, RENDERER_NEW: call Pause without Initializing the renderer.
* @tc.number: Audio_Renderer_Pause_002
* @tc.desc  : Test Pause interface. Returns false, if the renderer state is not RENDERER_RUNNING.
*/
HWTEST(AudioRendererModuleTest, Audio_Renderer_Pause_002, TestSize.Level1)
{
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    bool isPaused = audioRenderer->Pause();
    EXPECT_EQ(false, isPaused);
}

/**
* @tc.name  : Test Pause API via illegal state, RENDERER_PREPARED: call Pause without Start.
* @tc.number: Audio_Renderer_Pause_003
* @tc.desc  : Test Pause interface. Returns false, if the renderer state is not RENDERER_RUNNING.
*/
HWTEST(AudioRendererModuleTest, Audio_Renderer_Pause_003, TestSize.Level1)
{
    int32_t ret = -1;
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    ret = AudioRendererModuleTest::InitializeRenderer(audioRenderer);
    EXPECT_EQ(SUCCESS, ret);

    bool isPaused = audioRenderer->Pause();
    EXPECT_EQ(false, isPaused);

    audioRenderer->Release();
}

/**
* @tc.name  : Test Pause API via illegal state, RENDERER_RELEASED: call Pause after Release.
* @tc.number: Audio_Renderer_Pause_004
* @tc.desc  : Test Pause interface. Returns false, if the renderer state is not RENDERER_RUNNING.
*/
HWTEST(AudioRendererModuleTest, Audio_Renderer_Pause_004, TestSize.Level1)
{
    int32_t ret = -1;
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    ret = AudioRendererModuleTest::InitializeRenderer(audioRenderer);
    EXPECT_EQ(SUCCESS, ret);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    bool isReleased = audioRenderer->Release();
    EXPECT_EQ(true, isReleased);

    bool isPaused = audioRenderer->Pause();
    EXPECT_EQ(false, isPaused);
}

/**
* @tc.name  : Test Pause and resume
* @tc.number: Audio_Renderer_Pause_005
* @tc.desc  : Test Pause interface. Returns true , if the pause is successful.
*/
HWTEST(AudioRendererModuleTest, Audio_Renderer_Pause_005, TestSize.Level1)
{
    int32_t ret = -1;
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    ret = AudioRendererModuleTest::InitializeRenderer(audioRenderer);
    EXPECT_EQ(SUCCESS, ret);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    bool isPaused = audioRenderer->Pause();
    EXPECT_EQ(true, isPaused);

    isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);
}

/**
* @tc.name  : Test Stop API.
* @tc.number: Audio_Renderer_Stop_001
* @tc.desc  : Test Stop interface. Returns true, if the stop is successful.
*/
HWTEST(AudioRendererModuleTest, Audio_Renderer_Stop_001, TestSize.Level1)
{
    int32_t ret = -1;
    FILE *wavFile = fopen(AUDIORENDER_TEST_FILE_PATH.c_str(), "rb");
    ASSERT_NE(nullptr, wavFile);

    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    ret = AudioRendererModuleTest::InitializeRenderer(audioRenderer);
    EXPECT_EQ(SUCCESS, ret);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    size_t bufferLen;
    ret = audioRenderer->GetBufferSize(bufferLen);
    EXPECT_EQ(SUCCESS, ret);

    uint8_t *buffer = (uint8_t *) malloc(bufferLen);
    ASSERT_NE(nullptr, buffer);

    size_t bytesToWrite = fread(buffer, 1, bufferLen, wavFile);
    int32_t bytesWritten = audioRenderer->Write(buffer, bytesToWrite);
    EXPECT_GE(bytesWritten, VALUE_ZERO);

    audioRenderer->Drain();

    bool isStopped = audioRenderer->Stop();
    EXPECT_EQ(true, isStopped);

    audioRenderer->Release();

    free(buffer);
    fclose(wavFile);
}

/**
* @tc.name  : Test Stop API via illegal state, RENDERER_NEW: call Stop without Initializing the renderer.
* @tc.number: Audio_Renderer_Stop_002
* @tc.desc  : Test Stop interface. Returns false, if the renderer state is not RENDERER_RUNNING.
*/
HWTEST(AudioRendererModuleTest, Audio_Renderer_Stop_002, TestSize.Level1)
{
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    bool isStopped = audioRenderer->Stop();
    EXPECT_EQ(false, isStopped);
}

/**
* @tc.name  : Test Stop API via illegal state, RENDERER_PREPARED: call Stop without Start.
* @tc.number: Audio_Renderer_Stop_003
* @tc.desc  : Test Stop interface. Returns false, if the renderer state is not RENDERER_RUNNING.
*/
HWTEST(AudioRendererModuleTest, Audio_Renderer_Stop_003, TestSize.Level1)
{
    int32_t ret = -1;
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    ret = AudioRendererModuleTest::InitializeRenderer(audioRenderer);
    EXPECT_EQ(SUCCESS, ret);

    bool isStopped = audioRenderer->Stop();
    EXPECT_EQ(false, isStopped);

    audioRenderer->Release();
}

/**
* @tc.name  : Test Stop API via illegal state, RENDERER_RELEASED: call Stop after Release.
* @tc.number: Audio_Renderer_Stop_004
* @tc.desc  : Test Stop interface. Returns false, if the renderer state is not RENDERER_RUNNING.
*/
HWTEST(AudioRendererModuleTest, Audio_Renderer_Stop_004, TestSize.Level1)
{
    int32_t ret = -1;
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    ret = AudioRendererModuleTest::InitializeRenderer(audioRenderer);
    EXPECT_EQ(SUCCESS, ret);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    bool isReleased = audioRenderer->Release();
    EXPECT_EQ(true, isReleased);

    bool isStopped = audioRenderer->Stop();
    EXPECT_EQ(false, isStopped);
}

/**
* @tc.name  : Test Stop API via legal state. call Start, Stop, Start and Stop again
* @tc.number: Audio_Renderer_Stop_005
* @tc.desc  : Test Stop interface. Returns true , if the stop is successful.
*/
HWTEST(AudioRendererModuleTest, Audio_Renderer_Stop_005, TestSize.Level1)
{
    int32_t ret = -1;
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    ret = AudioRendererModuleTest::InitializeRenderer(audioRenderer);
    EXPECT_EQ(SUCCESS, ret);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    bool isStopped = audioRenderer->Stop();
    EXPECT_EQ(true, isStopped);

    isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    isStopped = audioRenderer->Stop();
    EXPECT_EQ(true, isStopped);
}

/**
* @tc.name  : Test Release API.
* @tc.number: Audio_Renderer_Release_001
* @tc.desc  : Test Release interface. Returns true, if the release is successful.
*/
HWTEST(AudioRendererModuleTest, Audio_Renderer_Release_001, TestSize.Level1)
{
    int32_t ret = -1;
    FILE *wavFile = fopen(AUDIORENDER_TEST_FILE_PATH.c_str(), "rb");
    ASSERT_NE(nullptr, wavFile);

    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    ret = AudioRendererModuleTest::InitializeRenderer(audioRenderer);
    EXPECT_EQ(SUCCESS, ret);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    size_t bufferLen;
    ret = audioRenderer->GetBufferSize(bufferLen);
    EXPECT_EQ(SUCCESS, ret);

    uint8_t *buffer = (uint8_t *) malloc(bufferLen);
    ASSERT_NE(nullptr, buffer);

    size_t bytesToWrite = fread(buffer, 1, bufferLen, wavFile);
    int32_t bytesWritten = audioRenderer->Write(buffer, bytesToWrite);
    EXPECT_GE(bytesWritten, VALUE_ZERO);

    audioRenderer->Drain();
    audioRenderer->Stop();

    bool isReleased = audioRenderer->Release();
    EXPECT_EQ(true, isReleased);

    free(buffer);
    fclose(wavFile);
}

/**
* @tc.name  : Test Release API via illegal state, RENDERER_NEW: Call Release without initializing the renderer.
* @tc.number: Audio_Renderer_Release_002
* @tc.desc  : Test Release interface, Returns false, if the state is RENDERER_NEW.
*/
HWTEST(AudioRendererModuleTest, Audio_Renderer_Release_002, TestSize.Level1)
{
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    bool isReleased = audioRenderer->Release();
    EXPECT_EQ(false, isReleased);
}

/**
* @tc.name  : Test Release API via illegal state, RENDERER_RELEASED: call Release repeatedly.
* @tc.number: Audio_Renderer_Release_003
* @tc.desc  : Test Release interface. Returns false, if the state is already RENDERER_RELEASED.
*/
HWTEST(AudioRendererModuleTest, Audio_Renderer_Release_003, TestSize.Level1)
{
    int32_t ret = -1;
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    ret = AudioRendererModuleTest::InitializeRenderer(audioRenderer);
    EXPECT_EQ(SUCCESS, ret);

    bool isReleased = audioRenderer->Release();
    EXPECT_EQ(true, isReleased);

    isReleased = audioRenderer->Release();
    EXPECT_EQ(false, isReleased);
}

/**
* @tc.name  : Test Release API via legal state, RENDERER_RUNNING: call Release after Start
* @tc.number: Audio_Renderer_Release_004
* @tc.desc  : Test Release interface. Returns true, if the release is successful.
*/
HWTEST(AudioRendererModuleTest, Audio_Renderer_Release_004, TestSize.Level1)
{
    int32_t ret = -1;
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    ret = AudioRendererModuleTest::InitializeRenderer(audioRenderer);
    EXPECT_EQ(SUCCESS, ret);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    bool isReleased = audioRenderer->Release();
    EXPECT_EQ(true, isReleased);
}

/**
* @tc.name  : Test Release API via legal state, RENDERER_STOPPED: call release after Start and Stop
* @tc.number: Audio_Renderer_Release_005
* @tc.desc  : Test Release interface. Returns true, if the release is successful.
*/
HWTEST(AudioRendererModuleTest, Audio_Renderer_Release_005, TestSize.Level1)
{
    int32_t ret = -1;
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    ret = AudioRendererModuleTest::InitializeRenderer(audioRenderer);
    EXPECT_EQ(SUCCESS, ret);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    bool isStopped = audioRenderer->Stop();
    EXPECT_EQ(true, isStopped);

    bool isReleased = audioRenderer->Release();
    EXPECT_EQ(true, isReleased);
}

/**
* @tc.name  : Test GetStatus API.
* @tc.number: Audio_Renderer_GetStatus_001
* @tc.desc  : Test GetStatus interface. Returns correct state on success.
*/
HWTEST(AudioRendererModuleTest, Audio_Renderer_GetStatus_001, TestSize.Level1)
{
    int32_t ret = -1;
    RendererState state = RENDERER_INVALID;

    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);
    state = audioRenderer->GetStatus();
    EXPECT_EQ(RENDERER_NEW, state);

    AudioRendererParams rendererParams;
    rendererParams.sampleFormat = SAMPLE_S16LE;
    rendererParams.sampleRate = SAMPLE_RATE_44100;
    rendererParams.channelCount = STEREO;
    rendererParams.encodingType = ENCODING_PCM;
    ret = audioRenderer->SetParams(rendererParams);
    EXPECT_EQ(SUCCESS, ret);
    state = audioRenderer->GetStatus();
    EXPECT_EQ(RENDERER_PREPARED, state);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);
    state = audioRenderer->GetStatus();
    EXPECT_EQ(RENDERER_RUNNING, state);

    bool isStopped = audioRenderer->Stop();
    EXPECT_EQ(true, isStopped);
    state = audioRenderer->GetStatus();
    EXPECT_EQ(RENDERER_STOPPED, state);

    bool isReleased = audioRenderer->Release();
    EXPECT_EQ(true, isReleased);
    state = audioRenderer->GetStatus();
    EXPECT_EQ(RENDERER_RELEASED, state);
}
/**
* @tc.name  : Test GetStatus API, call Start without Initializing the renderer
* @tc.number: Audio_Renderer_GetStatus_002
* @tc.desc  : Test GetStatus interface. Not changes to RENDERER_RUNNING, if the current state is RENDERER_NEW.
*/
HWTEST(AudioRendererModuleTest, Audio_Renderer_GetStatus_002, TestSize.Level1)
{
    RendererState state = RENDERER_INVALID;

    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(false, isStarted);
    state = audioRenderer->GetStatus();
    EXPECT_NE(RENDERER_RUNNING, state);
    EXPECT_EQ(RENDERER_NEW, state);
}

/**
* @tc.name  : Test GetStatus API, call Stop without Start
* @tc.desc  : Test GetStatus interface. Not changes to RENDERER_STOPPED, if the current state is RENDERER_PREPARED.
*/
HWTEST(AudioRendererModuleTest, Audio_Renderer_GetStatus_003, TestSize.Level1)
{
    int32_t ret = -1;
    RendererState state = RENDERER_INVALID;

    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    ret = AudioRendererModuleTest::InitializeRenderer(audioRenderer);
    EXPECT_EQ(SUCCESS, ret);

    bool isStopped = audioRenderer->Stop();
    EXPECT_EQ(false, isStopped);
    state = audioRenderer->GetStatus();
    EXPECT_NE(RENDERER_STOPPED, state);
    EXPECT_EQ(RENDERER_PREPARED, state);

    audioRenderer->Release();
}

/**
* @tc.name  : Test GetStatus API, call Start, Stop and then Start again
* @tc.number: Audio_Renderer_GetStatus_004
* @tc.desc  : Test GetStatus interface.  Returns correct state on success.
*/
HWTEST(AudioRendererModuleTest, Audio_Renderer_GetStatus_004, TestSize.Level1)
{
    int32_t ret = -1;
    RendererState state = RENDERER_INVALID;

    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    ret = AudioRendererModuleTest::InitializeRenderer(audioRenderer);
    EXPECT_EQ(SUCCESS, ret);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);
    state = audioRenderer->GetStatus();
    EXPECT_EQ(RENDERER_RUNNING, state);

    bool isStopped = audioRenderer->Stop();
    EXPECT_EQ(true, isStopped);
    state = audioRenderer->GetStatus();
    EXPECT_EQ(RENDERER_STOPPED, state);

    isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);
    state = audioRenderer->GetStatus();
    EXPECT_EQ(RENDERER_RUNNING, state);

    audioRenderer->Release();
}

/**
* @tc.name  : Test GetStatus API, call Release without initializing
* @tc.number: Audio_Renderer_GetStatus_005
* @tc.desc  : Test GetStatus interface. Not changes to RENDERER_RELEASED, if the current state is RENDERER_NEW.
*/
HWTEST(AudioRendererModuleTest, Audio_Renderer_GetStatus_005, TestSize.Level1)
{
    RendererState state = RENDERER_INVALID;

    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    bool isReleased = audioRenderer->Release();
    EXPECT_EQ(false, isReleased);
    state = audioRenderer->GetStatus();
    EXPECT_NE(RENDERER_RELEASED, state);
    EXPECT_EQ(RENDERER_NEW, state);
}

/**
* @tc.name  : Test GetLatency API.
* @tc.number: Audio_Renderer_GetLatency_001
* @tc.desc  : Test GetLatency interface. Returns 0 {SUCCESS}, if the getting is successful.
*/
HWTEST(AudioRendererModuleTest, Audio_Renderer_GetLatency_001, TestSize.Level1)
{
    int32_t ret = -1;
    FILE *wavFile = fopen(AUDIORENDER_TEST_FILE_PATH.c_str(), "rb");
    ASSERT_NE(nullptr, wavFile);

    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    ret = AudioRendererModuleTest::InitializeRenderer(audioRenderer);
    EXPECT_EQ(SUCCESS, ret);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    size_t bufferLen;
    ret = audioRenderer->GetBufferSize(bufferLen);
    EXPECT_EQ(SUCCESS, ret);

    uint8_t *buffer = (uint8_t *) malloc(bufferLen);
    ASSERT_NE(nullptr, buffer);

    size_t bytesToWrite = 0;
    int32_t bytesWritten = 0;
    size_t minBytes = 4;
    int32_t numBuffersToRender = WRITE_BUFFERS_COUNT;

    while (numBuffersToRender) {
        bytesToWrite = fread(buffer, 1, bufferLen, wavFile);
        bytesWritten = 0;
        uint64_t latency;
        ret = audioRenderer->GetLatency(latency);
        EXPECT_EQ(SUCCESS, ret);
        while ((static_cast<size_t>(bytesWritten) < bytesToWrite) &&
            ((static_cast<size_t>(bytesToWrite) - bytesWritten) > minBytes)) {
            bytesWritten += audioRenderer->Write(buffer + static_cast<size_t>(bytesWritten),
                                                 bytesToWrite - static_cast<size_t>(bytesWritten));
            EXPECT_GE(bytesWritten, VALUE_ZERO);
            if (bytesWritten < 0) {
                break;
            }
        }
        numBuffersToRender--;
    }

    audioRenderer->Drain();
    audioRenderer->Release();

    free(buffer);
    fclose(wavFile);
}

/**
* @tc.name  : Test GetLatency API via illegal state, RENDERER_NEW: without initializing the renderer
* @tc.number: Audio_Renderer_GetLatency_002
* @tc.desc  : Test GetLatency interface. Returns error code, if the renderer state is RENDERER_NEW.
*/
HWTEST(AudioRendererModuleTest, Audio_Renderer_GetLatency_002, TestSize.Level1)
{
    int32_t ret = -1;

    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(false, isStarted);

    uint64_t latency;
    ret = audioRenderer->GetLatency(latency);
    EXPECT_EQ(ERR_OPERATION_FAILED, ret);
}

/**
* @tc.name  : Test GetLatency API via legal state, RENDERER_PREPARED
* @tc.number: Audio_Renderer_GetLatency_003
* @tc.desc  : Test GetLatency interface. Returns 0 {SUCCESS}, if the getting is successful.
*/
HWTEST(AudioRendererModuleTest, Audio_Renderer_GetLatency_003, TestSize.Level1)
{
    int32_t ret = -1;

    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    ret = AudioRendererModuleTest::InitializeRenderer(audioRenderer);
    EXPECT_EQ(SUCCESS, ret);

    uint64_t latency;
    ret = audioRenderer->GetLatency(latency);
    EXPECT_EQ(SUCCESS, ret);

    audioRenderer->Release();
}

/**
* @tc.name  : Test GetLatency API via legal state, RENDERER_STOPPED: After Stop
* @tc.number: Audio_Renderer_GetLatency_004
* @tc.desc  : Test GetLatency interface. Returns 0 {SUCCESS}, if the getting is successful.
*/
HWTEST(AudioRendererModuleTest, Audio_Renderer_GetLatency_004, TestSize.Level1)
{
    int32_t ret = -1;

    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    ret = AudioRendererModuleTest::InitializeRenderer(audioRenderer);
    EXPECT_EQ(SUCCESS, ret);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    bool isStopped = audioRenderer->Stop();
    EXPECT_EQ(true, isStopped);

    uint64_t latency;
    ret = audioRenderer->GetLatency(latency);
    EXPECT_EQ(SUCCESS, ret);

    audioRenderer->Release();
}

/**
* @tc.name  : Test GetLatency API via illegal state, RENDERER_RELEASED: After Release
* @tc.number: Audio_Renderer_GetLatency_005
* @tc.desc  : Test GetLatency interface. Returns error code, if the renderer state is RENDERER_RELEASED.
*/
HWTEST(AudioRendererModuleTest, Audio_Renderer_GetLatency_005, TestSize.Level1)
{
    int32_t ret = -1;

    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    ret = AudioRendererModuleTest::InitializeRenderer(audioRenderer);
    EXPECT_EQ(SUCCESS, ret);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    bool isReleased = audioRenderer->Release();
    EXPECT_EQ(true, isReleased);

    uint64_t latency;
    ret = audioRenderer->GetLatency(latency);
    EXPECT_EQ(ERR_OPERATION_FAILED, ret);
}
} // namespace AudioStandard
} // namespace OHOS
