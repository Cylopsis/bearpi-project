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

#include "image_asset_napi.h"
#include "hilog/log.h"

using OHOS::HiviewDFX::HiLog;
using OHOS::HiviewDFX::HiLogLabel;

namespace {
    constexpr HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "ImageAssetNapi"};
    const int32_t DEFAULT_IMAGE_WIDTH = 1280;
    const int32_t DEFAULT_IMAGE_HEIGHT = 720;
    const std::string DEFAULT_IMAGE_MIME_TYPE = "image/*";
}

namespace OHOS {
napi_ref ImageAssetNapi::sConstructor_ = nullptr;
Media::ImageAsset *ImageAssetNapi::sImageAsset_ = nullptr;
Media::IMediaLibraryClient *ImageAssetNapi::sMediaLibrary_ = nullptr;

ImageAssetNapi::ImageAssetNapi()
    : env_(nullptr), wrapper_(nullptr)
{
    width_ = DEFAULT_IMAGE_WIDTH;
    height_ = DEFAULT_IMAGE_HEIGHT;
    mimeType_ = DEFAULT_IMAGE_MIME_TYPE;
}

ImageAssetNapi::~ImageAssetNapi()
{
    if (wrapper_ != nullptr) {
        napi_delete_reference(env_, wrapper_);
    }
}

void ImageAssetNapi::ImageAssetNapiDestructor(napi_env env, void *nativeObject, void *finalize_hint)
{
    ImageAssetNapi *image = reinterpret_cast<ImageAssetNapi*>(nativeObject);
    if (image != nullptr) {
        image->~ImageAssetNapi();
    }
}

napi_value ImageAssetNapi::Init(napi_env env, napi_value exports)
{
    napi_status status;
    napi_value ctorObj;
    int32_t refCount = 1;

    napi_property_descriptor image_asset_props[] = {
        DECLARE_NAPI_GETTER("mimeType", GetMimeType),
        DECLARE_NAPI_GETTER("width", GetWidth),
        DECLARE_NAPI_GETTER("height", GetHeight),
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

    status = napi_define_class(env, IMAGE_ASSET_NAPI_CLASS_NAME.c_str(), NAPI_AUTO_LENGTH,
                               ImageAssetNapiConstructor, nullptr,
                               sizeof(image_asset_props) / sizeof(image_asset_props[PARAM0]),
                               image_asset_props, &ctorObj);
    if (status == napi_ok) {
        status = napi_create_reference(env, ctorObj, refCount, &sConstructor_);
        if (status == napi_ok) {
            status = napi_set_named_property(env, exports, IMAGE_ASSET_NAPI_CLASS_NAME.c_str(), ctorObj);
            if (status == napi_ok) {
                return exports;
            }
        }
    }

    return nullptr;
}

// Constructor callback
napi_value ImageAssetNapi::ImageAssetNapiConstructor(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value result = nullptr;
    napi_value thisVar = nullptr;

    napi_get_undefined(env, &result);
    GET_JS_OBJ_WITH_ZERO_ARGS(env, info, status, thisVar);
    if (status == napi_ok && thisVar != nullptr) {
        std::unique_ptr<ImageAssetNapi> obj = std::make_unique<ImageAssetNapi>();
        if (obj != nullptr) {
            obj->env_ = env;
            obj->SetMediaLibraryClient(*sMediaLibrary_);
            if (sImageAsset_ != nullptr) {
                obj->UpdateImageAssetInfo();
            } else {
                HiLog::Error(LABEL, "No native instance assigned yet");
                return result;
            }

            status = napi_wrap(env, thisVar, reinterpret_cast<void*>(obj.get()),
                               ImageAssetNapi::ImageAssetNapiDestructor, nullptr, &(obj->wrapper_));
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

napi_value ImageAssetNapi::CreateImageAsset(napi_env env, Media::ImageAsset &iAsset,
    Media::IMediaLibraryClient &mediaLibClient)
{
    napi_status status;
    napi_value result = nullptr;
    napi_value constructor;

    status = napi_get_reference_value(env, sConstructor_, &constructor);
    if (status == napi_ok) {
        sImageAsset_ = &iAsset;
        sMediaLibrary_ = &mediaLibClient;
        status = napi_new_instance(env, constructor, 0, nullptr, &result);
        sImageAsset_ = nullptr;
        if (status == napi_ok && result != nullptr) {
            return result;
        } else {
            HiLog::Error(LABEL, "Failed to create image asset instance");
        }
    }

    napi_get_undefined(env, &result);
    return result;
}

napi_value ImageAssetNapi::GetMimeType(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value jsResult = nullptr;
    napi_value undefinedResult = nullptr;
    ImageAssetNapi* obj = nullptr;
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

napi_value ImageAssetNapi::GetWidth(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value jsResult = nullptr;
    napi_value undefinedResult = nullptr;
    ImageAssetNapi* obj = nullptr;
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

napi_value ImageAssetNapi::GetHeight(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value jsResult = nullptr;
    napi_value undefinedResult = nullptr;
    ImageAssetNapi* obj = nullptr;
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

void ImageAssetNapi::UpdateImageAssetInfo()
{
    this->SetId(sImageAsset_->id_);
    this->SetUri(sImageAsset_->uri_);
    this->SetMediaType(static_cast<int32_t>(sImageAsset_->mediaType_));
    this->SetName(sImageAsset_->name_);
    this->SetSize(sImageAsset_->size_);
    this->SetDateAdded(sImageAsset_->dateAdded_);
    this->SetDateModified(sImageAsset_->dateModified_);
    this->SetAlbumName(sImageAsset_->albumName_);
    this->SetAlbumId(sImageAsset_->albumId_);
    mimeType_ = sImageAsset_->mimeType_;
    width_  = sImageAsset_->width_;
    height_ = sImageAsset_->height_;
}
} // namespace OHOS
