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

#include "media_library_napi.h"
#include <securec.h>
#include "hilog/log.h"

using OHOS::HiviewDFX::HiLog;
using OHOS::HiviewDFX::HiLogLabel;

namespace {
    constexpr HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "MediaLibraryNapi"};
}

namespace OHOS {
napi_ref MediaLibraryNapi::sConstructor_ = nullptr;

MediaLibraryNapi::MediaLibraryNapi()
    : mediaLibrary_(nullptr), env_(nullptr), wrapper_(nullptr) {}

MediaLibraryNapi::~MediaLibraryNapi()
{
    if (wrapper_ != nullptr) {
        napi_delete_reference(env_, wrapper_);
    }
}

void MediaLibraryNapi::MediaLibraryNapiDestructor(napi_env env, void *nativeObject, void *finalize_hint)
{
    MediaLibraryNapi *mediaLibrary = reinterpret_cast<MediaLibraryNapi*>(nativeObject);
    if (mediaLibrary != nullptr) {
        mediaLibrary->~MediaLibraryNapi();
    }
}

napi_value MediaLibraryNapi::Init(napi_env env, napi_value exports)
{
    napi_status status;
    napi_value ctorObj;
    int32_t refCount = 1;

    napi_property_descriptor media_library_properties[] = {
        DECLARE_NAPI_FUNCTION("getMediaAssets", GetMediaAssets),
        DECLARE_NAPI_FUNCTION("getAudioAssets", GetAudioAssets),
        DECLARE_NAPI_FUNCTION("getVideoAssets", GetVideoAssets),
        DECLARE_NAPI_FUNCTION("getImageAssets", GetImageAssets),
        DECLARE_NAPI_FUNCTION("getVideoAlbums", GetVideoAlbums),
        DECLARE_NAPI_FUNCTION("getImageAlbums", GetImageAlbums),
        DECLARE_NAPI_FUNCTION("createVideoAsset", CreateVideoAsset),
        DECLARE_NAPI_FUNCTION("createImageAsset", CreateImageAsset),
        DECLARE_NAPI_FUNCTION("createAudioAsset", CreateAudioAsset),
        DECLARE_NAPI_FUNCTION("createAlbum", CreateAlbum)
    };

    napi_property_descriptor static_prop[] = {
        DECLARE_NAPI_STATIC_FUNCTION("getMediaLibraryHelper", GetMediaLibraryInstance),
    };

    status = napi_define_class(env, MEDIA_LIBRARY_NAPI_CLASS_NAME.c_str(), NAPI_AUTO_LENGTH,
                               MediaLibraryNapiConstructor, nullptr,
                               sizeof(media_library_properties) / sizeof(media_library_properties[PARAM0]),
                               media_library_properties, &ctorObj);
    if (status == napi_ok) {
        status = napi_create_reference(env, ctorObj, refCount, &sConstructor_);
        if (status == napi_ok) {
            status = napi_set_named_property(env, exports, MEDIA_LIBRARY_NAPI_CLASS_NAME.c_str(), ctorObj);
            if (status == napi_ok) {
                status = napi_define_properties(env, exports,
                                                sizeof(static_prop) / sizeof(static_prop[PARAM0]), static_prop);
                if (status == napi_ok) {
                    return exports;
                }
            }
        }
    }

    return nullptr;
}

// Constructor callback
napi_value MediaLibraryNapi::MediaLibraryNapiConstructor(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value result = nullptr;
    napi_value thisVar = nullptr;

    napi_get_undefined(env, &result);
    GET_JS_OBJ_WITH_ZERO_ARGS(env, info, status, thisVar);
    if (status == napi_ok && thisVar != nullptr) {
        std::unique_ptr<MediaLibraryNapi> obj = std::make_unique<MediaLibraryNapi>();
        if (obj != nullptr) {
            obj->env_ = env;
            obj->mediaLibrary_ = Media::IMediaLibraryClient::GetMediaLibraryClientInstance();
            if (obj->mediaLibrary_ == nullptr) {
                HiLog::Error(LABEL, "MediaLibrary client instance creation failed!");
                return result;
            }
            status = napi_wrap(env, thisVar, reinterpret_cast<void*>(obj.get()),
                               MediaLibraryNapi::MediaLibraryNapiDestructor, nullptr, &(obj->wrapper_));
            if (status == napi_ok) {
                obj.release();
                return thisVar;
            } else {
                HiLog::Error(LABEL, "Failed to wrap the native media lib client object with JS");
            }
        }
    }

    return result;
}

napi_value MediaLibraryNapi::GetMediaLibraryInstance(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value result = nullptr;
    napi_value ctor;

    status = napi_get_reference_value(env, sConstructor_, &ctor);
    if (status == napi_ok) {
        status = napi_new_instance(env, ctor, 0, nullptr, &result);
        if (status == napi_ok) {
            return result;
        } else {
            HiLog::Error(LABEL, "New instance could not be obtained");
        }
    }

    napi_get_undefined(env, &result);
    return result;
}

Media::IMediaLibraryClient* MediaLibraryNapi::GetMediaLibClientInstance()
{
    Media::IMediaLibraryClient *ins = this->mediaLibrary_;
    return ins;
}

void GetFetchOptionsParam(napi_env env, napi_value arg, const std::string& str,
                          const std::vector<std::string>& strArr, bool &err)
{
    napi_value selection = nullptr;
    char buffer[SIZE];
    size_t res;
    std::string strItem;
    uint32_t len = 0;
    napi_value selectionArgs = nullptr;
    napi_value stringItem = nullptr;
    bool present = false;
    auto selectionStr = const_cast<std::string *>(&str);
    auto selectionStrArray = const_cast<std::vector<std::string> *>(&strArr);

    if (napi_get_named_property(env, arg, "selections", &selection) != napi_ok
        || napi_get_value_string_utf8(env, selection, buffer, SIZE, &res) != napi_ok) {
        HiLog::Error(LABEL, "Could not get the string argument!");
        err = true;
        return;
    } else {
        *selectionStr = buffer;
        memset_s(buffer, SIZE, 0, sizeof(buffer));
    }

    napi_has_named_property(env, arg, "selectionArgs", &present);
    if (present && napi_get_named_property(env, arg, "selectionArgs", &selectionArgs) == napi_ok) {
        napi_get_array_length(env, selectionArgs, &len);
        for (size_t i = 0; i < len; i++) {
            napi_get_element(env, selectionArgs, i, &stringItem);
            napi_get_value_string_utf8(env, stringItem, buffer, SIZE, &res);
            strItem = buffer;
            selectionStrArray->push_back(strItem);
            memset_s(buffer, SIZE, 0, sizeof(buffer));
        }
    } else {
        HiLog::Error(LABEL, "Could not get the string argument!");
        err = true;
    }
}

napi_value ConvertJSArgsToNative(napi_env env, size_t argc, const napi_value argv[],
                                 MediaLibraryAsyncContext &asyncContext)
{
    std::string str = "";
    std::vector<std::string> strArr;
    bool err = false;
    const int32_t refCount = 1;
    napi_value result;
    auto context = &asyncContext;

    NAPI_ASSERT(env, argv != nullptr, "Argument list is empty");

    for (size_t i = PARAM0; i < argc; i++) {
        napi_valuetype valueType = napi_undefined;
        napi_typeof(env, argv[i], &valueType);

        if (i == PARAM0 && valueType == napi_object) {
            GetFetchOptionsParam(env, argv[PARAM0], str, strArr, err);
            if (!err) {
                context->selection = str;
                context->selectionArgs = strArr;
            } else {
                NAPI_ASSERT(env, false, "type mismatch");
            }
        } else if (i == PARAM0 && valueType == napi_function) {
            napi_create_reference(env, argv[i], refCount, &context->callbackRef);
            break;
        } else if (i == PARAM1 && valueType == napi_function) {
            napi_create_reference(env, argv[i], refCount, &context->callbackRef);
            break;
        } else {
            NAPI_ASSERT(env, false, "type mismatch");
        }
    }

    // Return true napi_value if params are successfully obtained
    napi_get_boolean(env, true, &result);
    return result;
}

static void MediaAssetsAsyncCallbackComplete(napi_env env, napi_status status, void* data)
{
    auto context = static_cast<MediaLibraryAsyncContext*>(data);
    napi_value result[ARGS_TWO] = {0};
    napi_value mediaArray = nullptr;
    napi_value mAsset = nullptr;

    if (context == nullptr) {
        HiLog::Error(LABEL, "Async context is null");
        return;
    }

    napi_get_undefined(env, &result[PARAM0]);
    if (!context->mediaAssets.empty()) {
        size_t len = context->mediaAssets.size();
        if (napi_create_array(env, &mediaArray) == napi_ok) {
            size_t i;
            for (i = 0; i < len; i++) {
                mAsset = MediaAssetNapi::CreateMediaAsset(env, *(context->mediaAssets[i]),
                    *(context->objectInfo->GetMediaLibClientInstance()));
                if (mAsset == nullptr || napi_set_element(env, mediaArray, i, mAsset) != napi_ok) {
                    HiLog::Error(LABEL, "Failed to get media asset napi object");
                    napi_get_undefined(env, &result[PARAM1]);
                    break;
                }
            }
            if (i == len) {
                result[PARAM1] = mediaArray;
            }
        } else {
            napi_get_undefined(env, &result[PARAM1]);
        }
    } else {
        HiLog::Error(LABEL, "No media assets found!");
        napi_get_undefined(env, &result[PARAM1]);
    }

    if (context->work != nullptr) {
        MediaLibraryNapiUtils::InvokeJSAsyncMethod(env, context->deferred, result, ARGS_TWO,
                                                   context->callbackRef, context->work);
    }
    delete context;
}

napi_value MediaLibraryNapi::GetMediaAssets(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value result = nullptr;
    napi_value resource = nullptr;
    size_t argc = ARGS_TWO;
    napi_value argv[ARGS_TWO] = {0};
    napi_value thisVar = nullptr;

    GET_JS_ARGS(env, info, argc, argv, thisVar);
    NAPI_ASSERT(env, argc <= ARGS_TWO, "requires 2 parameters maximum");

    napi_get_undefined(env, &result);
    std::unique_ptr<MediaLibraryAsyncContext> asyncContext = std::make_unique<MediaLibraryAsyncContext>();
    status = napi_unwrap(env, thisVar, reinterpret_cast<void**>(&asyncContext->objectInfo));
    if (status == napi_ok && asyncContext->objectInfo != nullptr) {
        result = ConvertJSArgsToNative(env, argc, argv, *asyncContext);
        if (result == nullptr) {
            return result;
        }

        NAPI_CREATE_PROMISE(env, asyncContext->callbackRef, asyncContext->deferred, result);
        NAPI_CREATE_RESOURCE_NAME(env, resource, "GetMediaAssets");

        status = napi_create_async_work(
            env, nullptr, resource,
            [](napi_env env, void* data) {
                MediaLibraryAsyncContext* context = static_cast<MediaLibraryAsyncContext*>(data);
                context->mediaAssets = context->objectInfo->mediaLibrary_->GetMediaAssets(context->selection,
                                                                                          context->selectionArgs);
                context->status = 0;
            },
            MediaAssetsAsyncCallbackComplete, static_cast<void*>(asyncContext.get()), &asyncContext->work);
        if (status != napi_ok) {
            napi_get_undefined(env, &result);
        } else {
            napi_queue_async_work(env, asyncContext->work);
            asyncContext.release();
        }
    }

    return result;
}

static void AudioAssetsAsyncCallbackComplete(napi_env env, napi_status status, void* data)
{
    auto context = static_cast<MediaLibraryAsyncContext*>(data);
    napi_value result[ARGS_TWO] = {0};
    napi_value audioArray = nullptr;
    napi_value aAsset = nullptr;

    if (context == nullptr) {
        HiLog::Error(LABEL, "Async context is null");
        return;
    }

    napi_get_undefined(env, &result[PARAM0]);
    if (!context->audioAssets.empty()) {
        size_t len = context->audioAssets.size();
        if (napi_create_array(env, &audioArray) == napi_ok) {
            size_t i = 0;
            for (; i < len; i++) {
                aAsset = AudioAssetNapi::CreateAudioAsset(env, *(context->audioAssets[i]),
                                                          *(context->objectInfo->GetMediaLibClientInstance()));
                if (aAsset == nullptr || napi_set_element(env, audioArray, i, aAsset) != napi_ok) {
                    HiLog::Error(LABEL, "Failed to get audio asset napi object");
                    napi_get_undefined(env, &result[PARAM1]);
                    break;
                }
            }
            if (i == len) {
                result[PARAM1] = audioArray;
            }
        } else {
            napi_get_undefined(env, &result[PARAM1]);
        }
    } else {
        HiLog::Error(LABEL, "No audio assets found!");
        napi_get_undefined(env, &result[PARAM1]);
    }

    if (context->work != nullptr) {
        MediaLibraryNapiUtils::InvokeJSAsyncMethod(env, context->deferred, result, ARGS_TWO,
                                                   context->callbackRef, context->work);
    }
    delete context;
}

napi_value MediaLibraryNapi::GetAudioAssets(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value result = nullptr;
    napi_value resource = nullptr;
    size_t argc = ARGS_TWO;
    napi_value argv[ARGS_TWO] = {0};
    napi_value thisVar = nullptr;

    GET_JS_ARGS(env, info, argc, argv, thisVar);
    NAPI_ASSERT(env, argc <= ARGS_TWO, "requires 2 parameters maximum");

    napi_get_undefined(env, &result);
    std::unique_ptr<MediaLibraryAsyncContext> asyncContext = std::make_unique<MediaLibraryAsyncContext>();
    status = napi_unwrap(env, thisVar, reinterpret_cast<void**>(&asyncContext->objectInfo));
    if (status == napi_ok && asyncContext->objectInfo != nullptr) {
        result = ConvertJSArgsToNative(env, argc, argv, *asyncContext);
        if (result == nullptr) {
            return result;
        }

        NAPI_CREATE_PROMISE(env, asyncContext->callbackRef, asyncContext->deferred, result);
        NAPI_CREATE_RESOURCE_NAME(env, resource, "GetAudioAssets");

        status = napi_create_async_work(
            env, nullptr, resource,
            [](napi_env env, void* data) {
                MediaLibraryAsyncContext* context = static_cast<MediaLibraryAsyncContext*>(data);
                context->audioAssets = context->objectInfo->mediaLibrary_->GetAudioAssets(context->selection,
                                                                                          context->selectionArgs);
                context->status = 0;
            },
            AudioAssetsAsyncCallbackComplete, static_cast<void*>(asyncContext.get()), &asyncContext->work);
        if (status != napi_ok) {
            napi_get_undefined(env, &result);
        } else {
            napi_queue_async_work(env, asyncContext->work);
            asyncContext.release();
        }
    }

    return result;
}

static void VideoAssetsAsyncCallbackComplete(napi_env env, napi_status status, void* data)
{
    auto context = static_cast<MediaLibraryAsyncContext*>(data);
    napi_value result[ARGS_TWO] = {0};
    napi_value videoArray = nullptr;
    napi_value vAsset = nullptr;

    if (context == nullptr) {
        HiLog::Error(LABEL, "Async context is null");
        return;
    }

    napi_get_undefined(env, &result[PARAM0]);
    if (!context->videoAssets.empty()) {
        size_t len = context->videoAssets.size();
        if (napi_create_array(env, &videoArray) == napi_ok) {
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
            napi_get_undefined(env, &result[PARAM1]);
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

napi_value MediaLibraryNapi::GetVideoAssets(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value result = nullptr;
    napi_value resource = nullptr;
    size_t argc = ARGS_TWO;
    napi_value argv[ARGS_TWO] = {0};
    napi_value thisVar = nullptr;

    GET_JS_ARGS(env, info, argc, argv, thisVar);
    NAPI_ASSERT(env, argc <= ARGS_TWO, "requires 2 parameters maximum");

    napi_get_undefined(env, &result);
    std::unique_ptr<MediaLibraryAsyncContext> asyncContext = std::make_unique<MediaLibraryAsyncContext>();
    status = napi_unwrap(env, thisVar, reinterpret_cast<void**>(&asyncContext->objectInfo));
    if (status == napi_ok && asyncContext->objectInfo != nullptr) {
        result = ConvertJSArgsToNative(env, argc, argv, *asyncContext);
        if (result == nullptr) {
            return result;
        }

        NAPI_CREATE_PROMISE(env, asyncContext->callbackRef, asyncContext->deferred, result);
        NAPI_CREATE_RESOURCE_NAME(env, resource, "GetVideoAssets");

        status = napi_create_async_work(
            env, nullptr, resource,
            [](napi_env env, void* data) {
                MediaLibraryAsyncContext* context = static_cast<MediaLibraryAsyncContext*>(data);
                context->videoAssets = context->objectInfo->mediaLibrary_->GetVideoAssets(context->selection,
                                                                                          context->selectionArgs);
                context->status = 0;
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
    auto context = static_cast<MediaLibraryAsyncContext*>(data);
    napi_value result[ARGS_TWO] = {0};
    napi_value imageArray = nullptr;
    napi_value iAsset = nullptr;

    if (context == nullptr) {
        HiLog::Error(LABEL, "Async context is null");
        return;
    }

    napi_get_undefined(env, &result[PARAM0]);
    if (!context->imageAssets.empty()) {
        size_t len = context->imageAssets.size();
        if (napi_create_array(env, &imageArray) == napi_ok) {
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
            napi_get_undefined(env, &result[PARAM1]);
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

napi_value MediaLibraryNapi::GetImageAssets(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value result = nullptr;
    napi_value resource = nullptr;
    size_t argc = ARGS_TWO;
    napi_value argv[ARGS_TWO] = {0};
    napi_value thisVar = nullptr;

    GET_JS_ARGS(env, info, argc, argv, thisVar);
    NAPI_ASSERT(env, argc <= ARGS_TWO, "requires 2 parameters maximum");

    napi_get_undefined(env, &result);
    std::unique_ptr<MediaLibraryAsyncContext> asyncContext = std::make_unique<MediaLibraryAsyncContext>();
    status = napi_unwrap(env, thisVar, reinterpret_cast<void**>(&asyncContext->objectInfo));
    if (status == napi_ok && asyncContext->objectInfo != nullptr) {
        result = ConvertJSArgsToNative(env, argc, argv, *asyncContext);
        if (result == nullptr) {
            return result;
        }

        NAPI_CREATE_PROMISE(env, asyncContext->callbackRef, asyncContext->deferred, result);
        NAPI_CREATE_RESOURCE_NAME(env, resource, "GetImageAssets");

        status = napi_create_async_work(
            env, nullptr, resource,
            [](napi_env env, void* data) {
                MediaLibraryAsyncContext* context = static_cast<MediaLibraryAsyncContext*>(data);
                context->imageAssets = context->objectInfo->mediaLibrary_->GetImageAssets(context->selection,
                                                                                          context->selectionArgs);
                context->status = 0;
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

static void AlbumAssetsAsyncCallbackComplete(napi_env env, napi_status status, void* data)
{
    auto context = static_cast<MediaLibraryAsyncContext*>(data);
    napi_value result[ARGS_TWO] = {0};
    napi_value albumArray = nullptr;
    napi_value albumAsset = nullptr;

    if (context == nullptr) {
        HiLog::Error(LABEL, "Async context is null");
        return;
    }

    napi_get_undefined(env, &result[PARAM0]);
    if (!context->albumAssets.empty() && (napi_create_array(env, &albumArray) == napi_ok)) {
        size_t len = context->albumAssets.size();
        size_t i;
        for (i = 0; i < len; i++) {
            std::string path = ALBUM_ROOT_PATH;
            if (!context->selection.empty()) {
                path = ALBUM_ROOT_PATH + "/" + context->selection;
            }
            albumAsset = AlbumAssetNapi::CreateAlbumAsset(env, context->albumType, path,
                *(context->albumAssets[i]), *(context->objectInfo->GetMediaLibClientInstance()));
            if (albumAsset == nullptr || napi_set_element(env, albumArray, i, albumAsset) != napi_ok) {
                HiLog::Error(LABEL, "Failed to get album asset napi object");
                napi_get_undefined(env, &result[PARAM1]);
                break;
            }
        }
        if (i == len) {
            result[PARAM1] = albumArray;
        }
    } else {
        HiLog::Error(LABEL, "No album assets found!");
        napi_get_undefined(env, &result[PARAM1]);
    }

    if (context->work != nullptr) {
        MediaLibraryNapiUtils::InvokeJSAsyncMethod(env, context->deferred, result, ARGS_TWO,
                                                   context->callbackRef, context->work);
    }
    delete context;
}

napi_value MediaLibraryNapi::GetVideoAlbums(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value result = nullptr;
    napi_value resource = nullptr;
    size_t argc = ARGS_TWO;
    napi_value argv[ARGS_TWO] = {0};
    napi_value thisVar = nullptr;

    GET_JS_ARGS(env, info, argc, argv, thisVar);
    NAPI_ASSERT(env, argc >= ARGS_ONE, "requires 1 parameter minimum");

    napi_get_undefined(env, &result);
    std::unique_ptr<MediaLibraryAsyncContext> asyncContext = std::make_unique<MediaLibraryAsyncContext>();
    status = napi_unwrap(env, thisVar, reinterpret_cast<void**>(&asyncContext->objectInfo));
    if (status == napi_ok && asyncContext->objectInfo != nullptr) {
        result = ConvertJSArgsToNative(env, argc, argv, *asyncContext);
        if (result == nullptr) {
            return result;
        }

        NAPI_CREATE_PROMISE(env, asyncContext->callbackRef, asyncContext->deferred, result);
        NAPI_CREATE_RESOURCE_NAME(env, resource, "GetVideoAlbums");

        status = napi_create_async_work(
            env, nullptr, resource,
            [](napi_env env, void* data) {
                MediaLibraryAsyncContext* context = static_cast<MediaLibraryAsyncContext*>(data);
                context->albumAssets =
                    context->objectInfo->mediaLibrary_->GetVideoAlbumAssets(context->selection,
                                                                            context->selectionArgs);
                context->albumType = TYPE_VIDEO_ALBUM;
                context->status = 0;
            },
            AlbumAssetsAsyncCallbackComplete, static_cast<void*>(asyncContext.get()), &asyncContext->work);
        if (status != napi_ok) {
            napi_get_undefined(env, &result);
        } else {
            napi_queue_async_work(env, asyncContext->work);
            asyncContext.release();
        }
    }

    return result;
}

napi_value MediaLibraryNapi::GetImageAlbums(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value result = nullptr;
    napi_value resource = nullptr;
    size_t argc = ARGS_TWO;
    napi_value argv[ARGS_TWO] = {0};
    napi_value thisVar = nullptr;

    GET_JS_ARGS(env, info, argc, argv, thisVar);
    NAPI_ASSERT(env, argc >= ARGS_ONE, "requires 1 parameter minimum");

    napi_get_undefined(env, &result);
    std::unique_ptr<MediaLibraryAsyncContext> asyncContext = std::make_unique<MediaLibraryAsyncContext>();
    status = napi_unwrap(env, thisVar, reinterpret_cast<void**>(&asyncContext->objectInfo));
    if (status == napi_ok && asyncContext->objectInfo != nullptr) {
        result = ConvertJSArgsToNative(env, argc, argv, *asyncContext);
        if (result == nullptr) {
            return result;
        }

        NAPI_CREATE_PROMISE(env, asyncContext->callbackRef, asyncContext->deferred, result);
        NAPI_CREATE_RESOURCE_NAME(env, resource, "GetImageAlbums");

        status = napi_create_async_work(
            env, nullptr, resource,
            [](napi_env env, void* data) {
                MediaLibraryAsyncContext* context = static_cast<MediaLibraryAsyncContext*>(data);
                context->albumAssets =
                    context->objectInfo->mediaLibrary_->GetImageAlbumAssets(context->selection,
                                                                            context->selectionArgs);
                context->albumType = TYPE_IMAGE_ALBUM;
                context->status = 0;
            },
            AlbumAssetsAsyncCallbackComplete, static_cast<void*>(asyncContext.get()), &asyncContext->work);
        if (status != napi_ok) {
            napi_get_undefined(env, &result);
        } else {
            napi_queue_async_work(env, asyncContext->work);
            asyncContext.release();
        }
    }

    return result;
}

napi_value GetAssetJSObject(napi_env env, AssetType type, Media::IMediaLibraryClient &mediaLibrary)
{
    napi_value assetObj = nullptr;

    switch (type) {
        case TYPE_AUDIO: {
            std::unique_ptr<Media::AudioAsset> audioObj = std::make_unique<Media::AudioAsset>();
            assetObj = AudioAssetNapi::CreateAudioAsset(env, *(audioObj), mediaLibrary);
            break;
        }
        case TYPE_VIDEO: {
            std::unique_ptr<Media::VideoAsset> videoObj = std::make_unique<Media::VideoAsset>();
            assetObj = VideoAssetNapi::CreateVideoAsset(env, *(videoObj), mediaLibrary);
            break;
        }
        case TYPE_IMAGE: {
            std::unique_ptr<Media::ImageAsset> imageObj = std::make_unique<Media::ImageAsset>();
            assetObj = ImageAssetNapi::CreateImageAsset(env, *(imageObj), mediaLibrary);
            break;
        }
        case TYPE_ALBUM: {
            std::unique_ptr<Media::AlbumAsset> albumObj = std::make_unique<Media::AlbumAsset>();
            assetObj = AlbumAssetNapi::CreateAlbumAsset(env, TYPE_NONE, "", *(albumObj), mediaLibrary);
            break;
        }
        default:
            HiLog::Error(LABEL, "Wrong media type");
            break;
    }

    if (assetObj == nullptr) {
        HiLog::Error(LABEL, "No assets obtained");
        napi_get_undefined(env, &assetObj);
    }

    return assetObj;
}

void CreateAssetAsyncCbComplete(napi_env env, napi_status status, void* data)
{
    auto context = static_cast<MediaLibraryAsyncContext*>(data);
    napi_value result[ARGS_TWO] = {0};

    if (context != nullptr) {
        napi_get_undefined(env, &result[PARAM0]);
        result[PARAM1] = GetAssetJSObject(env, context->assetType,
                                          *(context->objectInfo->GetMediaLibClientInstance()));

        if (context->work != nullptr) {
            MediaLibraryNapiUtils::InvokeJSAsyncMethod(env, context->deferred, result, ARGS_TWO,
                                                       context->callbackRef, context->work);
        }
        delete context;
    } else {
        HiLog::Error(LABEL, "Async context is null");
    }
}

void CreateAsyncWork(napi_env env, napi_value resource,
                     const MediaLibraryAsyncContext& mediaLibContext,
                     AssetType type, bool &err)
{
    napi_status status;
    MediaLibraryAsyncContext* asyncContext = const_cast<MediaLibraryAsyncContext *>(&mediaLibContext);
    asyncContext->assetType = type;

    status = napi_create_async_work(
        env, nullptr, resource,
        [](napi_env env, void* data) {},
        CreateAssetAsyncCbComplete, (void*)asyncContext, &asyncContext->work);
    if (status != napi_ok) {
        err = true;
    } else {
        napi_queue_async_work(env, asyncContext->work);
    }
}

napi_value MediaLibraryNapi::CreateAudioAsset(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value result = nullptr;
    bool err = false;
    napi_value resource = nullptr;
    size_t argc = ARGS_ONE;
    napi_value argv[ARGS_ONE] = {0};
    napi_value thisVar = nullptr;

    GET_JS_ARGS(env, info, argc, argv, thisVar);
    NAPI_ASSERT(env, argc <= ARGS_ONE, "requires 1 parameter maximum");

    napi_get_undefined(env, &result);
    std::unique_ptr<MediaLibraryAsyncContext> asyncContext = std::make_unique<MediaLibraryAsyncContext>();
    status = napi_unwrap(env, thisVar, reinterpret_cast<void**>(&asyncContext->objectInfo));
    if (status == napi_ok && asyncContext->objectInfo != nullptr) {
        result = ConvertJSArgsToNative(env, argc, argv, *asyncContext);
        ASSERT_NULLPTR_CHECK(env, result);

        NAPI_CREATE_PROMISE(env, asyncContext->callbackRef, asyncContext->deferred, result);
        NAPI_CREATE_RESOURCE_NAME(env, resource, "CreateAudioAsset");

        CreateAsyncWork(env, resource, *(asyncContext.get()), TYPE_AUDIO, err);
        if (!err) {
            asyncContext.release();
        } else {
            napi_get_undefined(env, &result);
        }
    }

    return result;
}

napi_value MediaLibraryNapi::CreateVideoAsset(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value result = nullptr;
    bool err = false;
    napi_value resource = nullptr;
    size_t argc = ARGS_ONE;
    napi_value argv[ARGS_ONE] = {0};
    napi_value thisVar = nullptr;

    GET_JS_ARGS(env, info, argc, argv, thisVar);
    NAPI_ASSERT(env, argc <= ARGS_ONE, "requires 1 parameter maximum");

    napi_get_undefined(env, &result);
    std::unique_ptr<MediaLibraryAsyncContext> asyncContext = std::make_unique<MediaLibraryAsyncContext>();
    status = napi_unwrap(env, thisVar, reinterpret_cast<void**>(&asyncContext->objectInfo));
    if (status == napi_ok && asyncContext->objectInfo != nullptr) {
        result = ConvertJSArgsToNative(env, argc, argv, *asyncContext);
        ASSERT_NULLPTR_CHECK(env, result);

        NAPI_CREATE_PROMISE(env, asyncContext->callbackRef, asyncContext->deferred, result);
        NAPI_CREATE_RESOURCE_NAME(env, resource, "CreateVideoAsset");

        CreateAsyncWork(env, resource, *(asyncContext.get()), TYPE_VIDEO, err);
        if (!err) {
            asyncContext.release();
        } else {
            napi_get_undefined(env, &result);
        }
    }

    return result;
}

napi_value MediaLibraryNapi::CreateImageAsset(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value result = nullptr;
    bool err = false;
    napi_value resource = nullptr;
    size_t argc = ARGS_ONE;
    napi_value argv[ARGS_ONE] = {0};
    napi_value thisVar = nullptr;

    GET_JS_ARGS(env, info, argc, argv, thisVar);
    NAPI_ASSERT(env, argc <= ARGS_ONE, "requires 1 parameter maximum");

    napi_get_undefined(env, &result);
    std::unique_ptr<MediaLibraryAsyncContext> asyncContext = std::make_unique<MediaLibraryAsyncContext>();
    status = napi_unwrap(env, thisVar, reinterpret_cast<void**>(&asyncContext->objectInfo));
    if (status == napi_ok && asyncContext->objectInfo != nullptr) {
        result = ConvertJSArgsToNative(env, argc, argv, *asyncContext);
        ASSERT_NULLPTR_CHECK(env, result);

        NAPI_CREATE_PROMISE(env, asyncContext->callbackRef, asyncContext->deferred, result);
        NAPI_CREATE_RESOURCE_NAME(env, resource, "CreateImageAsset");

        CreateAsyncWork(env, resource, *(asyncContext.get()), TYPE_IMAGE, err);
        if (!err) {
            asyncContext.release();
        } else {
            napi_get_undefined(env, &result);
        }
    }

    return result;
}

napi_value MediaLibraryNapi::CreateAlbum(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value result = nullptr;
    bool err = false;
    napi_value resource = nullptr;
    size_t argc = ARGS_ONE;
    napi_value argv[ARGS_ONE] = {0};
    napi_value thisVar = nullptr;

    GET_JS_ARGS(env, info, argc, argv, thisVar);
    NAPI_ASSERT(env, argc <= ARGS_ONE, "requires 1 parameter maximum");

    napi_get_undefined(env, &result);
    std::unique_ptr<MediaLibraryAsyncContext> asyncContext = std::make_unique<MediaLibraryAsyncContext>();
    status = napi_unwrap(env, thisVar, reinterpret_cast<void**>(&asyncContext->objectInfo));
    if (status == napi_ok && asyncContext->objectInfo != nullptr) {
        result = ConvertJSArgsToNative(env, argc, argv, *asyncContext);
        ASSERT_NULLPTR_CHECK(env, result);

        NAPI_CREATE_PROMISE(env, asyncContext->callbackRef, asyncContext->deferred, result);
        NAPI_CREATE_RESOURCE_NAME(env, resource, "CreateAlbum");

        CreateAsyncWork(env, resource, *(asyncContext.get()), TYPE_ALBUM, err);
        if (!err) {
            asyncContext.release();
        } else {
            napi_get_undefined(env, &result);
        }
    }

    return result;
}
} // namespace OHOS
