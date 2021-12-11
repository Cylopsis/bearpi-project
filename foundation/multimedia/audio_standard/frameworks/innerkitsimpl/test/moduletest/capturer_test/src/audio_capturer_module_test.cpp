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

#include "audio_capturer_module_test.h"

#include "audio_capturer.h"
#include "audio_errors.h"
#include "audio_info.h"

using namespace std;
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
namespace {
    const string AUDIO_CAPTURE_FILE1 = "/data/audiocapturetest_blocking.pcm";
    const string AUDIO_CAPTURE_FILE2 = "/data/audiocapturetest_nonblocking.pcm";
    const int32_t READ_BUFFERS_COUNT = 128;
    const int32_t VALUE_ZERO = 0;
} // namespace

void AudioCapturerModuleTest::SetUpTestCase(void) {}
void AudioCapturerModuleTest::TearDownTestCase(void) {}
void AudioCapturerModuleTest::SetUp(void) {}
void AudioCapturerModuleTest::TearDown(void) {}

int32_t AudioCapturerModuleTest::InitializeCapturer(unique_ptr<AudioCapturer> &audioCapturer)
{
    AudioCapturerParams capturerParams;
    capturerParams.audioSampleFormat = SAMPLE_S16LE;
    capturerParams.samplingRate = SAMPLE_RATE_44100;
    capturerParams.audioChannel = STEREO;
    capturerParams.audioEncoding = ENCODING_PCM;

    return audioCapturer->SetParams(capturerParams);
}

/**
* @tc.name  : Test GetSupportedFormats API
* @tc.number: Audio_Capturer_GetSupportedFormats_001
* @tc.desc  : Test GetSupportedFormats interface. Returns supported Formats on success.
*/
HWTEST(AudioCapturerModuleTest, Audio_Capturer_GetSupportedFormats_001, TestSize.Level1)
{
    vector<AudioSampleFormat> supportedFormatList = AudioCapturer::GetSupportedFormats();
    EXPECT_EQ(AUDIO_SUPPORTED_FORMATS.size(), supportedFormatList.size());
}

/**
* @tc.name  : Test GetSupportedChannels API
* @tc.number: Audio_Capturer_GetSupportedChannels_001
* @tc.desc  : Test GetSupportedChannels interface. Returns supported Channels on success.
*/
HWTEST(AudioCapturerModuleTest, Audio_Capturer_GetSupportedChannels_001, TestSize.Level1)
{
    vector<AudioChannel> supportedChannelList = AudioCapturer::GetSupportedChannels();
    EXPECT_EQ(AUDIO_SUPPORTED_CHANNELS.size(), supportedChannelList.size());
}

/**
* @tc.name  : Test GetSupportedEncodingTypes API
* @tc.number: Audio_Capturer_GetSupportedEncodingTypes_001
* @tc.desc  : Test GetSupportedEncodingTypes interface. Returns supported Encoding types on success.
*/
HWTEST(AudioCapturerModuleTest, Audio_Capturer_GetSupportedEncodingTypes_001, TestSize.Level1)
{
    vector<AudioEncodingType> supportedEncodingTypes
                                        = AudioCapturer::GetSupportedEncodingTypes();
    EXPECT_EQ(AUDIO_SUPPORTED_ENCODING_TYPES.size(), supportedEncodingTypes.size());
}

/**
* @tc.name  : Test GetSupportedSamplingRates API
* @tc.number: Audio_Capturer_GetSupportedSamplingRates_001
* @tc.desc  : Test GetSupportedSamplingRates interface. Returns supported Sampling rates on success.
*/
HWTEST(AudioCapturerModuleTest, Audio_Capturer_GetSupportedSamplingRates_001, TestSize.Level1)
{
    vector<AudioSamplingRate> supportedSamplingRates = AudioCapturer::GetSupportedSamplingRates();
    EXPECT_EQ(AUDIO_SUPPORTED_SAMPLING_RATES.size(), supportedSamplingRates.size());
}

/**
* @tc.name  : Test Create API via legal input.
* @tc.number: Audio_Capturer_Create_001
* @tc.desc  : Test Create interface with STREAM_MUSIC. Returns audioCapturer instance, if create is successful.
*/
HWTEST(AudioCapturerModuleTest, Audio_Capturer_Create_001, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(STREAM_MUSIC);
    EXPECT_NE(nullptr, audioCapturer);
}

/**
* @tc.name  : Test Create API via legal input.
* @tc.number: Audio_Capturer_Create_002
* @tc.desc  : Test Create interface with STREAM_RING. Returns audioCapturer instance, if create is successful.
*/
HWTEST(AudioCapturerModuleTest, Audio_Capturer_Create_002, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(STREAM_RING);
    EXPECT_NE(nullptr, audioCapturer);
}

/**
* @tc.name  : Test Create API via legal input.
* @tc.number: Audio_Capturer_Create_003
* @tc.desc  : Test Create interface with STREAM_VOICE_CALL. Returns audioCapturer instance if create is successful.
*             Note: instance will be created but functional support for STREAM_VOICE_CALL not available yet.
*/
HWTEST(AudioCapturerModuleTest, Audio_Capturer_Create_003, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(STREAM_VOICE_CALL);
    EXPECT_NE(nullptr, audioCapturer);
}

/**
* @tc.name  : Test Create API via legal input.
* @tc.number: Audio_Capturer_Create_004
* @tc.desc  : Test Create interface with STREAM_SYSTEM. Returns audioCapturer instance, if create is successful.
*             Note: instance will be created but functional support for STREAM_SYSTEM not available yet.
*/
HWTEST(AudioCapturerModuleTest, Audio_Capturer_Create_004, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(STREAM_SYSTEM);
    EXPECT_NE(nullptr, audioCapturer);
}

/**
* @tc.name  : Test Create API via legal input.
* @tc.number: Audio_Capturer_Create_005
* @tc.desc  : Test Create interface with STREAM_BLUETOOTH_SCO. Returns audioCapturer instance, if create is successful.
*             Note: instance will be created but functional support for STREAM_BLUETOOTH_SCO not available yet
*/
HWTEST(AudioCapturerModuleTest, Audio_Capturer_Create_005, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(STREAM_BLUETOOTH_SCO);
    EXPECT_NE(nullptr, audioCapturer);
}

/**
* @tc.name  : Test Create API via legal input.
* @tc.number: Audio_Capturer_Create_006
* @tc.desc  : Test Create interface with STREAM_ALARM. Returns audioCapturer instance, if create is successful.
*             Note: instance will be created but functional support for STREAM_ALARM not available yet.
*/
HWTEST(AudioCapturerModuleTest, Audio_Capturer_Create_006, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(STREAM_ALARM);
    EXPECT_NE(nullptr, audioCapturer);
}

/**
* @tc.name  : Test Create API via legal input.
* @tc.number: Audio_Capturer_Create_007
* @tc.desc  : Test Create interface with STREAM_NOTIFICATION. Returns audioCapturer instance, if create is successful.
*             Note: instance will be created but functional support for STREAM_NOTIFICATION not available yet.
*/
HWTEST(AudioCapturerModuleTest, Audio_Capturer_Create_007, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(STREAM_NOTIFICATION);
    EXPECT_NE(nullptr, audioCapturer);
}

/**
* @tc.name  : Test SetParams API via legal input
* @tc.number: Audio_Capturer_SetParams_001
* @tc.desc  : Test SetParams interface. Returns 0 {SUCCESS}, if the setting is successful.
*             capturerParams.audioSampleFormat = SAMPLE_S16LE;
*             capturerParams.samplingRate = SAMPLE_RATE_44100;
*             capturerParams.audioChannel = STEREO;
*             capturerParams.audioEncoding = ENCODING_PCM;
*/
HWTEST(AudioCapturerModuleTest, Audio_Capturer_SetParams_001, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioCapturer);

    AudioCapturerParams capturerParams;
    capturerParams.audioSampleFormat = SAMPLE_S16LE;
    capturerParams.samplingRate = SAMPLE_RATE_44100;
    capturerParams.audioChannel = STEREO;
    capturerParams.audioEncoding = ENCODING_PCM;

    int32_t ret = audioCapturer->SetParams(capturerParams);
    EXPECT_EQ(SUCCESS, ret);
    audioCapturer->Release();
}

/**
* @tc.name  : Test SetParams API via legal input.
* @tc.number: Audio_Capturer_SetParams_002
* @tc.desc  : Test SetParams interface. Returns 0 {SUCCESS}, if the setting is successful.
*             capturerParams.audioSampleFormat = SAMPLE_S16LE;
*             capturerParams.samplingRate = SAMPLE_RATE_8000;
*             capturerParams.audioChannel = MONO;
*             capturerParams.audioEncoding = ENCODING_PCM;
*/
HWTEST(AudioCapturerModuleTest, Audio_Capturer_SetParams_002, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioCapturer);

    AudioCapturerParams capturerParams;
    capturerParams.audioSampleFormat = SAMPLE_S16LE;
    capturerParams.samplingRate = SAMPLE_RATE_8000;
    capturerParams.audioChannel = MONO;
    capturerParams.audioEncoding = ENCODING_PCM;

    int32_t ret = audioCapturer->SetParams(capturerParams);
    EXPECT_EQ(SUCCESS, ret);
    audioCapturer->Release();
}

/**
* @tc.name  : Test SetParams API via legal input.
* @tc.number: Audio_Capturer_SetParams_003
* @tc.desc  : Test SetParams interface. Returns 0 {SUCCESS}, if the setting is successful.
*             capturerParams.audioSampleFormat = SAMPLE_S16LE;
*             capturerParams.samplingRate = SAMPLE_RATE_11025;
*             capturerParams.audioChannel = STEREO;
*             capturerParams.audioEncoding = ENCODING_PCM;
*/
HWTEST(AudioCapturerModuleTest, Audio_Capturer_SetParams_003, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioCapturer);

    AudioCapturerParams capturerParams;
    capturerParams.audioSampleFormat = SAMPLE_S16LE;
    capturerParams.samplingRate = SAMPLE_RATE_11025;
    capturerParams.audioChannel = STEREO;
    capturerParams.audioEncoding = ENCODING_PCM;

    int32_t ret = audioCapturer->SetParams(capturerParams);
    EXPECT_EQ(SUCCESS, ret);
    audioCapturer->Release();
}

/**
* @tc.name  : Test SetParams API via legal input.
* @tc.number: Audio_Capturer_SetParams_004
* @tc.desc  : Test SetParams interface. Returns 0 {SUCCESS}, if the setting is successful.
*             capturerParams.audioSampleFormat = SAMPLE_S16LE;
*             capturerParams.samplingRate = SAMPLE_RATE_22050;
*             capturerParams.audioChannel = MONO;
*             capturerParams.audioEncoding = ENCODING_PCM;
*/
HWTEST(AudioCapturerModuleTest, Audio_Capturer_SetParams_004, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioCapturer);

    AudioCapturerParams capturerParams;
    capturerParams.audioSampleFormat = SAMPLE_S16LE;
    capturerParams.samplingRate = SAMPLE_RATE_22050;
    capturerParams.audioChannel = MONO;
    capturerParams.audioEncoding = ENCODING_PCM;

    int32_t ret = audioCapturer->SetParams(capturerParams);
    EXPECT_EQ(SUCCESS, ret);
    audioCapturer->Release();
}

/**
* @tc.name  : Test SetParams API via legal input.
* @tc.number: Audio_Capturer_SetParams_005
* @tc.desc  : Test SetParams interface. Returns 0 {SUCCESS}, if the setting is successful.
*             capturerParams.audioSampleFormat = SAMPLE_S16LE;
*             capturerParams.samplingRate = SAMPLE_RATE_96000;
*             capturerParams.audioChannel = MONO;
*             capturerParams.audioEncoding = ENCODING_PCM;
*/
HWTEST(AudioCapturerModuleTest, Audio_Capturer_SetParams_005, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioCapturer);

    AudioCapturerParams capturerParams;
    capturerParams.audioSampleFormat = SAMPLE_S16LE;
    capturerParams.samplingRate = SAMPLE_RATE_96000;
    capturerParams.audioChannel = MONO;
    capturerParams.audioEncoding = ENCODING_PCM;

    int32_t ret = audioCapturer->SetParams(capturerParams);
    EXPECT_EQ(SUCCESS, ret);
}

/**
* @tc.name  : Test SetParams API via legal input.
* @tc.number: Audio_Capturer_SetParams_006
* @tc.desc  : Test SetParams interface. Returns 0 {SUCCESS}, if the setting is successful.
*             capturerParams.audioSampleFormat = SAMPLE_S24LE;
*             capturerParams.samplingRate = SAMPLE_RATE_64000;
*             capturerParams.audioChannel = MONO;
*             capturerParams.audioEncoding = ENCODING_PCM;
*/
HWTEST(AudioCapturerModuleTest, Audio_Capturer_SetParams_006, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioCapturer);

    AudioCapturerParams capturerParams;
    capturerParams.audioSampleFormat = SAMPLE_S24LE;
    capturerParams.samplingRate = SAMPLE_RATE_64000;
    capturerParams.audioChannel = MONO;
    capturerParams.audioEncoding = ENCODING_PCM;

    int32_t ret = audioCapturer->SetParams(capturerParams);
    EXPECT_EQ(SUCCESS, ret);
}

/**
* @tc.name  : Test SetParams API via illegal input.
* @tc.number: Audio_Capturer_SetParams_007
* @tc.desc  : Test SetParams interface. Returns 0 {SUCCESS}, if the setting is successful.
*             capturerParams.audioSampleFormat = SAMPLE_S16LE;
*             capturerParams.samplingRate = SAMPLE_RATE_16000;
*             capturerParams.audioChannel = STEREO;
*             capturerParams.audioEncoding = ENCODING_PCM;
*/
HWTEST(AudioCapturerModuleTest, Audio_Capturer_SetParams_007, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioCapturer);

    AudioCapturerParams capturerParams;
    capturerParams.audioSampleFormat = SAMPLE_S16LE;
    capturerParams.samplingRate = SAMPLE_RATE_16000;
    capturerParams.audioChannel = STEREO;
    capturerParams.audioEncoding = ENCODING_PCM;

    int32_t ret = audioCapturer->SetParams(capturerParams);
    EXPECT_EQ(SUCCESS, ret);
    audioCapturer->Release();
}

/**
* @tc.name  : Test GetParams API via legal input.
* @tc.number: Audio_Capturer_GetParams_001
* @tc.desc  : Test GetParams interface. Returns 0 {SUCCESS}, if the getting is successful.
*/
HWTEST(AudioCapturerModuleTest, Audio_Capturer_GetParams_001, TestSize.Level1)
{
    int32_t ret = -1;
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioCapturer);

    AudioCapturerParams capturerParams;
    capturerParams.audioSampleFormat = SAMPLE_S16LE;
    capturerParams.samplingRate = SAMPLE_RATE_44100;
    capturerParams.audioChannel = STEREO;
    capturerParams.audioEncoding = ENCODING_PCM;
    ret = audioCapturer->SetParams(capturerParams);
    EXPECT_EQ(SUCCESS, ret);

    AudioCapturerParams getCapturerParams;
    ret = audioCapturer->GetParams(getCapturerParams);
    EXPECT_EQ(SUCCESS, ret);
    EXPECT_EQ(capturerParams.audioSampleFormat, getCapturerParams.audioSampleFormat);
    EXPECT_EQ(capturerParams.samplingRate, getCapturerParams.samplingRate);
    EXPECT_EQ(capturerParams.audioChannel, getCapturerParams.audioChannel);
    EXPECT_EQ(capturerParams.audioEncoding, getCapturerParams.audioEncoding);

    audioCapturer->Release();
}

/**
* @tc.name  : Test GetParams API via legal state, CAPTURER_RUNNING: GetParams after Start.
* @tc.number: Audio_Capturer_GetParams_002
* @tc.desc  : Test GetParams interface. Returns 0 {SUCCESS} if the getting is successful.
*/
HWTEST(AudioCapturerModuleTest, Audio_Capturer_GetParams_002, TestSize.Level1)
{
    int32_t ret = -1;
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioCapturer);

    AudioCapturerParams capturerParams;
    capturerParams.audioSampleFormat = SAMPLE_S16LE;
    capturerParams.samplingRate = SAMPLE_RATE_44100;
    capturerParams.audioChannel = MONO;
    capturerParams.audioEncoding = ENCODING_PCM;
    ret = audioCapturer->SetParams(capturerParams);
    EXPECT_EQ(SUCCESS, ret);

    bool isStarted = audioCapturer->Start();
    EXPECT_EQ(true, isStarted);

    AudioCapturerParams getCapturerParams;
    ret = audioCapturer->GetParams(getCapturerParams);
    EXPECT_EQ(SUCCESS, ret);

    audioCapturer->Release();
}

/**
* @tc.name  : Test GetParams API via illegal state, CAPTURER_NEW: Call GetParams without SetParams.
* @tc.number: Audio_Capturer_GetParams_003
* @tc.desc  : Test GetParams interface. Returns error code, if the capturer state is CAPTURER_NEW.
*/
HWTEST(AudioCapturerModuleTest, Audio_Capturer_GetParams_003, TestSize.Level1)
{
    int32_t ret = -1;
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioCapturer);

    AudioCapturerParams capturerParams;
    capturerParams.audioSampleFormat = SAMPLE_S16LE;
    capturerParams.samplingRate = SAMPLE_RATE_44100;
    capturerParams.audioChannel = MONO;
    capturerParams.audioEncoding = ENCODING_PCM;

    AudioCapturerParams getCapturerParams;
    ret = audioCapturer->GetParams(getCapturerParams);
    EXPECT_EQ(ERR_OPERATION_FAILED, ret);
}

/**
* @tc.name  : Test GetParams API via illegal state, CAPTURER_RELEASED: Call GetParams after Release.
* @tc.number: Audio_Capturer_GetParams_004
* @tc.desc  : Test GetParams interface. Returns error code, if the capturer state is CAPTURER_RELEASED.
*/
HWTEST(AudioCapturerModuleTest, Audio_Capturer_GetParams_004, TestSize.Level1)
{
    int32_t ret = -1;
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioCapturer);

    ret = AudioCapturerModuleTest::InitializeCapturer(audioCapturer);
    EXPECT_EQ(SUCCESS, ret);

    bool isReleased = audioCapturer->Release();
    EXPECT_EQ(true, isReleased);

    AudioCapturerParams getCapturerParams;
    ret = audioCapturer->GetParams(getCapturerParams);
    EXPECT_EQ(ERR_OPERATION_FAILED, ret);
}

/**
* @tc.name  : Test GetParams API via legal state, CAPTURER_STOPPED: GetParams after Stop.
* @tc.number: Audio_Capturer_GetParams_005
* @tc.desc  : Test GetParams interface. Returns 0 {SUCCESS}, if the getting is successful.
*/
HWTEST(AudioCapturerModuleTest, Audio_Capturer_GetParams_005, TestSize.Level1)
{
    int32_t ret = -1;
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioCapturer);

    ret = AudioCapturerModuleTest::InitializeCapturer(audioCapturer);
    EXPECT_EQ(SUCCESS, ret);

    bool isStarted = audioCapturer->Start();
    EXPECT_EQ(true, isStarted);

    bool isStopped = audioCapturer->Stop();
    EXPECT_EQ(true, isStopped);

    AudioCapturerParams getCapturerParams;
    ret = audioCapturer->GetParams(getCapturerParams);
    EXPECT_EQ(SUCCESS, ret);

    audioCapturer->Release();
}

/**
* @tc.name  : Test GetBufferSize API via legal input.
* @tc.number: Audio_Capturer_GetBufferSize_001
* @tc.desc  : Test GetBufferSize interface. Returns 0 {SUCCESS}, if the getting is successful.
*/
HWTEST(AudioCapturerModuleTest, Audio_Capturer_GetBufferSize_001, TestSize.Level1)
{
    int32_t ret = -1;
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioCapturer);

    ret = AudioCapturerModuleTest::InitializeCapturer(audioCapturer);
    EXPECT_EQ(SUCCESS, ret);

    size_t bufferLen;
    ret = audioCapturer->GetBufferSize(bufferLen);
    EXPECT_EQ(SUCCESS, ret);

    audioCapturer->Release();
}

/**
* @tc.name  : Test GetBufferSize API via illegal state, CAPTURER_NEW: without initializing the capturer.
* @tc.number: Audio_Capturer_GetBufferSize_002
* @tc.desc  : Test GetBufferSize interface. Returns error code, if the capturer state is CAPTURER_NEW.
*/
HWTEST(AudioCapturerModuleTest, Audio_Capturer_GetBufferSize_002, TestSize.Level1)
{
    int32_t ret = -1;
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioCapturer);

    size_t bufferLen;
    ret = audioCapturer->GetBufferSize(bufferLen);
    EXPECT_EQ(ERR_OPERATION_FAILED, ret);
}

/**
* @tc.name  : Test GetBufferSize API via illegal state, CAPTURER_RELEASED: call Release before GetBufferSize
* @tc.number: Audio_Capturer_GetBufferSize_003
* @tc.desc  : Test GetBufferSize interface. Returns error code, if the capturer state is CAPTURER_RELEASED.
*/
HWTEST(AudioCapturerModuleTest, Audio_Capturer_GetBufferSize_003, TestSize.Level1)
{
    int32_t ret = -1;
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioCapturer);

    ret = AudioCapturerModuleTest::InitializeCapturer(audioCapturer);
    EXPECT_EQ(SUCCESS, ret);

    bool isReleased = audioCapturer->Release();
    EXPECT_EQ(true, isReleased);

    size_t bufferLen;
    ret = audioCapturer->GetBufferSize(bufferLen);
    EXPECT_EQ(ERR_OPERATION_FAILED, ret);
}

/**
* @tc.name  : Test GetBufferSize API via legal state, CAPTURER_STOPPED: call Stop before GetBufferSize
* @tc.number: Audio_Capturer_GetBufferSize_004
* @tc.desc  : Test GetBufferSize interface. Returns 0 {SUCCESS}, if the getting is successful.
*/
HWTEST(AudioCapturerModuleTest, Audio_Capturer_GetBufferSize_004, TestSize.Level1)
{
    int32_t ret = -1;
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioCapturer);

    ret = AudioCapturerModuleTest::InitializeCapturer(audioCapturer);
    EXPECT_EQ(SUCCESS, ret);

    bool isStarted = audioCapturer->Start();
    EXPECT_EQ(true, isStarted);

    bool isStopped = audioCapturer->Stop();
    EXPECT_EQ(true, isStopped);

    size_t bufferLen;
    ret = audioCapturer->GetBufferSize(bufferLen);
    EXPECT_EQ(SUCCESS, ret);

    audioCapturer->Release();
}

/**
* @tc.name  : Test GetBufferSize API via legal state, CAPTURER_RUNNING: call Start before GetBufferSize
* @tc.number: Audio_Capturer_GetBufferSize_005
* @tc.desc  : test GetBufferSize interface. Returns 0 {SUCCESS}, if the getting is successful.
*/
HWTEST(AudioCapturerModuleTest, Audio_Capturer_GetBufferSize_005, TestSize.Level1)
{
    int32_t ret = -1;
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioCapturer);

    ret = AudioCapturerModuleTest::InitializeCapturer(audioCapturer);
    EXPECT_EQ(SUCCESS, ret);

    bool isStarted = audioCapturer->Start();
    EXPECT_EQ(true, isStarted);

    size_t bufferLen;
    ret = audioCapturer->GetBufferSize(bufferLen);
    EXPECT_EQ(SUCCESS, ret);

    audioCapturer->Release();
}

/**
* @tc.name  : Test GetFrameCount API via legal input.
* @tc.number: Audio_Capturer_GetFrameCount_001
* @tc.desc  : test GetFrameCount interface, Returns 0 {SUCCESS}, if the getting is successful.
*/
HWTEST(AudioCapturerModuleTest, Audio_Capturer_GetFrameCount_001, TestSize.Level1)
{
    int32_t ret = -1;
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioCapturer);

    ret = AudioCapturerModuleTest::InitializeCapturer(audioCapturer);
    EXPECT_EQ(SUCCESS, ret);

    uint32_t frameCount;
    ret = audioCapturer->GetFrameCount(frameCount);
    EXPECT_EQ(SUCCESS, ret);

    audioCapturer->Release();
}

/**
* @tc.name  : Test GetFrameCount API via illegal state, CAPTURER_NEW: without initialiing the capturer.
* @tc.number: Audio_Capturer_GetFrameCount_002
* @tc.desc  : Test GetFrameCount interface. Returns error code, if the capturer state is CAPTURER_NEW.
*/
HWTEST(AudioCapturerModuleTest, Audio_Capturer_GetFrameCount_002, TestSize.Level1)
{
    int32_t ret = -1;
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioCapturer);

    uint32_t frameCount;
    ret = audioCapturer->GetFrameCount(frameCount);
    EXPECT_EQ(ERR_OPERATION_FAILED, ret);
}

/**
* @tc.name  : Test GetFrameCount API via legal state, CAPTURER_RUNNING: call Start before GetFrameCount.
* @tc.number: Audio_Capturer_GetFrameCount_003
* @tc.desc  : Test GetFrameCount interface. Returns 0 {SUCCESS}, if the getting is successful.
*/
HWTEST(AudioCapturerModuleTest, Audio_Capturer_GetFrameCount_003, TestSize.Level1)
{
    int32_t ret = -1;
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioCapturer);

    ret = AudioCapturerModuleTest::InitializeCapturer(audioCapturer);
    EXPECT_EQ(SUCCESS, ret);

    bool isStarted = audioCapturer->Start();
    EXPECT_EQ(true, isStarted);

    uint32_t frameCount;
    ret = audioCapturer->GetFrameCount(frameCount);
    EXPECT_EQ(SUCCESS, ret);

    audioCapturer->Release();
}

/**
* @tc.name  : Test GetFrameCount API via legal state, CAPTURER_STOPPED: call Stop before GetFrameCount
* @tc.number: Audio_Capturer_GetFrameCount_004
* @tc.desc  : Test GetFrameCount interface. Returns 0 {SUCCESS}, if the getting is successful.
*/
HWTEST(AudioCapturerModuleTest, Audio_Capturer_GetFrameCount_004, TestSize.Level1)
{
    int32_t ret = -1;
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioCapturer);

    ret = AudioCapturerModuleTest::InitializeCapturer(audioCapturer);
    EXPECT_EQ(SUCCESS, ret);

    bool isStarted = audioCapturer->Start();
    EXPECT_EQ(true, isStarted);

    bool isStopped = audioCapturer->Stop();
    EXPECT_EQ(true, isStopped);

    uint32_t frameCount;
    ret = audioCapturer->GetFrameCount(frameCount);
    EXPECT_EQ(SUCCESS, ret);

    audioCapturer->Release();
}

/**
* @tc.name  : Test GetFrameCount API via illegal state, CAPTURER_RELEASED: call Release before GetFrameCount
* @tc.number: Audio_Capturer_GetFrameCount_005
* @tc.desc  : Test GetFrameCount interface.  Returns error code, if the state is CAPTURER_RELEASED.
*/
HWTEST(AudioCapturerModuleTest, Audio_Capturer_GetFrameCount_005, TestSize.Level1)
{
    int32_t ret = -1;
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioCapturer);

    ret = AudioCapturerModuleTest::InitializeCapturer(audioCapturer);
    EXPECT_EQ(SUCCESS, ret);

    bool isReleased = audioCapturer->Release();
    EXPECT_EQ(true, isReleased);

    uint32_t frameCount;
    ret = audioCapturer->GetFrameCount(frameCount);
    EXPECT_EQ(ERR_OPERATION_FAILED, ret);
}

/**
* @tc.name  : Test Start API via legal state, CAPTURER_PREPARED.
* @tc.number: Audio_Capturer_Start_001
* @tc.desc  : Test Start interface. Returns true if start is successful.
*/
HWTEST(AudioCapturerModuleTest, Audio_Capturer_Start_001, TestSize.Level1)
{
    int32_t ret = -1;
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioCapturer);

    ret = AudioCapturerModuleTest::InitializeCapturer(audioCapturer);
    EXPECT_EQ(SUCCESS, ret);

    bool isStarted = audioCapturer->Start();
    EXPECT_EQ(true, isStarted);

    audioCapturer->Release();
}

/**
* @tc.name  : Test Start API via illegal state, CAPTURER_NEW: without initializing the capturer.
* @tc.number: Audio_Capturer_Start_002
* @tc.desc  : Test Start interface. Returns false, if the capturer state is CAPTURER_NEW.
*/
HWTEST(AudioCapturerModuleTest, Audio_Capturer_Start_002, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioCapturer);

    bool isStarted = audioCapturer->Start();
    EXPECT_EQ(false, isStarted);
}

/**
* @tc.name  : Test Start API via illegal state, CAPTURER_RELEASED: call Start after Release
* @tc.number: Audio_Capturer_Start_003
* @tc.desc  : Test Start interface. Returns false, if the capturer state is CAPTURER_RELEASED.
*/
HWTEST(AudioCapturerModuleTest, Audio_Capturer_Start_003, TestSize.Level1)
{
    int32_t ret = -1;
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioCapturer);

    ret = AudioCapturerModuleTest::InitializeCapturer(audioCapturer);
    EXPECT_EQ(SUCCESS, ret);

    bool isReleased = audioCapturer->Release();
    EXPECT_EQ(true, isReleased);

    bool isStarted = audioCapturer->Start();
    EXPECT_EQ(false, isStarted);

    audioCapturer->Release();
}

/**
* @tc.name  : Test Start API via legal state, CAPTURER_STOPPED: Start Stop and then Start again
* @tc.number: Audio_Capturer_Start_004
* @tc.desc  : Test Start interface. Returns true, if the start is successful.
*/
HWTEST(AudioCapturerModuleTest, Audio_Capturer_Start_004, TestSize.Level1)
{
    int32_t ret = -1;
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioCapturer);

    ret = AudioCapturerModuleTest::InitializeCapturer(audioCapturer);
    EXPECT_EQ(SUCCESS, ret);

    bool isStarted = audioCapturer->Start();
    EXPECT_EQ(true, isStarted);

    bool isStopped = audioCapturer->Stop();
    EXPECT_EQ(true, isStopped);

    isStarted = audioCapturer->Start();
    EXPECT_EQ(true, isStarted);

    audioCapturer->Release();
}

/**
* @tc.name  : Test Start API via illegal state, CAPTURER_RUNNING : call Start repeatedly
* @tc.number: Audio_Capturer_Start_005
* @tc.desc  : Test Start interface. Returns false, if the capturer state is CAPTURER_RUNNING.
*/
HWTEST(AudioCapturerModuleTest, Audio_Capturer_Start_005, TestSize.Level1)
{
    int32_t ret = -1;
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioCapturer);

    ret = AudioCapturerModuleTest::InitializeCapturer(audioCapturer);
    EXPECT_EQ(SUCCESS, ret);

    bool isStarted = audioCapturer->Start();
    EXPECT_EQ(true, isStarted);

    isStarted = audioCapturer->Start();
    EXPECT_EQ(false, isStarted);

    audioCapturer->Release();
}

/**
* @tc.name  : Test Read API via isBlockingRead = true.
* @tc.number: Audio_Capturer_Read_001
* @tc.desc  : Test Read interface. Returns number of bytes read, if the read is successful.
*/
HWTEST(AudioCapturerModuleTest, Audio_Capturer_Read_001, TestSize.Level1)
{
    int32_t ret = -1;
    bool isBlockingRead = true;
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioCapturer);

    ret = AudioCapturerModuleTest::InitializeCapturer(audioCapturer);
    EXPECT_EQ(SUCCESS, ret);

    bool isStarted = audioCapturer->Start();
    EXPECT_EQ(true, isStarted);

    size_t bufferLen;
    ret = audioCapturer->GetBufferSize(bufferLen);
    EXPECT_EQ(SUCCESS, ret);

    uint8_t *buffer = (uint8_t *) malloc(bufferLen);
    ASSERT_NE(nullptr, buffer);
    FILE *capFile = fopen(AUDIO_CAPTURE_FILE1.c_str(), "wb");
    ASSERT_NE(nullptr, capFile);

    size_t size = 1;
    int32_t bytesRead = 0;
    int32_t numBuffersToCapture = READ_BUFFERS_COUNT;

    while (numBuffersToCapture) {
        bytesRead = audioCapturer->Read(*buffer, bufferLen, isBlockingRead);
        if (bytesRead < 0) {
            break;
        } else if (bytesRead > 0) {
            fwrite(buffer, size, bytesRead, capFile);
            numBuffersToCapture--;
        }
    }

    audioCapturer->Flush();
    audioCapturer->Stop();
    audioCapturer->Release();

    free(buffer);
    fclose(capFile);
}

/**
* @tc.name  : Test Read API via isBlockingRead = false.
* @tc.number: Audio_Capturer_Read_002
* @tc.desc  : Test Read interface. Returns number of bytes read, if the read is successful.
*/
HWTEST(AudioCapturerModuleTest, Audio_Capturer_Read_002, TestSize.Level1)
{
    int32_t ret = -1;
    bool isBlockingRead = false;
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioCapturer);

    ret = AudioCapturerModuleTest::InitializeCapturer(audioCapturer);
    EXPECT_EQ(SUCCESS, ret);

    bool isStarted = audioCapturer->Start();
    EXPECT_EQ(true, isStarted);

    size_t bufferLen;
    ret = audioCapturer->GetBufferSize(bufferLen);
    EXPECT_EQ(SUCCESS, ret);

    uint8_t *buffer = (uint8_t *) malloc(bufferLen);
    ASSERT_NE(nullptr, buffer);
    FILE *capFile = fopen(AUDIO_CAPTURE_FILE2.c_str(), "wb");
    ASSERT_NE(nullptr, capFile);

    size_t size = 1;
    int32_t bytesRead = 0;
    int32_t numBuffersToCapture = READ_BUFFERS_COUNT;

    while (numBuffersToCapture) {
        bytesRead = audioCapturer->Read(*buffer, bufferLen, isBlockingRead);
        if (bytesRead < 0) {
            break;
        } else if (bytesRead > 0) {
            fwrite(buffer, size, bytesRead, capFile);
            numBuffersToCapture--;
        }
    }

    audioCapturer->Flush();
    audioCapturer->Stop();
    audioCapturer->Release();

    free(buffer);
    fclose(capFile);
}


/**
* @tc.name  : Test Read API via illegl state, CAPTURER_NEW : without Initializing the capturer.
* @tc.number: Audio_Capturer_Read_003
* @tc.desc  : Test Read interface. Returns error code, if the capturer state is CAPTURER_NEW.
*           : bufferLen is invalid here, firstly bufferLen is validated in Read. So it returns ERR_INVALID_PARAM.
*/
HWTEST(AudioCapturerModuleTest, Audio_Capturer_Read_003, TestSize.Level1)
{
    int32_t ret = -1;
    bool isBlockingRead = true;
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioCapturer);

    bool isStarted = audioCapturer->Start();
    EXPECT_EQ(false, isStarted);

    size_t bufferLen;
    ret = audioCapturer->GetBufferSize(bufferLen);
    EXPECT_EQ(ERR_OPERATION_FAILED, ret);

    uint8_t *buffer = (uint8_t *) malloc(bufferLen);
    ASSERT_NE(nullptr, buffer);

    int32_t bytesRead = audioCapturer->Read(*buffer, bufferLen, isBlockingRead);
    EXPECT_EQ(ERR_INVALID_PARAM, bytesRead);

    audioCapturer->Flush();
    audioCapturer->Stop();
    audioCapturer->Release();

    free(buffer);
}

/**
* @tc.name  : Test Read API via illegl state, CAPTURER_PREPARED : Read without Start.
* @tc.number: Audio_Capturer_Read_004
* @tc.desc  : Test Read interface. Returns error code, if the capturer state is not CAPTURER_RUNNING.
*/
HWTEST(AudioCapturerModuleTest, Audio_Capturer_Read_004, TestSize.Level1)
{
    int32_t ret = -1;
    bool isBlockingRead = true;
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioCapturer);

    ret = AudioCapturerModuleTest::InitializeCapturer(audioCapturer);
    EXPECT_EQ(SUCCESS, ret);

    size_t bufferLen;
    ret = audioCapturer->GetBufferSize(bufferLen);
    EXPECT_EQ(SUCCESS, ret);

    uint8_t *buffer = (uint8_t *) malloc(bufferLen);
    ASSERT_NE(nullptr, buffer);

    int32_t bytesRead = audioCapturer->Read(*buffer, bufferLen, isBlockingRead);
    EXPECT_EQ(ERR_ILLEGAL_STATE, bytesRead);

    audioCapturer->Flush();
    audioCapturer->Stop();
    audioCapturer->Release();

    free(buffer);
}

/**
* @tc.name  : Test Read API via illegal input, bufferLength = 0.
* @tc.number: Audio_Capturer_Read_005
* @tc.desc  : Test Read interface. Returns error code, if the bufferLength <= 0.
*/
HWTEST(AudioCapturerModuleTest, Audio_Capturer_Read_005, TestSize.Level1)
{
    int32_t ret = -1;
    bool isBlockingRead = true;
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioCapturer);

    ret = AudioCapturerModuleTest::InitializeCapturer(audioCapturer);
    EXPECT_EQ(SUCCESS, ret);

    bool isStarted = audioCapturer->Start();
    EXPECT_EQ(true, isStarted);

    size_t bufferLen = 0;

    uint8_t *buffer = (uint8_t *) malloc(bufferLen);
    ASSERT_NE(nullptr, buffer);

    int32_t bytesRead = audioCapturer->Read(*buffer, bufferLen, isBlockingRead);
    EXPECT_EQ(ERR_INVALID_PARAM, bytesRead);

    audioCapturer->Flush();
    audioCapturer->Stop();
    audioCapturer->Release();

    free(buffer);
}

/**
* @tc.name  : Test Read API via illegal state, CAPTURER_STOPPED: Read after Stop.
* @tc.number: Audio_Capturer_Read_006
* @tc.desc  : Test Read interface. Returns error code, if the capturer state is not CAPTURER_RUNNING.
*/
HWTEST(AudioCapturerModuleTest, Audio_Capturer_Read_006, TestSize.Level1)
{
    int32_t ret = -1;
    bool isBlockingRead = true;
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioCapturer);

    ret = AudioCapturerModuleTest::InitializeCapturer(audioCapturer);
    EXPECT_EQ(SUCCESS, ret);

    bool isStarted = audioCapturer->Start();
    EXPECT_EQ(true, isStarted);

    size_t bufferLen;
    ret = audioCapturer->GetBufferSize(bufferLen);
    EXPECT_EQ(SUCCESS, ret);

    uint8_t *buffer = (uint8_t *) malloc(bufferLen);
    ASSERT_NE(nullptr, buffer);

    bool isStopped = audioCapturer->Stop();
    EXPECT_EQ(true, isStopped);

    int32_t bytesRead = audioCapturer->Read(*buffer, bufferLen, isBlockingRead);
    EXPECT_EQ(ERR_ILLEGAL_STATE, bytesRead);

    audioCapturer->Release();

    free(buffer);
}

/**
* @tc.name  : Test Read API via illegal state, CAPTURER_RELEASED: Read after Release.
* @tc.number: Audio_Capturer_Read_007
* @tc.desc  : Test Read interface. Returns error code, if the capturer state is not CAPTURER_RUNNING.
*/
HWTEST(AudioCapturerModuleTest, Audio_Capturer_Read_007, TestSize.Level1)
{
    int32_t ret = -1;
    bool isBlockingRead = true;
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioCapturer);

    ret = AudioCapturerModuleTest::InitializeCapturer(audioCapturer);
    EXPECT_EQ(SUCCESS, ret);

    bool isStarted = audioCapturer->Start();
    EXPECT_EQ(true, isStarted);

    size_t bufferLen;
    ret = audioCapturer->GetBufferSize(bufferLen);
    EXPECT_EQ(SUCCESS, ret);

    uint8_t *buffer = (uint8_t *) malloc(bufferLen);
    ASSERT_NE(nullptr, buffer);

    bool isReleased = audioCapturer->Release();
    EXPECT_EQ(true, isReleased);

    int32_t bytesRead = audioCapturer->Read(*buffer, bufferLen, isBlockingRead);
    EXPECT_EQ(ERR_ILLEGAL_STATE, bytesRead);

    free(buffer);
}

/**
* @tc.name  : Test GetAudioTime API via legal input.
* @tc.number: Audio_Capturer_GetAudioTime_001
* @tc.desc  : Test GetAudioTime interface. Returns true, if the getting is successful.
*/
HWTEST(AudioCapturerModuleTest, Audio_Capturer_GetAudioTime_001, TestSize.Level1)
{
    int32_t ret = -1;
    bool isBlockingRead = true;
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioCapturer);

    ret = AudioCapturerModuleTest::InitializeCapturer(audioCapturer);
    EXPECT_EQ(SUCCESS, ret);

    bool isStarted = audioCapturer->Start();
    EXPECT_EQ(true, isStarted);

    size_t bufferLen;
    ret = audioCapturer->GetBufferSize(bufferLen);
    EXPECT_EQ(SUCCESS, ret);

    uint8_t *buffer = (uint8_t *) malloc(bufferLen);
    ASSERT_NE(nullptr, buffer);
    int32_t bytesRead = audioCapturer->Read(*buffer, bufferLen, isBlockingRead);
    EXPECT_GE(bytesRead, VALUE_ZERO);

    Timestamp timeStamp;
    bool getAudioTime = audioCapturer->GetAudioTime(timeStamp, Timestamp::Timestampbase::MONOTONIC);
    EXPECT_EQ(true, getAudioTime);
    EXPECT_GE(timeStamp.time.tv_sec, (const long)VALUE_ZERO);
    EXPECT_GE(timeStamp.time.tv_nsec, (const long)VALUE_ZERO);

    audioCapturer->Flush();
    audioCapturer->Stop();
    audioCapturer->Release();

    free(buffer);
}

/**
* @tc.name  : Test GetAudioTime API via illegal state, CAPTURER_NEW: GetAudioTime without initializing the capturer.
* @tc.number: Audio_Capturer_GetAudioTime_002
* @tc.desc  : Test GetAudioTime interface. Returns false, if the capturer state is CAPTURER_NEW.
*/
HWTEST(AudioCapturerModuleTest, Audio_Capturer_GetAudioTime_002, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioCapturer);

    Timestamp timeStamp;
    bool getAudioTime = audioCapturer->GetAudioTime(timeStamp, Timestamp::Timestampbase::MONOTONIC);
    EXPECT_EQ(false, getAudioTime);
}

/**
* @tc.name  : Test GetAudioTime API via legal state, CAPTURER_RUNNING.
* @tc.number: Audio_Capturer_GetAudioTime_003
* @tc.desc  : test GetAudioTime interface. Returns true, if the getting is successful.
*/
HWTEST(AudioCapturerModuleTest, Audio_Capturer_GetAudioTime_003, TestSize.Level1)
{
    int32_t ret = -1;
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioCapturer);

    ret = AudioCapturerModuleTest::InitializeCapturer(audioCapturer);
    EXPECT_EQ(SUCCESS, ret);

    bool isStarted = audioCapturer->Start();
    EXPECT_EQ(true, isStarted);

    Timestamp timeStamp;
    bool getAudioTime = audioCapturer->GetAudioTime(timeStamp, Timestamp::Timestampbase::MONOTONIC);
    EXPECT_EQ(true, getAudioTime);

    audioCapturer->Release();
}

/**
* @tc.name  : Test GetAudioTime API via legal state, CAPTURER_STOPPED.
* @tc.number: Audio_Capturer_GetAudioTime_004
* @tc.desc  : Test GetAudioTime interface.  Returns true, if the getting is successful.
*/
HWTEST(AudioCapturerModuleTest, Audio_Capturer_GetAudioTime_004, TestSize.Level1)
{
    int32_t ret = -1;
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioCapturer);

    ret = AudioCapturerModuleTest::InitializeCapturer(audioCapturer);
    EXPECT_EQ(SUCCESS, ret);

    bool isStarted = audioCapturer->Start();
    EXPECT_EQ(true, isStarted);

    bool isStopped = audioCapturer->Stop();
    EXPECT_EQ(true, isStopped);

    Timestamp timeStamp;
    bool getAudioTime = audioCapturer->GetAudioTime(timeStamp, Timestamp::Timestampbase::MONOTONIC);
    EXPECT_EQ(true, getAudioTime);

    audioCapturer->Release();
}

/**
* @tc.name  : Test GetAudioTime API via illegal state, CAPTURER_RELEASED: GetAudioTime after Release.
* @tc.number: Audio_Capturer_GetAudioTime_005
* @tc.desc  : Test GetAudioTime interface. Returns false, if the capturer state is CAPTURER_RELEASED
*/
HWTEST(AudioCapturerModuleTest, Audio_Capturer_GetAudioTime_005, TestSize.Level1)
{
    int32_t ret = -1;
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioCapturer);

    ret = AudioCapturerModuleTest::InitializeCapturer(audioCapturer);
    EXPECT_EQ(SUCCESS, ret);

    bool isStarted = audioCapturer->Start();
    EXPECT_EQ(true, isStarted);

    bool isStopped = audioCapturer->Stop();
    EXPECT_EQ(true, isStopped);

    bool isReleased = audioCapturer->Release();
    EXPECT_EQ(true, isReleased);

    Timestamp timeStamp;
    bool getAudioTime = audioCapturer->GetAudioTime(timeStamp, Timestamp::Timestampbase::MONOTONIC);
    EXPECT_EQ(false, getAudioTime);
}

/**
* @tc.name  : Test Flush API.
* @tc.number: Audio_Capturer_Flush_001
* @tc.desc  : Test Flush interface. Returns true, if the flush is successful.
*/
HWTEST(AudioCapturerModuleTest, Audio_Capturer_Flush_001, TestSize.Level1)
{
    int32_t ret = -1;
    bool isBlockingRead = true;
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioCapturer);

    ret = AudioCapturerModuleTest::InitializeCapturer(audioCapturer);
    EXPECT_EQ(SUCCESS, ret);

    bool isStarted = audioCapturer->Start();
    EXPECT_EQ(true, isStarted);

    size_t bufferLen;
    ret = audioCapturer->GetBufferSize(bufferLen);
    EXPECT_EQ(SUCCESS, ret);

    uint8_t *buffer = (uint8_t *) malloc(bufferLen);
    ASSERT_NE(nullptr, buffer);

    int32_t bytesRead = audioCapturer->Read(*buffer, bufferLen, isBlockingRead);
    EXPECT_GE(bytesRead, VALUE_ZERO);

    bool isFlushed = audioCapturer->Flush();
    EXPECT_EQ(true, isFlushed);

    audioCapturer->Stop();
    audioCapturer->Release();

    free(buffer);
}

/**
* @tc.name  : Test Flush API via illegal state, CAPTURER_NEW: Without initializing the capturer.
* @tc.number: Audio_Capturer_Flush_002
* @tc.desc  : Test Flush interface. Returns false, if the capturer state is not CAPTURER_RUNNING.
*/
HWTEST(AudioCapturerModuleTest, Audio_Capturer_Flush_002, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioCapturer);

    bool isFlushed = audioCapturer->Flush();
    EXPECT_EQ(false, isFlushed);

    audioCapturer->Release();
}

/**
* @tc.name  : Test Flush API via illegal state, CAPTURER_PREPARED: Without Start.
* @tc.number: Audio_Capturer_Flush_003
* @tc.desc  : Test Flush interface. Returns false, if the capturer state is not CAPTURER_RUNNING.
*/
HWTEST(AudioCapturerModuleTest, Audio_Capturer_Flush_003, TestSize.Level1)
{
    int32_t ret = -1;
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioCapturer);

    ret = AudioCapturerModuleTest::InitializeCapturer(audioCapturer);
    EXPECT_EQ(SUCCESS, ret);

    bool isFlushed = audioCapturer->Flush();
    EXPECT_EQ(false, isFlushed);

    audioCapturer->Release();
}

/**
* @tc.name  : Test Flush API via illegal state, CAPTURER_STOPPED: call Stop before Flush.
* @tc.number: Audio_Capturer_Flush_004
* @tc.desc  : Test Flush interface. Returns false, if the capturer state is not CAPTURER_RUNNING.
*/
HWTEST(AudioCapturerModuleTest, Audio_Capturer_Flush_004, TestSize.Level1)
{
    int32_t ret = -1;
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioCapturer);

    ret = AudioCapturerModuleTest::InitializeCapturer(audioCapturer);
    EXPECT_EQ(SUCCESS, ret);

    bool isStarted = audioCapturer->Start();
    EXPECT_EQ(true, isStarted);

    bool isStopped = audioCapturer->Stop();
    EXPECT_EQ(true, isStopped);

    bool isFlushed = audioCapturer->Flush();
    EXPECT_EQ(false, isFlushed);

    audioCapturer->Release();
}

/**
* @tc.name  : Test Flush API via illegal state, CAPTURER_RELEASED: call Release before Flush.
* @tc.number: Audio_Capturer_Flush_005
* @tc.desc  : Test Flush interface. Returns false, if the capturer state is not CAPTURER_RUNNING.
*/
HWTEST(AudioCapturerModuleTest, Audio_Capturer_Flush_005, TestSize.Level1)
{
    int32_t ret = -1;
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioCapturer);

    ret = AudioCapturerModuleTest::InitializeCapturer(audioCapturer);
    EXPECT_EQ(SUCCESS, ret);

    bool isStarted = audioCapturer->Start();
    EXPECT_EQ(true, isStarted);

    bool isReleased = audioCapturer->Release();
    EXPECT_EQ(true, isReleased);

    bool isFlushed = audioCapturer->Flush();
    EXPECT_EQ(false, isFlushed);

    audioCapturer->Release();
}

/**
* @tc.name  : Test Stop API.
* @tc.number: Audio_Capturer_Stop_001
* @tc.desc  : Test Stop interface. Returns true, if the stop is successful.
*/
HWTEST(AudioCapturerModuleTest, Audio_Capturer_Stop_001, TestSize.Level1)
{
    int32_t ret = -1;
    bool isBlockingRead = true;
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioCapturer);

    ret = AudioCapturerModuleTest::InitializeCapturer(audioCapturer);
    EXPECT_EQ(SUCCESS, ret);

    bool isStarted = audioCapturer->Start();
    EXPECT_EQ(true, isStarted);

    size_t bufferLen;
    ret = audioCapturer->GetBufferSize(bufferLen);
    EXPECT_EQ(SUCCESS, ret);

    uint8_t *buffer = (uint8_t *) malloc(bufferLen);
    ASSERT_NE(nullptr, buffer);

    int32_t bytesRead = audioCapturer->Read(*buffer, bufferLen, isBlockingRead);
    EXPECT_GE(bytesRead, VALUE_ZERO);

    audioCapturer->Flush();

    bool isStopped = audioCapturer->Stop();
    EXPECT_EQ(true, isStopped);

    audioCapturer->Release();

    free(buffer);
}

/**
* @tc.name  : Test Stop API via illegal state, CAPTURER_NEW: call Stop without Initializing the capturer.
* @tc.number: Audio_Capturer_Stop_002
* @tc.desc  : Test Stop interface. Returns false, if the capturer state is not CAPTURER_RUNNING.
*/
HWTEST(AudioCapturerModuleTest, Audio_Capturer_Stop_002, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioCapturer);

    bool isStopped = audioCapturer->Stop();
    EXPECT_EQ(false, isStopped);
}

/**
* @tc.name  : Test Stop API via illegal state, CAPTURER_PREPARED: call Stop without Start.
* @tc.number: Audio_Capturer_Stop_003
* @tc.desc  : Test Stop interface. Returns false, if the capturer state is not CAPTURER_RUNNING.
*/
HWTEST(AudioCapturerModuleTest, Audio_Capturer_Stop_003, TestSize.Level1)
{
    int32_t ret = -1;
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioCapturer);

    ret = AudioCapturerModuleTest::InitializeCapturer(audioCapturer);
    EXPECT_EQ(SUCCESS, ret);

    bool isStopped = audioCapturer->Stop();
    EXPECT_EQ(false, isStopped);

    audioCapturer->Release();
}

/**
* @tc.name  : Test Stop API via illegal state, CAPTURER_RELEASED: call Stop after Release.
* @tc.number: Audio_Capturer_Stop_004
* @tc.desc  : Test Stop interface. Returns false, if the capturer state is not CAPTURER_RUNNING.
*/
HWTEST(AudioCapturerModuleTest, Audio_Capturer_Stop_004, TestSize.Level1)
{
    int32_t ret = -1;
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioCapturer);

    ret = AudioCapturerModuleTest::InitializeCapturer(audioCapturer);
    EXPECT_EQ(SUCCESS, ret);

    bool isStarted = audioCapturer->Start();
    EXPECT_EQ(true, isStarted);

    bool isReleased = audioCapturer->Release();
    EXPECT_EQ(true, isReleased);

    bool isStopped = audioCapturer->Stop();
    EXPECT_EQ(false, isStopped);
}

/**
* @tc.name  : Test Stop API via legal state. call Start, Stop, Start and Stop again
* @tc.number: Audio_Capturer_Stop_005
* @tc.desc  : Test Stop interface. Returns true , if the stop is successful.
*/
HWTEST(AudioCapturerModuleTest, Audio_Capturer_Stop_005, TestSize.Level1)
{
    int32_t ret = -1;
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioCapturer);

    ret = AudioCapturerModuleTest::InitializeCapturer(audioCapturer);
    EXPECT_EQ(SUCCESS, ret);

    bool isStarted = audioCapturer->Start();
    EXPECT_EQ(true, isStarted);

    bool isStopped = audioCapturer->Stop();
    EXPECT_EQ(true, isStopped);

    isStarted = audioCapturer->Start();
    EXPECT_EQ(true, isStarted);

    isStopped = audioCapturer->Stop();
    EXPECT_EQ(true, isStopped);
}

/**
* @tc.name  : Test Release API.
* @tc.number: Audio_Capturer_Release_001
* @tc.desc  : Test Release interface. Returns true, if the release is successful.
*/
HWTEST(AudioCapturerModuleTest, Audio_Capturer_Release_001, TestSize.Level1)
{
    int32_t ret = -1;
    bool isBlockingRead = true;
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioCapturer);

    ret = AudioCapturerModuleTest::InitializeCapturer(audioCapturer);
    EXPECT_EQ(SUCCESS, ret);

    bool isStarted = audioCapturer->Start();
    EXPECT_EQ(true, isStarted);

    size_t bufferLen;
    ret = audioCapturer->GetBufferSize(bufferLen);
    EXPECT_EQ(SUCCESS, ret);

    uint8_t *buffer = (uint8_t *) malloc(bufferLen);
    ASSERT_NE(nullptr, buffer);

    int32_t bytesRead = audioCapturer->Read(*buffer, bufferLen, isBlockingRead);
    EXPECT_GE(bytesRead, VALUE_ZERO);

    audioCapturer->Flush();
    audioCapturer->Stop();

    bool isReleased = audioCapturer->Release();
    EXPECT_EQ(true, isReleased);

    free(buffer);
}

/**
* @tc.name  : Test Release API via illegal state, CAPTURER_NEW: Call Release without initializing the capturer.
* @tc.number: Audio_Capturer_Release_002
* @tc.desc  : Test Release interface, Returns false, if the state is CAPTURER_NEW.
*/
HWTEST(AudioCapturerModuleTest, Audio_Capturer_Release_002, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioCapturer);

    bool isReleased = audioCapturer->Release();
    EXPECT_EQ(false, isReleased);
}

/**
* @tc.name  : Test Release API via illegal state, CAPTURER_RELEASED: call Release repeatedly.
* @tc.number: Audio_Capturer_Release_003
* @tc.desc  : Test Release interface. Returns false, if the state is already CAPTURER_RELEASED.
*/
HWTEST(AudioCapturerModuleTest, Audio_Capturer_Release_003, TestSize.Level1)
{
    int32_t ret = -1;
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioCapturer);

    ret = AudioCapturerModuleTest::InitializeCapturer(audioCapturer);
    EXPECT_EQ(SUCCESS, ret);

    bool isReleased = audioCapturer->Release();
    EXPECT_EQ(true, isReleased);

    isReleased = audioCapturer->Release();
    EXPECT_EQ(false, isReleased);
}

/**
* @tc.name  : Test Release API via legal state, CAPTURER_RUNNING: call Release after Start
* @tc.number: Audio_Capturer_Release_004
* @tc.desc  : Test Release interface. Returns true, if the release is successful.
*/
HWTEST(AudioCapturerModuleTest, Audio_Capturer_Release_004, TestSize.Level1)
{
    int32_t ret = -1;
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioCapturer);

    ret = AudioCapturerModuleTest::InitializeCapturer(audioCapturer);
    EXPECT_EQ(SUCCESS, ret);

    bool isStarted = audioCapturer->Start();
    EXPECT_EQ(true, isStarted);

    bool isReleased = audioCapturer->Release();
    EXPECT_EQ(true, isReleased);
}

/**
* @tc.name  : Test Release API via legal state, CAPTURER_STOPPED: call release after Start and Stop
* @tc.number: Audio_Capturer_Release_005
* @tc.desc  : Test Release interface. Returns true, if the release is successful.
*/
HWTEST(AudioCapturerModuleTest, Audio_Capturer_Release_005, TestSize.Level1)
{
    int32_t ret = -1;
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioCapturer);

    ret = AudioCapturerModuleTest::InitializeCapturer(audioCapturer);
    EXPECT_EQ(SUCCESS, ret);

    bool isStarted = audioCapturer->Start();
    EXPECT_EQ(true, isStarted);

    bool isStopped = audioCapturer->Stop();
    EXPECT_EQ(true, isStopped);

    bool isReleased = audioCapturer->Release();
    EXPECT_EQ(true, isReleased);
}

/**
* @tc.name  : Test GetStatus API.
* @tc.number: Audio_Capturer_GetStatus_001
* @tc.desc  : Test GetStatus interface. Returns correct state on success.
*/
HWTEST(AudioCapturerModuleTest, Audio_Capturer_GetStatus_001, TestSize.Level1)
{
    int32_t ret = -1;
    CapturerState state = CAPTURER_INVALID;

    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioCapturer);
    state = audioCapturer->GetStatus();
    EXPECT_EQ(CAPTURER_NEW, state);

    AudioCapturerParams capturerParams;
    capturerParams.audioSampleFormat = SAMPLE_S16LE;
    capturerParams.samplingRate = SAMPLE_RATE_44100;
    capturerParams.audioChannel = STEREO;
    capturerParams.audioEncoding = ENCODING_PCM;
    ret = audioCapturer->SetParams(capturerParams);
    EXPECT_EQ(SUCCESS, ret);
    state = audioCapturer->GetStatus();
    EXPECT_EQ(CAPTURER_PREPARED, state);

    bool isStarted = audioCapturer->Start();
    EXPECT_EQ(true, isStarted);
    state = audioCapturer->GetStatus();
    EXPECT_EQ(CAPTURER_RUNNING, state);

    bool isStopped = audioCapturer->Stop();
    EXPECT_EQ(true, isStopped);
    state = audioCapturer->GetStatus();
    EXPECT_EQ(CAPTURER_STOPPED, state);

    bool isReleased = audioCapturer->Release();
    EXPECT_EQ(true, isReleased);
    state = audioCapturer->GetStatus();
    EXPECT_EQ(CAPTURER_RELEASED, state);
}
/**
* @tc.name  : Test GetStatus API, call Start without Initializing the capturer
* @tc.number: Audio_Capturer_GetStatus_002
* @tc.desc  : Test GetStatus interface. Not changes to CAPTURER_RUNNING, if the current state is CAPTURER_NEW.
*/
HWTEST(AudioCapturerModuleTest, Audio_Capturer_GetStatus_002, TestSize.Level1)
{
    CapturerState state = CAPTURER_INVALID;

    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioCapturer);

    bool isStarted = audioCapturer->Start();
    EXPECT_EQ(false, isStarted);
    state = audioCapturer->GetStatus();
    EXPECT_NE(CAPTURER_RUNNING, state);
    EXPECT_EQ(CAPTURER_NEW, state);
}

/**
* @tc.name  : Test GetStatus API, call Stop without Start
* @tc.desc  : Test GetStatus interface. Not changes to CAPTURER_STOPPED, if the current state is CAPTURER_PREPARED.
*/
HWTEST(AudioCapturerModuleTest, Audio_Capturer_GetStatus_003, TestSize.Level1)
{
    int32_t ret = -1;
    CapturerState state = CAPTURER_INVALID;

    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioCapturer);

    ret = AudioCapturerModuleTest::InitializeCapturer(audioCapturer);
    EXPECT_EQ(SUCCESS, ret);

    bool isStopped = audioCapturer->Stop();
    EXPECT_EQ(false, isStopped);
    state = audioCapturer->GetStatus();
    EXPECT_NE(CAPTURER_STOPPED, state);
    EXPECT_EQ(CAPTURER_PREPARED, state);

    audioCapturer->Release();
}

/**
* @tc.name  : Test GetStatus API, call Start, Stop and then Start again
* @tc.number: Audio_Capturer_GetStatus_004
* @tc.desc  : Test GetStatus interface.  Returns correct state on success.
*/
HWTEST(AudioCapturerModuleTest, Audio_Capturer_GetStatus_004, TestSize.Level1)
{
    int32_t ret = -1;
    CapturerState state = CAPTURER_INVALID;

    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioCapturer);

    ret = AudioCapturerModuleTest::InitializeCapturer(audioCapturer);
    EXPECT_EQ(SUCCESS, ret);

    bool isStarted = audioCapturer->Start();
    EXPECT_EQ(true, isStarted);
    state = audioCapturer->GetStatus();
    EXPECT_EQ(CAPTURER_RUNNING, state);

    bool isStopped = audioCapturer->Stop();
    EXPECT_EQ(true, isStopped);
    state = audioCapturer->GetStatus();
    EXPECT_EQ(CAPTURER_STOPPED, state);

    isStarted = audioCapturer->Start();
    EXPECT_EQ(true, isStarted);
    state = audioCapturer->GetStatus();
    EXPECT_EQ(CAPTURER_RUNNING, state);

    audioCapturer->Release();
}

/**
* @tc.name  : Test GetStatus API, call Release without initializing
* @tc.number: Audio_Capturer_GetStatus_005
* @tc.desc  : Test GetStatus interface. Not changes to CAPTURER_RELEASED, if the current state is CAPTURER_NEW.
*/
HWTEST(AudioCapturerModuleTest, Audio_Capturer_GetStatus_005, TestSize.Level1)
{
    CapturerState state = CAPTURER_INVALID;

    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioCapturer);

    bool isReleased = audioCapturer->Release();
    EXPECT_EQ(false, isReleased);
    state = audioCapturer->GetStatus();
    EXPECT_NE(CAPTURER_RELEASED, state);
    EXPECT_EQ(CAPTURER_NEW, state);
}
} // namespace AudioStandard
} // namespace OHOS
