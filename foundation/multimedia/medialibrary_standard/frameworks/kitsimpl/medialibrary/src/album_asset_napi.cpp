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

#include "album_asset_napi.h"
#include "hilog/log.h"

using OHOS::HiviewDFX::HiLog;
using OHOS::HiviewDFX::HiLogLabel;

namespace {
    constexpr HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "AlbumAssetNapi"};
    const int32_t DEFAULT_ALBUM_ID = 0;
    const std::string DEFAULT_ALBUM_NAME = "Unknown";
}

namespace OHOS {
napi_ref AlbumAssetNapi::sConstructor_ = nullptr;
Media::AlbumAsset *AlbumAssetNapi::sAlbumAsset_ = nullptr;
AlbumType AlbumAssetNapi::sAlbumType_ = TYPE_NONE;
std::string AlbumAssetNapi::sAlbumPath_ = "";
Media::IMediaLibraryClient *AlbumAssetNapi::sMediaLibrary_ = nullptr;

Media::AssetType GetAlbumType(AlbumType type)
{
    Media::AssetType result;

    switch (type) {
        case TYPE_VIDEO_ALBUM:
            result = Media::ASSET_VIDEOALBUM;
            break;
        case TYPE_IMAGE_ALBUM:
            result = Media::ASSET_IMAGEALBUM;
            break;
        case TYPE_NONE:
        default:
            result = Media::ASSET_GENERIC_ALBUM;
    }

    return result;
}

AlbumAssetNapi::AlbumAssetNapi()
    : mediaLibrary_(nullptr), env_(nullptr), wrapper_(nullptr)
{
    albumId_ = DEFAULT_ALBUM_ID;
    albumName_ = DEFAULT_ALBUM_NAME;
    type_ = TYPE_NONE;
}

AlbumAssetNapi::~AlbumAssetNapi()
{
    if (wrapper_ != nullptr) {
        napi_delete_reference(env_, wrapper_);
    }
}

void AlbumAssetNapi::AlbumAssetNapiDestructor(napi_env env, void *nativeObject, void *finalize_hint)
{
    AlbumAssetNapi *album = reinterpret_cast<AlbumAssetNapi*>(nativeObject);
    if (album != nullptr) {
        album->~AlbumAssetNapi();
    }
}

napi_value AlbumAssetNapi::Init(napi_env env, napi_value exports)
{
    napi_status status;
    napi_value ctorObj;
    int32_t refCount = 1;

    napi_property_descriptor album_asset_props[] = {
        DECLARE_NAPI_GETTER("albumId", GetAlbumId),
        DECLARE_NAPI_GETTER_SETTER("albumName", GetAlbumName, JSSetAlbumName),
        DECLARE_NAPI_FUNCTION("getVideoAssets", GetVideoAssets),
        DECLARE_NAPI_FUNCTION("getImageAssets", GetImageAssets),
        DECLARE_NAPI_FUNCTION("commitCreate", CommitCreate),
        DECLARE_NAPI_FUNCTION("commitDelete", CommitDelete),
        DECLARE_NAPI_FUNCTION("commitModify", CommitModify)
    };

    status = napi_define_class(env, ALBUM_ASSET_NAPI_CLASS_NAME.c_str(), NAPI_AUTO_LENGTH,
                               AlbumAssetNapiConstructor, nullptr,
                               sizeof(album_asset_props) / sizeof(album_asset_props[PARAM0]),
                               album_asset_props, &ctorObj);
    if (status == napi_ok) {
        status = napi_create_reference(env, ctorObj, refCount, &sConstructor_);
        if (status == napi_ok) {
            status = napi_set_named_property(env, exports, ALBUM_ASSET_NAPI_CLASS_NAME.c_str(), ctorObj);
            if (status == napi_ok) {
                return exports;
            }
        }
    }

    return nullptr;
}

// Constructor callback
napi_value AlbumAssetNapi::AlbumAssetNapiConstructor(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value result = nullptr;
    napi_value thisVar = nullptr;

    napi_get_undefined(env, &result);
    GET_JS_OBJ_WITH_ZERO_ARGS(env, info, status, thisVar);
    if (status == napi_ok && thisVar != nullptr) {
        std::unique_ptr<AlbumAssetNapi> obj = std::make_unique<AlbumAssetNapi>();
        if (obj == nullptr) {
            return result;
        }

        obj->env_ = env;
        obj->type_ = sAlbumType_;
        obj->albumPath_ = sAlbumPath_;
        obj->mediaLibrary_ = sMediaLibrary_;
        if (sAlbumAsset_ != nullptr) {
            obj->UpdateAlbumAssetInfo();

            for (size_t i = 0; i < sAlbumAsset_->videoAssetList_.size(); i++) {
                obj->videoAssets_.push_back(std::move(sAlbumAsset_->videoAssetList_[i]));
            }
            for (size_t i = 0; i < sAlbumAsset_->imageAssetList_.size(); i++) {
                obj->imageAssets_.push_back(std::move(sAlbumAsset_->imageAssetList_[i]));
            }
        } else {
            HiLog::Error(LABEL, "No native instance assigned yet");
            return result;
        }

        status = napi_wrap(env, thisVar, reinterpret_cast<void*>(obj.get()),
            AlbumAssetNapi::AlbumAssetNapiDestructor, nullptr, &(obj->wrapper_));
        if (status == napi_ok) {
            obj.release();
            return thisVar;
        } else {
            HiLog::Error(LABEL, "Failure wrapping js to native napi");
        }
    }

    return result;
}

napi_value AlbumAssetNapi::CreateAlbumAsset(napi_env env, AlbumType type,
                                            const std::string &albumParentPath,
                                            Media::AlbumAsset &aAsset,
                                            Media::IMediaLibraryClient &mediaLibClient)
{
    napi_status status;
    napi_value result = nullptr;
    napi_value constructor;

    status = napi_get_reference_value(env, sConstructor_, &constructor);
    if (status == napi_ok) {
        sAlbumAsset_ = &aAsset;
        sAlbumType_ = type;
        sAlbumPath_ = albumParentPath + "/" + sAlbumAsset_->albumName_;
        sMediaLibrary_ = &mediaLibClient;
        status = napi_new_instance(env, constructor, 0, nullptr, &result);
        sAlbumAsset_ = nullptr;
        if (status == napi_ok && result != nullptr) {
            return result;
        } else {
            HiLog::Error(LABEL, "Failed to create album asset instance");
        }
    }

    napi_get_undefined(env, &result);
    return result;
}

napi_value AlbumAssetNapi::GetAlbumId(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value jsResult = nullptr;
    napi_value undefinedResult = nullptr;
    AlbumAssetNapi* obj = nullptr;
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

napi_value AlbumAssetNapi::JSSetAlbumName(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value undefinedResult = nullptr;
    AlbumAssetNapi* obj = nullptr;
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
        if (napi_typeof(env, argv[PARAM0], &valueType) != napi_ok || valueType != napi_string) {
            HiLog::Error(LABEL, "Invalid arguments type!");
            return undefinedResult;
        }

        status = napi_get_value_string_utf8(env, argv[PARAM0], buffer, SIZE, &res);
        if (status == napi_ok) {
            obj->newAlbumName_ = buffer;
        }
    }

    return undefinedResult;
}

napi_value AlbumAssetNapi::GetAlbumName(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value jsResult = nullptr;
    napi_value undefinedResult = nullptr;
    AlbumAssetNapi* obj = nullptr;
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

Media::IMediaLibraryClient* AlbumAssetNapi::GetMediaLibClientInstance()
{
    Media::IMediaLibraryClient *ins = this->mediaLibrary_;
    return ins;
}

static void VideoAssetsAsyncCallbackComplete(napi_env env, napi_status status, void* data)
{
    auto context = static_cast<AlbumAsyncContext*>(data);
    napi_value result[ARGS_TWO] = {0};
    napi_value videoArray = nullptr;
    napi_value vAsset = nullptr;

    if (context == nullptr) {
        HiLog::Error(LABEL, "Async context is null");
        return;
    }

    napi_get_undefined(env, &result[PARAM0]);
    if (!context->videoAssets.empty() && (napi_create_array(env, &videoArray) == napi_ok)) {
        size_t len = context->videoAssets.size();
        size_t i;
        for (i = 0; i < len; i++) {
            vAsset = VideoAssetNapi::CreateVideoAsset(env, *(context->videoAssets[i]),
                *(context->objectInfo->GetMediaLibClientInstance()));
            if (vAsset == nullptr || napi_set_element(env, videoArray, i, vAsset) != napi_ok) {
                HiLog::Error(LABEL, "Failed to get video asset napi object");
                napi_get_undefined(env, &result[PARAM1]);
                break;
            }
        }
        if (i == len) {
            result[PARAM1] = videoArray;
        }
    } else {
        HiLog::Error(LABEL, "No video assets found!");
        napi_get_undefined(env, &result[PARAM1]);
    }

    if (context->work != nullptr) {
        MediaLibraryNapiUtils::InvokeJSAsyncMethod(env, context->deferred, result, ARGS_TWO,
                                                   context->callbackRef, context->work);
    }
    delete context;
}

napi_value AlbumAssetNapi::GetVideoAssets(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value result = nullptr;
    const int32_t refCount = 1;
    napi_value resource = nullptr;
    size_t argc = ARGS_ONE;
    napi_value argv[ARGS_ONE] = {0};
    napi_value thisVar = nullptr;

    GET_JS_ARGS(env, info, argc, argv, thisVar);
    NAPI_ASSERT(env, argc <= ARGS_ONE, "requires 1 parameter maximum");

    napi_get_undefined(env, &result);
    std::unique_ptr<AlbumAsyncContext> asyncContext = std::make_unique<AlbumAsyncContext>();
    status = napi_unwrap(env, thisVar, reinterpret_cast<void**>(&asyncContext->objectInfo));
    if (status == napi_ok && asyncContext->objectInfo != nullptr) {
        if (argc == ARGS_ONE) {
            GET_JS_ASYNC_CB_REF(env, argv[PARAM0], refCount, asyncContext->callbackRef);
        }

        NAPI_CREATE_PROMISE(env, asyncContext->callbackRef, asyncContext->deferred, result);
        NAPI_CREATE_RESOURCE_NAME(env, resource, "GetVideoAssets");

        status = napi_create_async_work(
            env, nullptr, resource,
            [](napi_env env, void* data) {
                AlbumAsyncContext* context = static_cast<AlbumAsyncContext*>(data);
                for (size_t i = 0; i < context->objectInfo->videoAssets_.size(); i += 1) {
                    context->videoAssets.push_back(std::move(context->objectInfo->videoAssets_[i]));
                }
            },
            VideoAssetsAsyncCallbackComplete, static_cast<void*>(asyncContext.get()), &asyncContext->work);
        if (status != napi_ok) {
            napi_get_undefined(env, &result);
        } else {
            napi_queue_async_work(env, asyncContext->work);
            asyncContext.release();
        }
    }

    return result;
}

static void ImageAssetsAsyncCallbackComplete(napi_env env, napi_status status, void* data)
{
    auto context = static_cast<AlbumAsyncContext*>(data);
    napi_value result[ARGS_TWO] = {0};
    napi_value imageArray = nullptr;
    napi_value iAsset = nullptr;

    if (context == nullptr) {
        HiLog::Error(LABEL, "Async context is null");
        return;
    }

    napi_get_undefined(env, &result[PARAM0]);
    if (!context->imageAssets.empty() && (napi_create_array(env, &imageArray) == napi_ok)) {
        size_t len = context->imageAssets.size();
        size_t i;
        for (i = 0; i < len; i++) {
            iAsset = ImageAssetNapi::CreateImageAsset(env, *(context->imageAssets[i]),
                *(context->objectInfo->GetMediaLibClientInstance()));
            if (iAsset == nullptr || napi_set_element(env, imageArray, i, iAsset) != napi_ok) {
                HiLog::Error(LABEL, "Failed to get image asset napi object");
                napi_get_undefined(env, &result[PARAM1]);
                break;
            }
        }
        if (i == len) {
            result[PARAM1] = imageArray;
        }
    } else {
        HiLog::Error(LABEL, "No image assets found!");
        napi_get_undefined(env, &result[PARAM1]);
    }

    if (context->work != nullptr) {
        MediaLibraryNapiUtils::InvokeJSAsyncMethod(env, context->deferred, result, ARGS_TWO,
                                                   context->callbackRef, context->work);
    }
    delete context;
}

napi_value AlbumAssetNapi::GetImageAssets(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value result = nullptr;
    const int32_t refCount = 1;
    napi_value resource = nullptr;
    size_t argc = ARGS_ONE;
    napi_value argv[ARGS_ONE] = {0};
    napi_value thisVar = nullptr;

    GET_JS_ARGS(env, info, argc, argv, thisVar);
    NAPI_ASSERT(env, argc <= ARGS_ONE, "requires 1 parameter maximum");

    napi_get_undefined(env, &result);
    std::unique_ptr<AlbumAsyncContext> asyncContext = std::make_unique<AlbumAsyncContext>();
    status = napi_unwrap(env, thisVar, reinterpret_cast<void**>(&asyncContext->objectInfo));
    if (status == napi_ok && asyncContext->objectInfo != nullptr) {
        if (argc == ARGS_ONE) {
            GET_JS_ASYNC_CB_REF(env, argv[PARAM0], refCount, asyncContext->callbackRef);
        }

        NAPI_CREATE_PROMISE(env, asyncContext->callbackRef, asyncContext->deferred, result);
        NAPI_CREATE_RESOURCE_NAME(env, resource, "GetImageAssets");

        status = napi_create_async_work(
            env, nullptr, resource,
            [](napi_env env, void* data) {
                AlbumAsyncContext* context = static_cast<AlbumAsyncContext*>(data);
                for (size_t i = 0; i < context->objectInfo->imageAssets_.size(); i += 1) {
                    context->imageAssets.push_back(std::move(context->objectInfo->imageAssets_[i]));
                }
            },
            ImageAssetsAsyncCallbackComplete, static_cast<void*>(asyncContext.get()), &asyncContext->work);
        if (status != napi_ok) {
            napi_get_undefined(env, &result);
        } else {
            napi_queue_async_work(env, asyncContext->work);
            asyncContext.release();
        }
    }

    return result;
}

void AlbumAssetNapi::UpdateAlbumAssetInfo()
{
    albumId_ = sAlbumAsset_->albumId_;
    albumName_ = sAlbumAsset_->albumName_;
}

static void CommonCompleteCallback(napi_env env, napi_status status, void* data)
{
    auto context = static_cast<AlbumAsyncContext*>(data);
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

napi_value AlbumAssetNapi::CommitCreate(napi_env env, napi_callback_info info)
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
    std::unique_ptr<AlbumAsyncContext> asyncContext = std::make_unique<AlbumAsyncContext>();
    status = napi_unwrap(env, thisVar, reinterpret_cast<void**>(&asyncContext->objectInfo));
    if (status == napi_ok && asyncContext->objectInfo != nullptr) {
        if (argc == ARGS_ONE) {
            GET_JS_ASYNC_CB_REF(env, argv[PARAM0], refCount, asyncContext->callbackRef);
        }

        NAPI_CREATE_PROMISE(env, asyncContext->callbackRef, asyncContext->deferred, result);
        NAPI_CREATE_RESOURCE_NAME(env, resource, "CommitCreate");

        status = napi_create_async_work(
            env, nullptr, resource, [](napi_env env, void* data) {
                auto context = static_cast<AlbumAsyncContext*>(data);
                Media::AlbumAsset asset;
                if (!context->objectInfo->newAlbumName_.empty()) {
                    asset.albumId_ = context->objectInfo->albumId_;
                    asset.albumName_ = context->objectInfo->newAlbumName_;
                    context->status = context->objectInfo->mediaLibrary_->CreateMediaAlbumAsset(
                        GetAlbumType(context->objectInfo->type_), asset);
                    if (context->status) {
                        context->objectInfo->albumName_ = context->objectInfo->newAlbumName_;
                    }
                } else {
                    context->status = false;
                }
                context->objectInfo->newAlbumName_ = "";
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

napi_value AlbumAssetNapi::CommitDelete(napi_env env, napi_callback_info info)
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
    std::unique_ptr<AlbumAsyncContext> asyncContext = std::make_unique<AlbumAsyncContext>();
    status = napi_unwrap(env, thisVar, reinterpret_cast<void**>(&asyncContext->objectInfo));
    if (status == napi_ok && asyncContext->objectInfo != nullptr) {
        if (argc == ARGS_ONE) {
            GET_JS_ASYNC_CB_REF(env, argv[PARAM0], refCount, asyncContext->callbackRef);
        }

        NAPI_CREATE_PROMISE(env, asyncContext->callbackRef, asyncContext->deferred, result);
        NAPI_CREATE_RESOURCE_NAME(env, resource, "CommitDelete");

        status = napi_create_async_work(
            env, nullptr, resource, [](napi_env env, void* data) {
                auto context = static_cast<AlbumAsyncContext*>(data);
                Media::AlbumAsset asset;
                std::string albumUri = context->objectInfo->albumPath_;

                asset.albumId_ = context->objectInfo->albumId_;
                asset.albumName_ = context->objectInfo->albumName_;

                context->status = context->objectInfo->mediaLibrary_->DeleteMediaAlbumAsset(
                    GetAlbumType(context->objectInfo->type_), asset, albumUri);
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

napi_value AlbumAssetNapi::CommitModify(napi_env env, napi_callback_info info)
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
    std::unique_ptr<AlbumAsyncContext> asyncContext = std::make_unique<AlbumAsyncContext>();
    status = napi_unwrap(env, thisVar, reinterpret_cast<void**>(&asyncContext->objectInfo));
    if (status == napi_ok && asyncContext->objectInfo != nullptr) {
        if (argc == ARGS_ONE) {
            GET_JS_ASYNC_CB_REF(env, argv[PARAM0], REFERENCE_COUNT_ONE, asyncContext->callbackRef);
        }

        NAPI_CREATE_PROMISE(env, asyncContext->callbackRef, asyncContext->deferred, result);
        NAPI_CREATE_RESOURCE_NAME(env, resource, "CommitModify");

        status = napi_create_async_work(env, nullptr, resource, [](napi_env env, void* data) {
            auto context = static_cast<AlbumAsyncContext*>(data);
            Media::AlbumAsset assetOld, assetNew;
            std::string albumUri = context->objectInfo->albumPath_;
            std::string albumNewName = context->objectInfo->newAlbumName_;

            context->status = false;
            assetOld.albumId_ = context->objectInfo->albumId_;
            assetOld.albumName_ = context->objectInfo->albumName_;

            assetNew.albumId_ = context->objectInfo->albumId_;
            if ((!albumNewName.empty()) && (albumNewName.compare(context->objectInfo->albumName_) != 0)) {
                assetNew.albumName_ = albumNewName;
                context->status = context->objectInfo->mediaLibrary_->ModifyMediaAlbumAsset(
                    GetAlbumType(context->objectInfo->type_), assetOld, assetNew, albumUri);
                if (context->status) {
                    context->objectInfo->albumName_ = assetNew.albumName_;
                }
                context->objectInfo->newAlbumName_ = "";
            } else {
                HiLog::Error(LABEL, "Incorrect or no modification values provided");
            }
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
