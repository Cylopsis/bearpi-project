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

#include "media_asset_napi.h"
#include "hilog/log.h"

using OHOS::HiviewDFX::HiLog;
using OHOS::HiviewDFX::HiLogLabel;

namespace {
    constexpr HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "MediaAssetNapi"};
    const int32_t DEFAULT_MEDIA_ID = 0;
    const int32_t DEFAULT_ALBUM_ID = 0;
    const uint64_t DEFAULT_MEDIA_SIZE = 0;
    const uint64_t DEFAULT_MEDIA_DATE_ADDED = 0;
    const uint64_t DEFAULT_MEDIA_DATE_MODIFIED = 0;
    const std::string DEFAULT_MEDIA_URI = "";
    const OHOS::Media::MediaType DEFAULT_MEDIA_TYPE = OHOS::Media::MEDIA_TYPE_FILE;
    const std::string DEFAULT_MEDIA_NAME = "Unknown";
    const std::string DEFAULT_ALBUM_NAME = "Unknown";
}

namespace OHOS {
napi_ref MediaAssetNapi::sConstructor_ = nullptr;
Media::MediaAsset *MediaAssetNapi::sMediaAsset_ = nullptr;
Media::IMediaLibraryClient *MediaAssetNapi::sMediaLibrary_ = nullptr;

Media::AssetType GetAssetType(Media::MediaType type)
{
    Media::AssetType result;

    switch (type) {
        case Media::MEDIA_TYPE_AUDIO:
            result = Media::ASSET_AUDIO;
            break;
        case Media::MEDIA_TYPE_VIDEO:
            result = Media::ASSET_VIDEO;
            break;
        case Media::MEDIA_TYPE_IMAGE:
            result = Media::ASSET_IMAGE;
            break;
        case Media::MEDIA_TYPE_MEDIA:
            result = Media::ASSET_MEDIA;
            break;
        default:
            result = Media::ASSET_NONE;
            break;
    }

    return result;
}

MediaAssetNapi::MediaAssetNapi()
    : env_(nullptr), wrapper_(nullptr)
{
    mediaLibrary_ = nullptr;
    id_ = DEFAULT_MEDIA_ID;
    albumId_ = DEFAULT_ALBUM_ID;
    albumName_ = DEFAULT_ALBUM_NAME;
    size_ = DEFAULT_MEDIA_SIZE;
    uri_ = DEFAULT_MEDIA_URI;
    mediaType_ = static_cast<int32_t>(DEFAULT_MEDIA_TYPE);
    name_ = DEFAULT_MEDIA_NAME;
    dateAdded_ = DEFAULT_MEDIA_DATE_ADDED;
    dateModified_ = DEFAULT_MEDIA_DATE_MODIFIED;
}

MediaAssetNapi::~MediaAssetNapi()
{
    if (wrapper_ != nullptr) {
        napi_delete_reference(env_, wrapper_);
    }
}

void MediaAssetNapi::MediaAssetNapiDestructor(napi_env env, void *nativeObject, void *finalize_hint)
{
    MediaAssetNapi *media = reinterpret_cast<MediaAssetNapi*>(nativeObject);
    if (media != nullptr) {
        media->~MediaAssetNapi();
    }
}

napi_value MediaAssetNapi::Init(napi_env env, napi_value exports)
{
    napi_status status;
    napi_value ctorObj;
    int32_t refCount = 1;

    napi_property_descriptor media_asset_properties[] = {
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

    status = napi_define_class(env, MEDIA_ASSET_NAPI_CLASS_NAME.c_str(), NAPI_AUTO_LENGTH,
                               MediaAssetNapiConstructor, nullptr,
                               sizeof(media_asset_properties) / sizeof(media_asset_properties[PARAM0]),
                               media_asset_properties, &ctorObj);
    if (status == napi_ok) {
        status = napi_create_reference(env, ctorObj, refCount, &sConstructor_);
        if (status == napi_ok) {
            status = napi_set_named_property(env, exports, MEDIA_ASSET_NAPI_CLASS_NAME.c_str(), ctorObj);
            if (status == napi_ok) {
                return exports;
            }
        }
    }

    return nullptr;
}

// Constructor callback
napi_value MediaAssetNapi::MediaAssetNapiConstructor(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value result = nullptr;
    napi_value thisVar = nullptr;

    napi_get_undefined(env, &result);
    GET_JS_OBJ_WITH_ZERO_ARGS(env, info, status, thisVar);
    if (status == napi_ok && thisVar != nullptr) {
        std::unique_ptr<MediaAssetNapi> obj = std::make_unique<MediaAssetNapi>();
        if (obj != nullptr) {
            obj->env_ = env;
            obj->mediaLibrary_ = sMediaLibrary_;
            if (sMediaAsset_ != nullptr) {
                obj->UpdateMediaAssetInfo(*sMediaAsset_);
            } else {
                HiLog::Error(LABEL, "No native instance assigned yet");
                return result;
            }

            status = napi_wrap(env, thisVar, reinterpret_cast<void*>(obj.get()),
                               MediaAssetNapi::MediaAssetNapiDestructor, nullptr, &(obj->wrapper_));
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

napi_value MediaAssetNapi::CreateMediaAsset(napi_env env, Media::MediaAsset &mAsset,
    Media::IMediaLibraryClient &mediaLibClient)
{
    napi_status status;
    napi_value result = nullptr;
    napi_value constructor;

    status = napi_get_reference_value(env, sConstructor_, &constructor);
    if (status == napi_ok) {
        sMediaAsset_ = &mAsset;
        sMediaLibrary_ = &mediaLibClient;
        status = napi_new_instance(env, constructor, 0, nullptr, &result);
        sMediaAsset_ = nullptr;
        if (status == napi_ok && result != nullptr) {
            return result;
        } else {
            HiLog::Error(LABEL, "Failed to create media asset instance");
        }
    }

    napi_get_undefined(env, &result);
    return result;
}

void MediaAssetNapi::SetId(int32_t id)
{
    this->id_ = id;
}

void MediaAssetNapi::SetUri(std::string uri)
{
    this->uri_ = uri;
}

void MediaAssetNapi::SetMediaType(int32_t mediaType)
{
    this->mediaType_ = mediaType;
}

void MediaAssetNapi::SetName(std::string name)
{
    this->name_ = name;
}

void MediaAssetNapi::SetSize(uint64_t size)
{
    this->size_ = size;
}

void MediaAssetNapi::SetDateAdded(uint64_t dateAdded)
{
    this->dateAdded_ = dateAdded;
}

void MediaAssetNapi::SetDateModified(uint64_t dateModified)
{
    this->dateModified_ = dateModified;
}

void MediaAssetNapi::SetAlbumName(std::string albumName)
{
    this->albumName_ = albumName;
}

void MediaAssetNapi::SetAlbumId(int32_t albumId)
{
    this->albumId_ = albumId;
}

void MediaAssetNapi::SetMediaLibraryClient(Media::IMediaLibraryClient &mediaLibrary)
{
    this->mediaLibrary_ = &mediaLibrary;
}

napi_value MediaAssetNapi::GetId(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value jsResult = nullptr;
    napi_value undefinedResult = nullptr;
    MediaAssetNapi* obj = nullptr;
    int32_t id;
    napi_value thisVar = nullptr;

    napi_get_undefined(env, &undefinedResult);
    GET_JS_OBJ_WITH_ZERO_ARGS(env, info, status, thisVar);
    if (status != napi_ok || thisVar == nullptr) {
        HiLog::Error(LABEL, "Invalid arguments!");
        return undefinedResult;
    }

    status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status == napi_ok && obj != nullptr) {
        id = obj->id_;
        status = napi_create_uint32(env, id, &jsResult);
        if (status == napi_ok) {
            return jsResult;
        }
    }

    return undefinedResult;
}

napi_value MediaAssetNapi::GetUri(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value jsResult = nullptr;
    napi_value undefinedResult = nullptr;
    MediaAssetNapi* obj = nullptr;
    std::string uri = "";
    napi_value thisVar = nullptr;

    napi_get_undefined(env, &undefinedResult);
    GET_JS_OBJ_WITH_ZERO_ARGS(env, info, status, thisVar);
    if (status != napi_ok || thisVar == nullptr) {
        HiLog::Error(LABEL, "Invalid arguments!");
        return undefinedResult;
    }

    status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status == napi_ok && obj != nullptr) {
        uri = obj->uri_;
        status = napi_create_string_utf8(env, uri.c_str(), NAPI_AUTO_LENGTH, &jsResult);
        if (status == napi_ok) {
            return jsResult;
        }
    }

    return undefinedResult;
}

napi_value MediaAssetNapi::GetMediaType(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value jsResult = nullptr;
    napi_value undefinedResult = nullptr;
    MediaAssetNapi* obj = nullptr;
    int32_t mediaType;
    napi_value thisVar = nullptr;

    napi_get_undefined(env, &undefinedResult);
    GET_JS_OBJ_WITH_ZERO_ARGS(env, info, status, thisVar);
    if (status != napi_ok || thisVar == nullptr) {
        HiLog::Error(LABEL, "Invalid arguments!");
        return undefinedResult;
    }

    status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status == napi_ok && obj != nullptr) {
        mediaType = obj->mediaType_;
        status = napi_create_int32(env, mediaType, &jsResult);
        if (status == napi_ok) {
            return jsResult;
        }
    }

    return undefinedResult;
}

napi_value MediaAssetNapi::JSSetName(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value undefinedResult = nullptr;
    MediaAssetNapi* obj = nullptr;
    napi_valuetype valueType = napi_undefined;
    size_t res = 0;
    char buffer[SIZE];
    size_t argc = ARGS_ONE;
    napi_value argv[ARGS_ONE] = {0};
    napi_value thisVar = nullptr;

    napi_get_undefined(env, &undefinedResult);

    GET_JS_ARGS(env, info, argc, argv, thisVar);
    NAPI_ASSERT(env, argc == ARGS_ONE, "requires 1 parameter");

    status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status == napi_ok && obj != nullptr) {
        if (obj->startCreateFlag == false && obj->startModifyFlag == false) {
            HiLog::Error(LABEL, "No Permission to set the values");
            return undefinedResult;
        }

        if (napi_typeof(env, argv[PARAM0], &valueType) != napi_ok || valueType != napi_string) {
            HiLog::Error(LABEL, "Invalid arguments type!");
            return undefinedResult;
        }

        status = napi_get_value_string_utf8(env, argv[PARAM0], buffer, SIZE, &res);
        if (status == napi_ok) {
            obj->newName_ = buffer;
            return undefinedResult;
        }
    }

    return undefinedResult;
}

napi_value MediaAssetNapi::GetName(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value jsResult = nullptr;
    napi_value undefinedResult = nullptr;
    MediaAssetNapi* obj = nullptr;
    std::string name = "";
    napi_value thisVar = nullptr;

    napi_get_undefined(env, &undefinedResult);
    GET_JS_OBJ_WITH_ZERO_ARGS(env, info, status, thisVar);
    if (status != napi_ok || thisVar == nullptr) {
        HiLog::Error(LABEL, "Invalid arguments!");
        return undefinedResult;
    }

    status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status == napi_ok && obj != nullptr) {
        name = obj->name_;
        status = napi_create_string_utf8(env, name.c_str(), NAPI_AUTO_LENGTH, &jsResult);
        if (status == napi_ok) {
            return jsResult;
        }
    }

    return undefinedResult;
}

napi_value MediaAssetNapi::GetSize(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value jsResult = nullptr;
    napi_value undefinedResult = nullptr;
    MediaAssetNapi* obj = nullptr;
    uint64_t size;
    napi_value thisVar = nullptr;

    napi_get_undefined(env, &undefinedResult);
    GET_JS_OBJ_WITH_ZERO_ARGS(env, info, status, thisVar);
    if (status != napi_ok || thisVar == nullptr) {
        HiLog::Error(LABEL, "Invalid arguments!");
        return undefinedResult;
    }

    status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status == napi_ok && obj != nullptr) {
        size = obj->size_;
        status = napi_create_int64(env, size, &jsResult);
        if (status == napi_ok) {
            return jsResult;
        }
    }

    return undefinedResult;
}

napi_value MediaAssetNapi::GetDateAdded(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value jsResult = nullptr;
    napi_value undefinedResult = nullptr;
    MediaAssetNapi* obj = nullptr;
    uint64_t dateAdded;
    napi_value thisVar = nullptr;

    napi_get_undefined(env, &undefinedResult);
    GET_JS_OBJ_WITH_ZERO_ARGS(env, info, status, thisVar);
    if (status != napi_ok || thisVar == nullptr) {
        HiLog::Error(LABEL, "Invalid arguments!");
        return undefinedResult;
    }

    status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status == napi_ok && obj != nullptr) {
        dateAdded = obj->dateAdded_;
        status = napi_create_int64(env, dateAdded, &jsResult);
        if (status == napi_ok) {
            return jsResult;
        }
    }

    return undefinedResult;
}

napi_value MediaAssetNapi::GetDateModified(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value jsResult = nullptr;
    napi_value undefinedResult = nullptr;
    MediaAssetNapi* obj = nullptr;
    uint64_t dateModified;
    napi_value thisVar = nullptr;

    napi_get_undefined(env, &undefinedResult);
    GET_JS_OBJ_WITH_ZERO_ARGS(env, info, status, thisVar);
    if (status != napi_ok || thisVar == nullptr) {
        HiLog::Error(LABEL, "Invalid arguments!");
        return undefinedResult;
    }

    status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status == napi_ok && obj != nullptr) {
        dateModified = obj->dateModified_;
        status = napi_create_int64(env, dateModified, &jsResult);
        if (status == napi_ok) {
            return jsResult;
        }
    }

    return undefinedResult;
}

napi_value MediaAssetNapi::GetAlbumId(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value jsResult = nullptr;
    napi_value undefinedResult = nullptr;
    MediaAssetNapi* obj = nullptr;
    int32_t id;
    napi_value thisVar = nullptr;

    napi_get_undefined(env, &undefinedResult);
    GET_JS_OBJ_WITH_ZERO_ARGS(env, info, status, thisVar);
    if (status != napi_ok || thisVar == nullptr) {
        HiLog::Error(LABEL, "Invalid arguments!");
        return undefinedResult;
    }

    status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status == napi_ok && obj != nullptr) {
        id = obj->albumId_;
        status = napi_create_int32(env, id, &jsResult);
        if (status == napi_ok) {
            return jsResult;
        }
    }

    return undefinedResult;
}

napi_value MediaAssetNapi::JSSetAlbumName(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value undefinedResult = nullptr;
    MediaAssetNapi* obj = nullptr;
    napi_valuetype valueType = napi_undefined;
    size_t res = 0;
    char buffer[SIZE];
    size_t argc = ARGS_ONE;
    napi_value argv[ARGS_ONE] = {0};
    napi_value thisVar = nullptr;

    napi_get_undefined(env, &undefinedResult);

    GET_JS_ARGS(env, info, argc, argv, thisVar);
    NAPI_ASSERT(env, argc == ARGS_ONE, "requires 1 parameter");

    status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status == napi_ok && obj != nullptr) {
        if (obj->startCreateFlag == false) {
            HiLog::Error(LABEL, "No Permission to set the values");
            return undefinedResult;
        }

        if (napi_typeof(env, argv[PARAM0], &valueType) != napi_ok || valueType != napi_string) {
            HiLog::Error(LABEL, "Invalid arguments type!");
            return undefinedResult;
        }

        status = napi_get_value_string_utf8(env, argv[PARAM0], buffer, SIZE, &res);
        if (status == napi_ok) {
            obj->newAlbumName_ = buffer;
            return undefinedResult;
        }
    }

    return undefinedResult;
}

napi_value MediaAssetNapi::GetAlbumName(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value jsResult = nullptr;
    napi_value undefinedResult = nullptr;
    MediaAssetNapi* obj = nullptr;
    std::string name = "";
    napi_value thisVar = nullptr;

    napi_get_undefined(env, &undefinedResult);
    GET_JS_OBJ_WITH_ZERO_ARGS(env, info, status, thisVar);
    if (status != napi_ok || thisVar == nullptr) {
        HiLog::Error(LABEL, "Invalid arguments!");
        return undefinedResult;
    }

    status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status == napi_ok && obj != nullptr) {
        name = obj->albumName_;
        status = napi_create_string_utf8(env, name.c_str(), NAPI_AUTO_LENGTH, &jsResult);
        if (status == napi_ok) {
            return jsResult;
        }
    }

    return undefinedResult;
}

void MediaAssetNapi::UpdateMediaAssetInfo(OHOS::Media::MediaAsset &mAsset)
{
    OHOS::Media::MediaAsset *mediaAsset = &mAsset;

    id_ = mediaAsset->id_;
    uri_ = mediaAsset->uri_;
    mediaType_ = static_cast<int32_t>(mediaAsset->mediaType_);
    name_ = mediaAsset->name_;
    size_ = mediaAsset->size_;
    dateAdded_ = mediaAsset->dateAdded_;
    dateModified_ = mediaAsset->dateModified_;
    albumId_ = mediaAsset->albumId_;
    albumName_ = mediaAsset->albumName_;
}

static void CommonCompleteCallback(napi_env env, napi_status status, void* data)
{
    auto context = static_cast<MediaAssetAsyncContext*>(data);
    napi_value result[ARGS_TWO] = {0};

    if (context == nullptr) {
        HiLog::Error(LABEL, "Async context is null");
        return;
    }

    napi_get_undefined(env, &result[PARAM0]);
    napi_get_boolean(env, context->status, &result[PARAM1]);

    if (context->work != nullptr) {
        MediaLibraryNapiUtils::InvokeJSAsyncMethod(env, context->deferred, result, ARGS_TWO,
                                                   context->callbackRef, context->work);
    }
    delete context;
}

napi_value MediaAssetNapi::StartCreate(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value result = nullptr;
    const int32_t refCount = 1;
    napi_value resource = nullptr;
    size_t argc = ARGS_ONE;
    napi_value argv[ARGS_ONE] = {0};
    napi_value thisVar = nullptr;

    GET_JS_ARGS(env, info, argc, argv, thisVar);
    NAPI_ASSERT(env, argc <= 1, "requires 1 parameter maximum");

    napi_get_undefined(env, &result);
    std::unique_ptr<MediaAssetAsyncContext> asyncContext = std::make_unique<MediaAssetAsyncContext>();
    status = napi_unwrap(env, thisVar, reinterpret_cast<void**>(&asyncContext->objectInfo));
    if (status == napi_ok && asyncContext->objectInfo != nullptr) {
        if (argc == ARGS_ONE) {
            GET_JS_ASYNC_CB_REF(env, argv[PARAM0], refCount, asyncContext->callbackRef);
        }

        NAPI_CREATE_PROMISE(env, asyncContext->callbackRef, asyncContext->deferred, result);
        NAPI_CREATE_RESOURCE_NAME(env, resource, "StartCreate");

        status = napi_create_async_work(
            env, nullptr, resource, [](napi_env env, void* data) {
                auto context = static_cast<MediaAssetAsyncContext*>(data);
                context->objectInfo->startCreateFlag = true;
                context->status = true;
            },
            CommonCompleteCallback, static_cast<void*>(asyncContext.get()), &asyncContext->work);
        if (status != napi_ok) {
            napi_get_undefined(env, &result);
        } else {
            napi_queue_async_work(env, asyncContext->work);
            asyncContext.release();
        }
    }

    return result;
}

void MediaAssetNapi::UpdateNativeMediaAsset(Media::MediaAsset& mAsset)
{
    mAsset.id_ = id_;
    mAsset.uri_ = uri_;
    mAsset.mediaType_ = static_cast<OHOS::Media::MediaType>(mediaType_);
    mAsset.name_ = name_;
    mAsset.size_ = size_;
    mAsset.dateAdded_ = dateAdded_;
    mAsset.dateModified_ = dateModified_;
    mAsset.albumId_ = albumId_;
    mAsset.albumName_ = albumName_;
}

napi_value MediaAssetNapi::CommitCreate(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value result = nullptr;
    napi_value resource = nullptr;
    size_t argc = ARGS_ONE;
    napi_value argv[ARGS_ONE] = {0};
    napi_value thisVar = nullptr;

    GET_JS_ARGS(env, info, argc, argv, thisVar);
    NAPI_ASSERT(env, argc <= ARGS_ONE, "requires 1 parameter maximum");

    napi_get_undefined(env, &result);
    std::unique_ptr<MediaAssetAsyncContext> asyncContext = std::make_unique<MediaAssetAsyncContext>();
    status = napi_unwrap(env, thisVar, reinterpret_cast<void**>(&asyncContext->objectInfo));
    if (status == napi_ok && asyncContext->objectInfo != nullptr) {
        if (argc == ARGS_ONE) {
            GET_JS_ASYNC_CB_REF(env, argv[PARAM0], REFERENCE_COUNT_ONE, asyncContext->callbackRef);
        }

        NAPI_CREATE_PROMISE(env, asyncContext->callbackRef, asyncContext->deferred, result);
        NAPI_CREATE_RESOURCE_NAME(env, resource, "CommitCreate");

        status = napi_create_async_work(env, nullptr, resource, [](napi_env env, void* data) {
            auto context = static_cast<MediaAssetAsyncContext*>(data);
            Media::MediaAsset asset;

            context->objectInfo->UpdateNativeMediaAsset(asset);
            context->status = false;
            if (!context->objectInfo->newAlbumName_.empty()) {
                asset.albumName_ = context->objectInfo->newAlbumName_;
            } else {
                // New album name not provided
                asset.albumName_ = "";
            }
            if (!context->objectInfo->newName_.empty()) {
                asset.name_ = context->objectInfo->newName_;
                context->status =
                    context->objectInfo->mediaLibrary_->CreateMediaAsset(GetAssetType(asset.mediaType_), asset);
                if (context->status) {
                    context->objectInfo->UpdateMediaAssetInfo(asset);
                }
            }

            context->objectInfo->newName_ = "";
            context->objectInfo->newAlbumName_ = "";
            context->objectInfo->startCreateFlag = false;
        }, CommonCompleteCallback, static_cast<void*>(asyncContext.get()), &asyncContext->work);
        if (status != napi_ok) {
            napi_get_undefined(env, &result);
        } else {
            napi_queue_async_work(env, asyncContext->work);
            asyncContext.release();
        }
    }

    return result;
}

napi_value MediaAssetNapi::CancelCreate(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value result = nullptr;
    const int32_t refCount = 1;
    napi_value resource = nullptr;
    size_t argc = ARGS_ONE;
    napi_value argv[ARGS_ONE] = {0};
    napi_value thisVar = nullptr;

    GET_JS_ARGS(env, info, argc, argv, thisVar);
    NAPI_ASSERT(env, argc <= 1, "requires 1 parameter maximum");

    napi_get_undefined(env, &result);
    std::unique_ptr<MediaAssetAsyncContext> asyncContext = std::make_unique<MediaAssetAsyncContext>();
    status = napi_unwrap(env, thisVar, reinterpret_cast<void**>(&asyncContext->objectInfo));
    if (status == napi_ok && asyncContext->objectInfo != nullptr) {
        if (argc == ARGS_ONE) {
            GET_JS_ASYNC_CB_REF(env, argv[PARAM0], refCount, asyncContext->callbackRef);
        }

        NAPI_CREATE_PROMISE(env, asyncContext->callbackRef, asyncContext->deferred, result);
        NAPI_CREATE_RESOURCE_NAME(env, resource, "CancelCreate");

        status = napi_create_async_work(
            env, nullptr, resource, [](napi_env env, void* data) {
                auto context = static_cast<MediaAssetAsyncContext*>(data);
                if (!context->objectInfo->newName_.empty()) {
                    context->objectInfo->newName_ = "";
                }
                if (!context->objectInfo->newAlbumName_.empty()) {
                    context->objectInfo->newAlbumName_ = "";
                }
                context->objectInfo->startCreateFlag = false;
                context->status = true;
            },
            CommonCompleteCallback, static_cast<void*>(asyncContext.get()), &asyncContext->work);
        if (status != napi_ok) {
            napi_get_undefined(env, &result);
        } else {
            napi_queue_async_work(env, asyncContext->work);
            asyncContext.release();
        }
    }

    return result;
}

napi_value MediaAssetNapi::StartModify(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value result = nullptr;
    const int32_t refCount = 1;
    napi_value resource = nullptr;
    size_t argc = ARGS_ONE;
    napi_value argv[ARGS_ONE] = {0};
    napi_value thisVar = nullptr;

    GET_JS_ARGS(env, info, argc, argv, thisVar);
    NAPI_ASSERT(env, argc <= 1, "requires 1 parameter maximum");

    napi_get_undefined(env, &result);
    std::unique_ptr<MediaAssetAsyncContext> asyncContext = std::make_unique<MediaAssetAsyncContext>();
    status = napi_unwrap(env, thisVar, reinterpret_cast<void**>(&asyncContext->objectInfo));
    if (status == napi_ok && asyncContext->objectInfo != nullptr) {
        if (argc == ARGS_ONE) {
            GET_JS_ASYNC_CB_REF(env, argv[PARAM0], refCount, asyncContext->callbackRef);
        }

        NAPI_CREATE_PROMISE(env, asyncContext->callbackRef, asyncContext->deferred, result);
        NAPI_CREATE_RESOURCE_NAME(env, resource, "StartModify");

        status = napi_create_async_work(
            env, nullptr, resource, [](napi_env env, void* data) {
                auto context = static_cast<MediaAssetAsyncContext*>(data);
                context->objectInfo->startModifyFlag = true;
                context->status = true;
            },
            CommonCompleteCallback, static_cast<void*>(asyncContext.get()), &asyncContext->work);
        if (status != napi_ok) {
            napi_get_undefined(env, &result);
        } else {
            napi_queue_async_work(env, asyncContext->work);
            asyncContext.release();
        }
    }

    return result;
}

napi_value MediaAssetNapi::CommitModify(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value result = nullptr;
    napi_value resource = nullptr;
    size_t argc = ARGS_ONE;
    napi_value argv[ARGS_ONE] = {0};
    napi_value thisVar = nullptr;

    GET_JS_ARGS(env, info, argc, argv, thisVar);
    NAPI_ASSERT(env, argc <= ARGS_ONE, "requires 1 parameter maximum");

    napi_get_undefined(env, &result);
    std::unique_ptr<MediaAssetAsyncContext> asyncContext = std::make_unique<MediaAssetAsyncContext>();
    status = napi_unwrap(env, thisVar, reinterpret_cast<void**>(&asyncContext->objectInfo));
    if (status == napi_ok && asyncContext->objectInfo != nullptr) {
        if (argc == ARGS_ONE) {
            GET_JS_ASYNC_CB_REF(env, argv[PARAM0], REFERENCE_COUNT_ONE, asyncContext->callbackRef);
        }

        NAPI_CREATE_PROMISE(env, asyncContext->callbackRef, asyncContext->deferred, result);
        NAPI_CREATE_RESOURCE_NAME(env, resource, "CommitModify");

        status = napi_create_async_work(env, nullptr, resource, [](napi_env env, void* data) {
            auto context = static_cast<MediaAssetAsyncContext*>(data);
            Media::MediaAsset assetOld, assetNew;
            std::string newName = context->objectInfo->newName_;

            context->status = false;
            context->objectInfo->UpdateNativeMediaAsset(assetOld);
            context->objectInfo->UpdateNativeMediaAsset(assetNew);

            if ((!newName.empty()) && (newName.compare(context->objectInfo->name_) != 0)) {
                assetNew.name_ = newName;

                context->status = context->objectInfo->mediaLibrary_->ModifyMediaAsset(
                    GetAssetType(assetOld.mediaType_), assetOld, assetNew);
                if (context->status) {
                    context->objectInfo->name_ = assetNew.name_;
                    context->objectInfo->uri_ = assetNew.uri_;
                }
                context->objectInfo->newName_ = "";
            } else {
                HiLog::Error(LABEL, "Incorrect or no modification values provided");
            }
            context->objectInfo->startModifyFlag = false;
        }, CommonCompleteCallback, static_cast<void*>(asyncContext.get()), &asyncContext->work);
        if (status != napi_ok) {
            napi_get_undefined(env, &result);
        } else {
            napi_queue_async_work(env, asyncContext->work);
            asyncContext.release();
        }
    }

    return result;
}

napi_value MediaAssetNapi::CancelModify(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value result = nullptr;
    const int32_t refCount = 1;
    napi_value resource = nullptr;
    size_t argc = ARGS_ONE;
    napi_value argv[ARGS_ONE] = {0};
    napi_value thisVar = nullptr;

    GET_JS_ARGS(env, info, argc, argv, thisVar);
    NAPI_ASSERT(env, argc <= 1, "requires 1 parameter maximum");

    napi_get_undefined(env, &result);
    std::unique_ptr<MediaAssetAsyncContext> asyncContext = std::make_unique<MediaAssetAsyncContext>();
    status = napi_unwrap(env, thisVar, reinterpret_cast<void**>(&asyncContext->objectInfo));
    if (status == napi_ok && asyncContext->objectInfo != nullptr) {
        if (argc == ARGS_ONE) {
            GET_JS_ASYNC_CB_REF(env, argv[PARAM0], refCount, asyncContext->callbackRef);
        }

        NAPI_CREATE_PROMISE(env, asyncContext->callbackRef, asyncContext->deferred, result);
        NAPI_CREATE_RESOURCE_NAME(env, resource, "CancelModify");

        status = napi_create_async_work(
            env, nullptr, resource, [](napi_env env, void* data) {
                auto context = static_cast<MediaAssetAsyncContext*>(data);
                if (!context->objectInfo->newName_.empty()) {
                    context->objectInfo->newName_ = "";
                }
                context->objectInfo->startModifyFlag = false;
                context->status = true;
            },
            CommonCompleteCallback, static_cast<void*>(asyncContext.get()), &asyncContext->work);
        if (status != napi_ok) {
            napi_get_undefined(env, &result);
        } else {
            napi_queue_async_work(env, asyncContext->work);
            asyncContext.release();
        }
    }

    return result;
}

napi_value MediaAssetNapi::CommitDelete(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value result = nullptr;
    const int32_t refCount = 1;
    napi_value resource = nullptr;
    size_t argc = ARGS_ONE;
    napi_value argv[ARGS_ONE] = {0};
    napi_value thisVar = nullptr;

    GET_JS_ARGS(env, info, argc, argv, thisVar);
    NAPI_ASSERT(env, argc <= 1, "requires 1 parameter maximum");

    napi_get_undefined(env, &result);
    std::unique_ptr<MediaAssetAsyncContext> asyncContext = std::make_unique<MediaAssetAsyncContext>();
    status = napi_unwrap(env, thisVar, reinterpret_cast<void**>(&asyncContext->objectInfo));
    if (status == napi_ok && asyncContext->objectInfo != nullptr) {
        if (argc == ARGS_ONE) {
            GET_JS_ASYNC_CB_REF(env, argv[PARAM0], refCount, asyncContext->callbackRef);
        }

        NAPI_CREATE_PROMISE(env, asyncContext->callbackRef, asyncContext->deferred, result);
        NAPI_CREATE_RESOURCE_NAME(env, resource, "CommitDelete");

        status = napi_create_async_work(
            env, nullptr, resource, [](napi_env env, void* data) {
                auto context = static_cast<MediaAssetAsyncContext*>(data);
                Media::MediaAsset asset;
                context->objectInfo->UpdateNativeMediaAsset(asset);
                context->status =
                    context->objectInfo->mediaLibrary_->DeleteMediaAsset(GetAssetType(asset.mediaType_), asset);
            },
            CommonCompleteCallback, static_cast<void*>(asyncContext.get()), &asyncContext->work);
        if (status != napi_ok) {
            napi_get_undefined(env, &result);
        } else {
            napi_queue_async_work(env, asyncContext->work);
            asyncContext.release();
        }
    }

    return result;
}

napi_value GetCommitCopyParams(napi_env env, size_t argCount, const napi_value argv[],
                               MediaAssetAsyncContext &context)
{
    const int32_t refCount = 1;
    napi_value result;
    auto asyncContext = &context;

    NAPI_ASSERT(env, argv != nullptr, "Argument list is empty");

    for (size_t i = PARAM0; i < argCount; i++) {
        napi_valuetype valueType = napi_undefined;
        napi_typeof(env, argv[i], &valueType);
        if ((i == PARAM0) && (valueType == napi_object)) {
            napi_unwrap(env, argv[i], reinterpret_cast<void**>(&asyncContext->targetCopyObject));
        } else if ((i == PARAM1) && (valueType == napi_function)) {
            napi_create_reference(env, argv[i], refCount, &asyncContext->callbackRef);
        } else {
            NAPI_ASSERT(env, false, "type mismatch");
        }
    }

    // Return true napi_value if params are successfully obtained
    napi_get_boolean(env, true, &result);
    return result;
}

napi_value MediaAssetNapi::CommitCopy(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value result = nullptr;
    napi_value resource = nullptr;
    size_t argc = ARGS_TWO;
    napi_value argv[ARGS_TWO] = {0};
    napi_value thisVar = nullptr;

    GET_JS_ARGS(env, info, argc, argv, thisVar);
    NAPI_ASSERT(env, argc >= ARGS_ONE, "requires 1 parameter minimum");

    status = napi_get_undefined(env, &result);
    std::unique_ptr<MediaAssetAsyncContext> asyncContext = std::make_unique<MediaAssetAsyncContext>();
    status = napi_unwrap(env, thisVar, reinterpret_cast<void**>(&asyncContext->objectInfo));
    if (status == napi_ok && asyncContext->objectInfo != nullptr) {
        result = GetCommitCopyParams(env, argc, argv, *asyncContext);
        if (result == nullptr) {
            napi_get_undefined(env, &result);
            return result;
        }

        NAPI_CREATE_PROMISE(env, asyncContext->callbackRef, asyncContext->deferred, result);
        NAPI_CREATE_RESOURCE_NAME(env, resource, "CommitCopy");

        status = napi_create_async_work(
            env, nullptr, resource, [](napi_env env, void* data) {
                auto context = static_cast<MediaAssetAsyncContext*>(data);
                Media::MediaAsset assetSrc, assetTarget;

                context->objectInfo->UpdateNativeMediaAsset(assetSrc);
                context->targetCopyObject->UpdateNativeMediaAsset(assetTarget);

                if (!context->targetCopyObject->newAlbumName_.empty()) {
                    assetTarget.albumName_ = context->targetCopyObject->newAlbumName_;

                    context->status = context->objectInfo->mediaLibrary_->CopyMediaAsset(
                        GetAssetType(assetSrc.mediaType_), assetSrc, assetTarget);
                    if (context->status) {
                        context->targetCopyObject->UpdateMediaAssetInfo(assetTarget);
                    }
                } else {
                    context->status = false;
                }
                context->targetCopyObject->newAlbumName_ = "";
                context->targetCopyObject->startCreateFlag = false;
            }, CommonCompleteCallback, static_cast<void*>(asyncContext.get()), &asyncContext->work);
        if (status != napi_ok) {
            napi_get_undefined(env, &result);
        } else {
            napi_queue_async_work(env, asyncContext->work);
            asyncContext.release();
        }
    }

    return result;
}
} // namespace OHOS
