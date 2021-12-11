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

#ifndef AUDIO_MNGR_NAPI_H_
#define AUDIO_MNGR_NAPI_H_

#include <iostream>
#include <vector>
#include "audio_system_manager.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"

namespace OHOS {
namespace AudioStandard {
static const std::string AUDIO_MNGR_NAPI_CLASS_NAME = "AudioManager";

class AudioManagerNapi {
public:
    AudioManagerNapi();
    ~AudioManagerNapi();

    enum AudioVolumeType {
        RINGTONE = 2,
        MEDIA = 3
    };

    enum AudioRingMode {
        RINGER_MODE_SILENT = 0,
        RINGER_MODE_VIBRATE,
        RINGER_MODE_NORMAL
    };

    static napi_value Init(napi_env env, napi_value exports);

private:
    static void Destructor(napi_env env, void *nativeObject, void *finalize_hint);
    static napi_value Construct(napi_env env, napi_callback_info info);
    static napi_value CreateAudioManagerWrapper(napi_env env);
    static napi_value GetAudioManager(napi_env env, napi_callback_info info);
    static napi_value SetVolume(napi_env env, napi_callback_info info);
    static napi_value GetVolume(napi_env env, napi_callback_info info);
    static napi_value GetMaxVolume(napi_env env, napi_callback_info info);
    static napi_value GetMinVolume(napi_env env, napi_callback_info info);
    static napi_value GetDevices(napi_env env, napi_callback_info info);
    static napi_value SetStreamMute(napi_env env, napi_callback_info info);
    static napi_value IsStreamMute(napi_env env, napi_callback_info info);
    static napi_value IsStreamActive(napi_env env, napi_callback_info info);
    static napi_value SetRingerMode(napi_env env, napi_callback_info info);
    static napi_value GetRingerMode(napi_env env, napi_callback_info info);
    static napi_value SetDeviceActive(napi_env env, napi_callback_info info);
    static napi_value IsDeviceActive(napi_env env, napi_callback_info info);
    static napi_value SetAudioParameter(napi_env env, napi_callback_info info);
    static napi_value GetAudioParameter(napi_env env, napi_callback_info info);
    static napi_value SetMicrophoneMute(napi_env env, napi_callback_info info);
    static napi_value IsMicrophoneMute(napi_env env, napi_callback_info info);

    static napi_status AddNamedProperty(napi_env env, napi_value object, const std::string name, int32_t enumValue);
    static napi_value CreateAudioVolumeTypeObject(napi_env env);
    static napi_value CreateDeviceFlagObject(napi_env env);
    static napi_value CreateDeviceRoleObject(napi_env env);
    static napi_value CreateDeviceTypeObject(napi_env env);
    static napi_value CreateActiveDeviceTypeObject(napi_env env);
    static napi_value CreateAudioRingModeObject(napi_env env);

    static napi_ref sConstructor_;
    static napi_ref audioVolumeTypeRef_;
    static napi_ref deviceFlagRef_;
    static napi_ref deviceRoleRef_;
    static napi_ref deviceTypeRef_;
    static napi_ref activeDeviceTypeRef_;
    static napi_ref audioRingModeRef_;

    AudioSystemManager *audioMngr_;
    napi_env env_;
    napi_ref wrapper_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif /* AUDIO_MNGR_NAPI_H_ */
