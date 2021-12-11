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

#include "video_asset_napi.h"
#include "hilog/log.h"

using OHOS::HiviewDFX::HiLog;
using OHOS::HiviewDFX::HiLogLabel;

namespace {
    constexpr HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "VideoAssetNapi"};
    const int32_t DEFAULT_VIDEO_WIDTH = 1280;
    const int32_t DEFAULT_VIDEO_HEIGHT = 720;
    const int32_t DEFAULT_VIDEO_DURATION = 0;
    const std::string DEFAULT_VIDEO_MIME_TYPE = "video/*";
}

namespace OHOS {
napi_ref VideoAssetNapi::sConstructor_ = nullptr;
Media::VideoAsset *VideoAssetNapi::sVideoAsset_ = nullptr;
Media::IMediaLibraryClient *VideoAssetNapi::sMediaLibrary_ = nullptr;

VideoAssetNapi::VideoAssetNapi()
    : env_(nullptr), wrapper_(nullptr)
{
    width_ = DEFAULT_VIDEO_WIDTH;
    height_ = DEFAULT_VIDEO_HEIGHT;
    duration_ = DEFAULT_VIDEO_DURATION;
    mimeType_ = DEFAULT_VIDEO_MIME_TYPE;
}

VideoAssetNapi::~VideoAssetNapi()
{
    if (wrapper_ != nullptr) {
        napi_delete_reference(env_, wrapper_);
    }
}

void VideoAssetNapi::VideoAssetNapiDestructor(napi_env env, void *nativeObject, void *finalize_hint)
{
    VideoAssetNapi *video = reinterpret_cast<VideoAssetNapi*>(nativeObject);
    if (video != nullptr) {
        video->~VideoAssetNapi();
    }
}

napi_value VideoAssetNapi::Init(napi_env env, napi_value exports)
{
    napi_status status;
    napi_value ctorObj;
    int32_t refCount = 1;

    napi_property_descriptor video_asset_props[] = {
        DECLARE_NAPI_GETTER("mimeType", GetMimeType),
        DECLARE_NAPI_GETTER("width", GetWidth),
        DECLARE_NAPI_GETTER("height", GetHeight),
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

    status = napi_define_class(env, VIDEO_ASSET_NAPI_CLASS_NAME.c_str(), NAPI_AUTO_LENGTH,
                               VideoAssetNapiConstructor, nullptr,
                               sizeof(video_asset_props) / sizeof(video_asset_props[PARAM0]),
                               video_asset_props, &ctorObj);
    if (status == napi_ok) {
        status = napi_create_reference(env, ctorObj, refCount, &sConstructor_);
        if (status == napi_ok) {
            status = napi_set_named_property(env, exports, VIDEO_ASSET_NAPI_CLASS_NAME.c_str(), ctorObj);
            if (status == napi_ok) {
                return exports;
            }
        }
    }

    return nullptr;
}

// Constructor callback
napi_value VideoAssetNapi::VideoAssetNapiConstructor(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value result = nullptr;
    napi_value thisVar = nullptr;

    napi_get_undefined(env, &result);
    GET_JS_OBJ_WITH_ZERO_ARGS(env, info, status, thisVar);
    if (status == napi_ok && thisVar != nullptr) {
        std::unique_ptr<VideoAssetNapi> obj = std::make_unique<VideoAssetNapi>();
        if (obj != nullptr) {
            obj->env_ = env;
            obj->SetMediaLibraryClient(*sMediaLibrary_);
            if (sVideoAsset_ != nullptr) {
                obj->UpdateVideoAssetInfo();
            } else {
                HiLog::Error(LABEL, "No native instance assigned yet");
                return result;
            }

            status = napi_wrap(env, thisVar, reinterpret_cast<void*>(obj.get()),
                               VideoAssetNapi::VideoAssetNapiDestructor, nullptr, &(obj->wrapper_));
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

napi_value VideoAssetNapi::CreateVideoAsset(napi_env env, Media::VideoAsset &vAsset,
    Media::IMediaLibraryClient &mediaLibClient)
{
    napi_status status;
    napi_value result = nullptr;
    napi_value constructor;

    status = napi_get_reference_value(env, sConstructor_, &constructor);
    if (status == napi_ok) {
        sVideoAsset_ = &vAsset;
        sMediaLibrary_ = &mediaLibClient;
        status = napi_new_instance(env, constructor, 0, nullptr, &result);
        sVideoAsset_ = nullptr;
        if (status == napi_ok && result != nullptr) {
            return result;
        } else {
            HiLog::Error(LABEL, "Failed to create video asset instance");
        }
    }

    napi_get_undefined(env, &result);
    return result;
}

napi_value VideoAssetNapi::GetMimeType(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value jsResult = nullptr;
    napi_value undefinedResult = nullptr;
    VideoAssetNapi* obj = nullptr;
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

napi_value VideoAssetNapi::GetWidth(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value jsResult = nullptr;
    napi_value undefinedResult = nullptr;
    VideoAssetNapi* obj = nullptr;
    int32_t width;
    napi_value thisVar = nullptr;

    napi_get_undefined(env, &undefinedResult);
    GET_JS_OBJ_WITH_ZERO_ARGS(env, info, status, thisVar);
    if (status != napi_ok || thisVar == nullptr) {
        HiLog::Error(LABEL, "Invalid arguments!");
        return undefinedResult;
    }

    status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status == napi_ok && obj != nullptr) {
        width = obj->width_;
        status = napi_create_int32(env, width, &jsResult);
        if (status == napi_ok) {
            return jsResult;
        }
    }

    return undefinedResult;
}

napi_value VideoAssetNapi::GetHeight(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value jsResult = nullptr;
    napi_value undefinedResult = nullptr;
    VideoAssetNapi* obj = nullptr;
    int32_t height;
    napi_value thisVar = nullptr;

    napi_get_undefined(env, &undefinedResult);
    GET_JS_OBJ_WITH_ZERO_ARGS(env, info, status, thisVar);
    if (status != napi_ok || thisVar == nullptr) {
        HiLog::Error(LABEL, "Invalid arguments!");
        return undefinedResult;
    }

    status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status == napi_ok && obj != nullptr) {
        height = obj->height_;
        status = napi_create_int32(env, height, &jsResult);
        if (status == napi_ok) {
            return jsResult;
        }
    }

    return undefinedResult;
}

napi_value VideoAssetNapi::GetDuration(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value jsResult = nullptr;
    napi_value undefinedResult = nullptr;
    VideoAssetNapi* obj = nullptr;
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

void VideoAssetNapi::UpdateVideoAssetInfo()
{
    this->SetId(sVideoAsset_->id_);
    this->SetUri(sVideoAsset_->uri_);
    this->SetMediaType(static_cast<int32_t>(sVideoAsset_->mediaType_));
    this->SetName(sVideoAsset_->name_);
    this->SetSize(sVideoAsset_->size_);
    this->SetDateAdded(sVideoAsset_->dateAdded_);
    this->SetDateModified(sVideoAsset_->dateModified_);
    this->SetAlbumName(sVideoAsset_->albumName_);
    this->SetAlbumId(sVideoAsset_->albumId_);
    mimeType_ = sVideoAsset_->mimeType_;
    width_  = sVideoAsset_->width_;
    height_ = sVideoAsset_->height_;
    duration_ = sVideoAsset_->duration_;
}
} // namespace OHOS
