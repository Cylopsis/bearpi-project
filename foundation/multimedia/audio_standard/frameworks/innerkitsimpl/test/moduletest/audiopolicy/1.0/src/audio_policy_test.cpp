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

#include "audio_policy_test.h"
#include "audio_system_manager.h"

using namespace std;
using namespace OHOS::AudioStandard;
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
namespace V1_0 {
void AudioPolicyTest::SetUpTestCase(void)
{
    ASSERT_NE(nullptr, AudioSystemManager::GetInstance());
}

void AudioPolicyTest::TearDownTestCase(void) {}

void AudioPolicyTest::SetUp(void) {}

void AudioPolicyTest::TearDown(void) {}

namespace {
const PolicyParam VOLUME_PARAMS[] = {
    {
        .volume = 8,
        .streamType = STREAM_MUSIC
    },
    {
        .volume = 8,
        .streamType = STREAM_RING
    }
};

const PolicyParam MUTE_PARAMS[] = {
    {
        .streamType = STREAM_MUSIC,
        .mute = true
    },
    {
        .streamType = STREAM_MUSIC,
        .mute = false
    }
};

const PolicyParam STREAM_PARAMS[] = {
    {
        .streamType = STREAM_MUSIC,
        .active = true
    },
    {
        .streamType = STREAM_RING,
        .active = false
    }
};

const PolicyParam ACTIVE_DEVICE_PARAMS[] = {
    {
        .actDeviceType = SPEAKER,
        .active = true
    },
    {
        .actDeviceType = BLUETOOTH_SCO,
        .active = true
    },
    {
        .actDeviceType = BLUETOOTH_SCO,
        .active = false
    },
    {
        .actDeviceType = SPEAKER,
        .active = true
    },
    {
        .actDeviceType = BLUETOOTH_SCO,
        .active = true
    },
    {
        .actDeviceType = SPEAKER,
        .active = false
    },
    {
        .actDeviceType = SPEAKER,
        .active = true
    },
};

const PolicyParam RINGER_MODE_PARAMS[] = {
    {
        .ringerMode = RINGER_MODE_NORMAL
    },
    {
        .ringerMode = RINGER_MODE_SILENT
    },
    {
        .ringerMode = RINGER_MODE_VIBRATE
    },
};

const PolicyParam MIC_MUTE_PARAMS[] = {
    {
        .mute = true
    },
    {
        .mute = false
    }
};

const PolicyParam VOLUME_RANGE_PARAMS[] = {
    {
        .streamType = STREAM_MUSIC
    },
    {
        .streamType = STREAM_RING
    }
};

const PolicyParam AUDIO_PARAMS[] = {
    {
        .key = "sampling_rate",
        .value = "8000"
    },
    {
        .key = "sampling_rate",
        .value = "44100"
    },
    {
        .key = "sampling_rate",
        .value = "96000"
    }
};

const PolicyParam DEVICES_PARAMS[] = {
    {
        .deviceType = AudioDeviceDescriptor::MIC,
        .deviceFlag = AudioDeviceDescriptor::DeviceFlag::INPUT_DEVICES_FLAG,
        .deviceRole = AudioDeviceDescriptor::DeviceRole::INPUT_DEVICE
    },
    {
        .deviceType = AudioDeviceDescriptor::SPEAKER,
        .deviceFlag = AudioDeviceDescriptor::DeviceFlag::OUTPUT_DEVICES_FLAG,
        .deviceRole = AudioDeviceDescriptor::DeviceRole::OUTPUT_DEVICE
    }
};
} // namespace

/*
 * Set Volume
 *
 */
class AudioPolicySetVolumeTest : public AudioPolicyTest {};

HWTEST_P(AudioPolicySetVolumeTest, SetVolume, TestSize.Level1)
{
    PolicyParam params = GetParam();

    AudioSystemManager::AudioVolumeType volumeType
        = static_cast<AudioSystemManager::AudioVolumeType>(params.streamType);
    float volume = params.volume;
    EXPECT_EQ(MEDIA_OK, AudioSystemManager::GetInstance()->SetVolume(volumeType, volume));
}

INSTANTIATE_TEST_CASE_P(
    SetVolume,
    AudioPolicySetVolumeTest,
    ValuesIn(VOLUME_PARAMS));

/*
 * Get Volume
 *
 */
class AudioPolicyGetVolumeTest : public AudioPolicyTest {};

HWTEST_P(AudioPolicyGetVolumeTest, GetVolume, TestSize.Level1)
{
    PolicyParam params = GetParam();
    AudioSystemManager::AudioVolumeType volumeType
        = static_cast<AudioSystemManager::AudioVolumeType>(params.streamType);
    float volume = params.volume;

    EXPECT_EQ(MEDIA_OK, AudioSystemManager::GetInstance()->SetVolume(volumeType, volume));
    EXPECT_EQ(volume, AudioSystemManager::GetInstance()->GetVolume(volumeType));
}

INSTANTIATE_TEST_CASE_P(
    GetVolume,
    AudioPolicyGetVolumeTest,
    ValuesIn(VOLUME_PARAMS));

/*
 * Set Mute
 *
 */
class AudioPolicySetMuteTest : public AudioPolicyTest {};

HWTEST_P(AudioPolicySetMuteTest, SetMute, TestSize.Level1)
{
    PolicyParam params = GetParam();
    AudioSystemManager::AudioVolumeType volumeType
        = static_cast<AudioSystemManager::AudioVolumeType>(params.streamType);
    bool mute = params.mute;

    EXPECT_EQ(MEDIA_OK, AudioSystemManager::GetInstance()->SetMute(volumeType, mute));
}

INSTANTIATE_TEST_CASE_P(
    SetMute,
    AudioPolicySetMuteTest,
    ValuesIn(MUTE_PARAMS));

/*
 * Is Mute
 *
 */
class AudioPolicyGetMuteTest : public AudioPolicyTest {};

HWTEST_P(AudioPolicyGetMuteTest, IsStreamMute, TestSize.Level1)
{
    PolicyParam params = GetParam();
    AudioSystemManager::AudioVolumeType volumeType
        = static_cast<AudioSystemManager::AudioVolumeType>(params.streamType);
    bool mute = params.mute;

    EXPECT_EQ(MEDIA_OK, AudioSystemManager::GetInstance()->SetMute(volumeType, mute));
    EXPECT_EQ(mute, AudioSystemManager::GetInstance()->IsStreamMute(volumeType));
}

INSTANTIATE_TEST_CASE_P(
    IsStreamMute,
    AudioPolicyGetMuteTest,
    ValuesIn(MUTE_PARAMS));

/*
 * Is Stream Active
 *
 */
class AudioPolicyIsStreamActiveTest : public AudioPolicyTest {};

HWTEST_P(AudioPolicyIsStreamActiveTest, IsStreamActive, TestSize.Level1)
{
    PolicyParam params = GetParam();
    AudioSystemManager::AudioVolumeType volumeType
        = static_cast<AudioSystemManager::AudioVolumeType>(params.streamType);

    // review this code
    EXPECT_EQ(params.active, AudioSystemManager::GetInstance()->IsStreamActive(volumeType));
}

INSTANTIATE_TEST_CASE_P(
    IsStreamActive,
    AudioPolicyIsStreamActiveTest,
    ValuesIn(STREAM_PARAMS));

/*
 * Set Device Active
 *
 */
class AudioPolicySetDeviceActiveTest : public AudioPolicyTest {};

HWTEST_P(AudioPolicySetDeviceActiveTest, SetDeviceActive, TestSize.Level1)
{
    PolicyParam params = GetParam();
    ActiveDeviceType deviceType = params.actDeviceType;
    bool active = params.active;

    EXPECT_EQ(MEDIA_OK, AudioSystemManager::GetInstance()->SetDeviceActive(deviceType, active));
}

INSTANTIATE_TEST_CASE_P(
    SetDeviceActive,
    AudioPolicySetDeviceActiveTest,
    ValuesIn(ACTIVE_DEVICE_PARAMS));

/*
 * Is Device Active
 *
 */
class AudioPolicyIsDeviceActiveTest : public AudioPolicyTest {};

HWTEST_P(AudioPolicyIsDeviceActiveTest, IsDeviceActive, TestSize.Level1)
{
    PolicyParam params = GetParam();
    ActiveDeviceType deviceType = params.actDeviceType;
    bool active = params.active;

    EXPECT_EQ(MEDIA_OK, AudioSystemManager::GetInstance()->SetDeviceActive(deviceType, active));
    EXPECT_EQ(active, AudioSystemManager::GetInstance()->IsDeviceActive(deviceType));
}

INSTANTIATE_TEST_CASE_P(
    IsDeviceActive,
    AudioPolicyIsDeviceActiveTest,
    ValuesIn(ACTIVE_DEVICE_PARAMS));

/*
 * Set Ringer Mode
 *
 */
class AudioPolicySetRingerModeTest : public AudioPolicyTest {};

HWTEST_P(AudioPolicySetRingerModeTest, SetRingerMode, TestSize.Level1)
{
    PolicyParam params = GetParam();
    AudioRingerMode ringerMode = params.ringerMode;

    EXPECT_EQ(MEDIA_OK, AudioSystemManager::GetInstance()->SetRingerMode(ringerMode));
}


INSTANTIATE_TEST_CASE_P(
    SetRingerMode,
    AudioPolicySetRingerModeTest,
    ValuesIn(RINGER_MODE_PARAMS));

/*
 * Get Ringer Mode
 *
 */
class AudioPolicyGetRingerModeTest : public AudioPolicyTest {};

HWTEST_P(AudioPolicyGetRingerModeTest, GetRingerMode, TestSize.Level1)
{
    PolicyParam params = GetParam();
    AudioRingerMode ringerMode = params.ringerMode;

    EXPECT_EQ(MEDIA_OK, AudioSystemManager::GetInstance()->SetRingerMode(ringerMode));
    EXPECT_EQ(ringerMode, AudioSystemManager::GetInstance()->GetRingerMode());
}

INSTANTIATE_TEST_CASE_P(
    GetRingerMode,
    AudioPolicyGetRingerModeTest,
    ValuesIn(RINGER_MODE_PARAMS));

/*
 * Set microphone mute
 *
 */
class AudioPolicySetMicrophoneMuteTest : public AudioPolicyTest {};

HWTEST_P(AudioPolicySetMicrophoneMuteTest, SetMicrophoneMute, TestSize.Level1)
{
    PolicyParam params = GetParam();
    bool mute = params.mute;

    EXPECT_EQ(MEDIA_OK, AudioSystemManager::GetInstance()->SetMicrophoneMute(mute));
}

INSTANTIATE_TEST_CASE_P(
    SetMicrophoneMute,
    AudioPolicySetMicrophoneMuteTest,
    ValuesIn(MIC_MUTE_PARAMS));

/*
 * Is Microphone Mute
 *
 */
class AudioPolicyGetMicrophoneMuteTest : public AudioPolicyTest {};

HWTEST_P(AudioPolicyGetMicrophoneMuteTest, IsMicrophoneMute, TestSize.Level1)
{
    PolicyParam params = GetParam();
    bool mute = params.mute;

    EXPECT_EQ(MEDIA_OK, AudioSystemManager::GetInstance()->SetMicrophoneMute(mute));
    EXPECT_EQ(mute, AudioSystemManager::GetInstance()->IsMicrophoneMute());
}

INSTANTIATE_TEST_CASE_P(
    IsMicrophoneMute,
    AudioPolicyGetMicrophoneMuteTest,
    ValuesIn(MIC_MUTE_PARAMS));

/*
 * Check volume range
 *
 */
class AudioPolicyVolumeRangeTest : public AudioPolicyTest {};

HWTEST_P(AudioPolicyVolumeRangeTest, GetMaxVolume, TestSize.Level1)
{
    PolicyParam params = GetParam();
    AudioSystemManager::AudioVolumeType volumeType
        = static_cast<AudioSystemManager::AudioVolumeType>(params.streamType);
    EXPECT_EQ(15, AudioSystemManager::GetInstance()->GetMaxVolume(volumeType));
}

HWTEST_P(AudioPolicyVolumeRangeTest, GetMinVolume, TestSize.Level1)
{
    PolicyParam params = GetParam();
    AudioSystemManager::AudioVolumeType volumeType
        = static_cast<AudioSystemManager::AudioVolumeType>(params.streamType);
    EXPECT_EQ(0, AudioSystemManager::GetInstance()->GetMinVolume(volumeType));
}

INSTANTIATE_TEST_CASE_P(
    GetMaxVolume,
    AudioPolicyVolumeRangeTest,
    ValuesIn(VOLUME_RANGE_PARAMS));

INSTANTIATE_TEST_CASE_P(
    GetMinVolume,
    AudioPolicyVolumeRangeTest,
    ValuesIn(VOLUME_RANGE_PARAMS));

/*
 * Check volume range
 *
 */
class AudioPolicyAudioParameterTest : public AudioPolicyTest {};

HWTEST_P(AudioPolicyAudioParameterTest, SetAudioParameter, TestSize.Level1)
{
    PolicyParam params = GetParam();
    AudioSystemManager::GetInstance()->SetAudioParameter(params.key, params.value);
    EXPECT_EQ(params.value, AudioSystemManager::GetInstance()->GetAudioParameter(params.key));
}

INSTANTIATE_TEST_CASE_P(
    SetAudioParameter,
    AudioPolicyAudioParameterTest,
    ValuesIn(AUDIO_PARAMS));

HWTEST_P(AudioPolicyAudioParameterTest, GetAudioParameter, TestSize.Level1)
{
    PolicyParam params = GetParam();
    AudioSystemManager::GetInstance()->SetAudioParameter(params.key, params.value);
    EXPECT_EQ(params.value, AudioSystemManager::GetInstance()->GetAudioParameter(params.key));
}

INSTANTIATE_TEST_CASE_P(
    GetAudioParameter,
    AudioPolicyAudioParameterTest,
    ValuesIn(AUDIO_PARAMS));

/*
 * Check volume range
 *
 */
class AudioPolicyGetDevicesTest : public AudioPolicyTest {};

HWTEST_P(AudioPolicyGetDevicesTest, GetDevices, TestSize.Level1)
{
    PolicyParam params = GetParam();
    AudioDeviceDescriptor::DeviceFlag deviceFlag = params.deviceFlag;
    AudioDeviceDescriptor::DeviceType deviceType = params.deviceType;
    AudioDeviceDescriptor::DeviceRole deviceRole = params.deviceRole;
    vector<sptr<AudioDeviceDescriptor>> audioDeviceDescriptors
        = AudioSystemManager::GetInstance()->GetDevices(deviceFlag);
    sptr<AudioDeviceDescriptor> audioDeviceDescriptor = audioDeviceDescriptors[0];
    EXPECT_EQ(deviceType, audioDeviceDescriptor->deviceType_);
    EXPECT_EQ(deviceRole, audioDeviceDescriptor->deviceRole_);
}

INSTANTIATE_TEST_CASE_P(
    GetDevices,
    AudioPolicyGetDevicesTest,
    ValuesIn(DEVICES_PARAMS));
}
}
}
