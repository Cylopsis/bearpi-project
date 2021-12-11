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

#include "audio_device_descriptor_napi.h"
#include "audio_system_manager.h"
#include "hilog/log.h"

using namespace std;
using OHOS::HiviewDFX::HiLog;
using OHOS::HiviewDFX::HiLogLabel;

namespace OHOS {
namespace AudioStandard {
napi_ref AudioDeviceDescriptorNapi::sConstructor_ = nullptr;
sptr<AudioDeviceDescriptor> AudioDeviceDescriptorNapi::sAudioDescriptor_ = nullptr;

namespace {
    constexpr HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "AudioDeviceDescriptorNapi"};
}

AudioDeviceDescriptorNapi::AudioDeviceDescriptorNapi()
    : env_(nullptr), wrapper_(nullptr) {
}

AudioDeviceDescriptorNapi::~AudioDeviceDescriptorNapi()
{
    if (wrapper_ != nullptr) {
        napi_delete_reference(env_, wrapper_);
    }
    audioDescriptor_ = nullptr;
}

void AudioDeviceDescriptorNapi::Destructor(napi_env env, void *nativeObject, void *finalize_hint)
{
    if (nativeObject != nullptr) {
        auto obj = static_cast<AudioDeviceDescriptorNapi*>(nativeObject);
        delete obj;
    }
}

napi_value AudioDeviceDescriptorNapi::Init(napi_env env, napi_value exports)
{
    napi_status status;
    napi_value constructor;
    napi_property_descriptor audio_dev_desc_properties[] = {
        DECLARE_NAPI_GETTER("deviceRole", GetDeviceRole),
        DECLARE_NAPI_GETTER("deviceType", GetDeviceType)
    };
    const int32_t refCount = 1;

    status = napi_define_class(env, AUDIO_DEVICE_DESCRIPTOR_NAPI_CLASS_NAME.c_str(), NAPI_AUTO_LENGTH, Construct,
                               nullptr, sizeof(audio_dev_desc_properties) / sizeof(audio_dev_desc_properties[0]),
                               audio_dev_desc_properties, &constructor);
    if (status == napi_ok) {
        status = napi_create_reference(env, constructor, refCount, &sConstructor_);
        if (status == napi_ok) {
            status = napi_set_named_property(env, exports, AUDIO_DEVICE_DESCRIPTOR_NAPI_CLASS_NAME.c_str(),
                                             constructor);
            if (status == napi_ok) {
                return exports;
            }
        }
    }
    HiLog::Error(LABEL, "Failure in AudioDeviceDescriptorNapi::Init()");

    return exports;
}

napi_value AudioDeviceDescriptorNapi::Construct(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value jsThis = nullptr;
    size_t argCount = 0;

    status = napi_get_cb_info(env, info, &argCount, nullptr, &jsThis, nullptr);
    if (status == napi_ok) {
        unique_ptr<AudioDeviceDescriptorNapi> obj = make_unique<AudioDeviceDescriptorNapi>();
        if (obj != nullptr) {
            obj->env_ = env;
            obj->audioDescriptor_ = sAudioDescriptor_;
            status = napi_wrap(env, jsThis, static_cast<void*>(obj.get()),
                               AudioDeviceDescriptorNapi::Destructor, nullptr, &(obj->wrapper_));
            if (status == napi_ok) {
                obj.release();
                return jsThis;
            }
        }
    }
    HiLog::Error(LABEL, "Failed in AudioDeviceDescriptorNapi::Construct()!");
    napi_get_undefined(env, &jsThis);

    return jsThis;
}

napi_value AudioDeviceDescriptorNapi::CreateAudioDeviceDescriptorWrapper(napi_env env,
    sptr<AudioDeviceDescriptor> deviceDescriptor)
{
    napi_status status;
    napi_value result = nullptr;
    napi_value constructor;

    if (deviceDescriptor != nullptr) {
        status = napi_get_reference_value(env, sConstructor_, &constructor);
        if (status == napi_ok) {
            sAudioDescriptor_ = deviceDescriptor;
            status = napi_new_instance(env, constructor, 0, nullptr, &result);
            sAudioDescriptor_ = nullptr;
            if (status == napi_ok) {
                return result;
            }
        }
        HiLog::Error(LABEL, "Failed in CreateAudioDeviceDescriptorWrapper, %{public}d", status);
    } else {
        HiLog::Error(LABEL, "sptr<AudioDeviceDescriptor> is null");
    }
    napi_get_undefined(env, &result);

    return result;
}

napi_value AudioDeviceDescriptorNapi::GetDeviceRole(napi_env env, napi_callback_info info)
{
    napi_status status;
    AudioDeviceDescriptorNapi *deviceDescriptor = nullptr;
    size_t argc = 0;
    napi_value thisVar = nullptr;
    AudioDeviceDescriptor::DeviceRole deviceRole;
    napi_value jsResult = nullptr;

    napi_get_cb_info(env, info, &argc, nullptr, &thisVar, nullptr);
    NAPI_ASSERT(env, argc == 0, "Invalid number of arguments");
    status = napi_unwrap(env, thisVar, (void **)&deviceDescriptor);
    if (status == napi_ok) {
        deviceRole = deviceDescriptor->audioDescriptor_->deviceRole_;
        status = napi_create_int32(env, deviceRole, &jsResult);
        if (status == napi_ok) {
            return jsResult;
        }
    }
    napi_get_undefined(env, &jsResult);
    return jsResult;
}

napi_value AudioDeviceDescriptorNapi::GetDeviceType(napi_env env, napi_callback_info info)
{
    napi_status status;
    size_t argc = 0;
    napi_value thisVar = nullptr;
    AudioDeviceDescriptor::DeviceType deviceType;
    napi_value jsResult = nullptr;
    AudioDeviceDescriptorNapi* deviceDescriptor = nullptr;

    napi_get_cb_info(env, info, &argc, nullptr, &thisVar, nullptr);
    NAPI_ASSERT(env, argc == 0, "Invalid number of arguments");
    status = napi_unwrap(env, thisVar, (void **)&deviceDescriptor);
    if (status == napi_ok) {
        deviceType = deviceDescriptor->audioDescriptor_->deviceType_;
        status = napi_create_int32(env, deviceType, &jsResult);
        if (status == napi_ok) {
            return jsResult;
        }
    }
    napi_get_undefined(env, &jsResult);
    return jsResult;
}
} // namespace AudioStandard
} // namespace OHOS
