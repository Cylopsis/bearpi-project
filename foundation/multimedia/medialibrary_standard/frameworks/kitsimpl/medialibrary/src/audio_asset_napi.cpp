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

#include "audio_asset_napi.h"
#include "hilog/log.h"

using OHOS::HiviewDFX::HiLog;
using OHOS::HiviewDFX::HiLogLabel;

namespace {
    constexpr HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "AudioAssetNapi"};
    const int32_t DEFAULT_AUDIO_DURATION = 0;
    const std::string DEFAULT_AUDIO_TITLE = "Unknown";
    const std::string DEFAULT_AUDIO_ARTIST = "Unknown";
    const std::string DEFAULT_AUDIO_MIME_TYPE = "audio/*";
}

namespace OHOS {
napi_ref AudioAssetNapi::sConstructor_ = nullptr;
Media::AudioAsset *AudioAssetNapi::sAudioAsset_ = nullptr;
Media::IMediaLibraryClient *AudioAssetNapi::sMediaLibrary_ = nullptr;

AudioAssetNapi::AudioAssetNapi()
    : env_(nullptr), wrapper_(nullptr)
{
    duration_ = DEFAULT_AUDIO_DURATION;
    title_ = DEFAULT_AUDIO_TITLE;
    artist_ = DEFAULT_AUDIO_ARTIST;
    mimeType_ = DEFAULT_AUDIO_MIME_TYPE;
}

AudioAssetNapi::~AudioAssetNapi()
{
    if (wrapper_ != nullptr) {
        napi_delete_reference(env_, wrapper_);
    }
}

void AudioAssetNapi::AudioAssetNapiDestructor(napi_env env, void *nativeObject, void *finalize_hint)
{
    AudioAssetNapi *audio = reinterpret_cast<AudioAssetNapi*>(nativeObject);
    if (audio != nullptr) {
        audio->~AudioAssetNapi();
    }
}

napi_value AudioAssetNapi::Init(napi_env env, napi_value exports)
{
    napi_status status;
    napi_value ctorObj;
    int32_t refCount = 1;

    napi_property_descriptor audio_asset_props[] = {
        DECLARE_NAPI_GETTER("mimeType", GetMimeType),
        DECLARE_NAPI_GETTER("title", GetTitle),
        DECLARE_NAPI_GETTER("artist", GetArtist),
        DECLARE_NAPI_GETTER("duration", GetDuration),
        DECLARE_NAPI_GETTER("id", GetId),
        DECLARE_NAPI_GETTER("URI", GetUri),
        DECLARE_NAPI_GETTER("mediaType", GetMediaType),
        DECLARE_NAPI_GETTER("size", GetSize),
        DECLARE_NAPI_GETTER("dateAdded", GetDateAdded),
        DECLARE_NAPI_GETTER("dateModified", GetDateModified),
        DECLARE_NAPI_GETTER("albumId", GetAlbumId),
        DECLARE_NAPI_GETTER_SETTER("albumName", GetAlbumName, JSSetAlbumName),
        DECLARE_NAPI_GETTER_SETTER("name", GetName, JSSetName),
        DECLARE_NAPI_FUNCTION("startCreate", StartCreate),
        DECLARE_NAPI_FUNCTION("cancelCreate", CancelCreate),
        DECLARE_NAPI_FUNCTION("commitCreate", CommitCreate),
        DECLARE_NAPI_FUNCTION("startModify", StartModify),
        DECLARE_NAPI_FUNCTION("cancelModify", CancelModify),
        DECLARE_NAPI_FUNCTION("commitModify", CommitModify),
        DECLARE_NAPI_FUNCTION("commitDelete", CommitDelete),
        DECLARE_NAPI_FUNCTION("commitCopy", CommitCopy),
    };

    status = napi_define_class(env, AUDIO_ASSET_NAPI_CLASS_NAME.c_str(), NAPI_AUTO_LENGTH,
                               AudioAssetNapiConstructor, nullptr,
                               sizeof(audio_asset_props) / sizeof(audio_asset_props[PARAM0]),
                               audio_asset_props, &ctorObj);
    if (status == napi_ok) {
        status = napi_create_reference(env, ctorObj, refCount, &sConstructor_);
        if (status == napi_ok) {
            status = napi_set_named_property(env, exports, AUDIO_ASSET_NAPI_CLASS_NAME.c_str(), ctorObj);
            if (status == napi_ok) {
                return exports;
            }
        }
    }

    return nullptr;
}

// Constructor callback
napi_value AudioAssetNapi::AudioAssetNapiConstructor(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value result = nullptr;
    napi_value thisVar = nullptr;

    napi_get_undefined(env, &result);
    GET_JS_OBJ_WITH_ZERO_ARGS(env, info, status, thisVar);
    if (status == napi_ok && thisVar != nullptr) {
        std::unique_ptr<AudioAssetNapi> obj = std::make_unique<AudioAssetNapi>();
        if (obj != nullptr) {
            obj->env_ = env;
            obj->SetMediaLibraryClient(*sMediaLibrary_);
            if (sAudioAsset_ != nullptr) {
                obj->UpdateAudioAssetInfo();
            } else {
                HiLog::Error(LABEL, "No native instance assigned yet");
                return result;
            }

            status = napi_wrap(env, thisVar, reinterpret_cast<void*>(obj.get()),
                               AudioAssetNapi::AudioAssetNapiDestructor, nullptr, &(obj->wrapper_));
            if (status == napi_ok) {
                obj.release();
                return thisVar;
            } else {
                HiLog::Error(LABEL, "Failure wrapping js to native napi");
            }
        }
    }

    return result;
}

napi_value AudioAssetNapi::CreateAudioAsset(napi_env env, Media::AudioAsset &aAsset,
    Media::IMediaLibraryClient &mediaLibClient)
{
    napi_status status;
    napi_value result = nullptr;
    napi_value constructor;

    status = napi_get_reference_value(env, sConstructor_, &constructor);
    if (status == napi_ok) {
        sAudioAsset_ = &aAsset;
        sMediaLibrary_ = &mediaLibClient;
        status = napi_new_instance(env, constructor, 0, nullptr, &result);
        sAudioAsset_ = nullptr;
        if (status == napi_ok && result != nullptr) {
            return result;
        } else {
            HiLog::Error(LABEL, "Failed to create audio asset instance");
        }
    }

    napi_get_undefined(env, &result);
    return result;
}

napi_value AudioAssetNapi::GetMimeType(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value jsResult = nullptr;
    napi_value undefinedResult = nullptr;
    AudioAssetNapi* obj = nullptr;
    std::string mimeType = "";
    napi_value thisVar = nullptr;

    napi_get_undefined(env, &undefinedResult);
    GET_JS_OBJ_WITH_ZERO_ARGS(env, info, status, thisVar);
    if (status != napi_ok || thisVar == nullptr) {
        HiLog::Error(LABEL, "Invalid arguments!");
        return undefinedResult;
    }

    status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status == napi_ok && obj != nullptr) {
        mimeType = obj->mimeType_;
        status = napi_create_string_utf8(env, mimeType.c_str(), NAPI_AUTO_LENGTH, &jsResult);
        if (status == napi_ok) {
            return jsResult;
        }
    }

    return undefinedResult;
}

napi_value AudioAssetNapi::GetTitle(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value jsResult = nullptr;
    napi_value undefinedResult = nullptr;
    AudioAssetNapi* obj = nullptr;
    std::string title = "";
    napi_value thisVar = nullptr;

    napi_get_undefined(env, &undefinedResult);
    GET_JS_OBJ_WITH_ZERO_ARGS(env, info, status, thisVar);
    if (status != napi_ok || thisVar == nullptr) {
        HiLog::Error(LABEL, "Invalid arguments!");
        return undefinedResult;
    }

    status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status == napi_ok && obj != nullptr) {
        title = obj->title_;
        status = napi_create_string_utf8(env, title.c_str(), NAPI_AUTO_LENGTH, &jsResult);
        if (status == napi_ok) {
            return jsResult;
        }
    }

    return undefinedResult;
}

napi_value AudioAssetNapi::GetArtist(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value jsResult = nullptr;
    napi_value undefinedResult = nullptr;
    AudioAssetNapi* obj = nullptr;
    std::string artist = "";
    napi_value thisVar = nullptr;

    napi_get_undefined(env, &undefinedResult);
    GET_JS_OBJ_WITH_ZERO_ARGS(env, info, status, thisVar);
    if (status != napi_ok || thisVar == nullptr) {
        HiLog::Error(LABEL, "Invalid arguments!");
        return undefinedResult;
    }

    status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status == napi_ok && obj != nullptr) {
        artist = obj->artist_;
        status = napi_create_string_utf8(env, artist.c_str(), NAPI_AUTO_LENGTH, &jsResult);
        if (status == napi_ok) {
            return jsResult;
        }
    }

    return undefinedResult;
}

napi_value AudioAssetNapi::GetDuration(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value jsResult = nullptr;
    napi_value undefinedResult = nullptr;
    AudioAssetNapi* obj = nullptr;
    int32_t duration;
    napi_value thisVar = nullptr;

    napi_get_undefined(env, &undefinedResult);
    GET_JS_OBJ_WITH_ZERO_ARGS(env, info, status, thisVar);
    if (status != napi_ok || thisVar == nullptr) {
        HiLog::Error(LABEL, "Invalid arguments!");
        return undefinedResult;
    }

    status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status == napi_ok && obj != nullptr) {
        duration = obj->duration_;
        status = napi_create_int32(env, duration, &jsResult);
        if (status == napi_ok) {
            return jsResult;
        }
    }

    return undefinedResult;
}

void AudioAssetNapi::UpdateAudioAssetInfo()
{
    this->SetId(sAudioAsset_->id_);
    this->SetUri(sAudioAsset_->uri_);
    this->SetMediaType(static_cast<int32_t>(sAudioAsset_->mediaType_));
    this->SetName(sAudioAsset_->name_);
    this->SetSize(sAudioAsset_->size_);
    this->SetDateAdded(sAudioAsset_->dateAdded_);
    this->SetDateModified(sAudioAsset_->dateModified_);
    this->SetAlbumName(sAudioAsset_->albumName_);
    this->SetAlbumId(sAudioAsset_->albumId_);
    mimeType_ = sAudioAsset_->mimeType_;
    title_  = sAudioAsset_->title_;
    artist_ = sAudioAsset_->artist_;
    duration_ = sAudioAsset_->duration_;
}
} // namespace OHOS
