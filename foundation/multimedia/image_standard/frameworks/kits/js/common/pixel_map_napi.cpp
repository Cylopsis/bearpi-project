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


#include "pixel_map_napi.h"
#include "media_errors.h"
#include "hilog/log.h"
#include "image_napi_utils.h"

using OHOS::HiviewDFX::HiLog;
namespace {
    constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "PixelMapNapi"};
}

namespace OHOS {
namespace Media {

static const std::string CLASS_NAME = "PixelMap";
napi_ref PixelMapNapi::sConstructor_ = nullptr;
std::shared_ptr<PixelMap> PixelMapNapi::sPixelMap_ = nullptr;

struct PositionArea {
    void* pixels;
    size_t size;
    uint32_t offset;
    uint32_t stride;
    Rect region;
};

struct PixelMapAsyncContext {
    napi_env env;
    napi_async_work work;
    napi_deferred deferred;
    napi_ref callbackRef;
    uint32_t status;
    PixelMapNapi *nConstructor;
    void* colorsBuffer;
    size_t colorsBufferSize;
    InitializationOptions opts;
    PositionArea area;
    std::shared_ptr<PixelMap> rPixelMap;
    uint32_t resultUint32;
};

static PixelFormat ParsePixlForamt(int32_t val)
{
    if(val <= static_cast<int32_t>(PixelFormat::CMYK))
    {
        return PixelFormat(val);
    }

    return PixelFormat::UNKNOWN;
}

static AlphaType ParseAlphaType(int32_t val)
{
    if(val <= static_cast<int32_t>(AlphaType::IMAGE_ALPHA_TYPE_UNPREMUL))
    {
        return AlphaType(val);
    }

    return AlphaType::IMAGE_ALPHA_TYPE_UNKNOWN;

}

static ScaleMode ParseScaleMode(int32_t val)
{
    if(val <= static_cast<int32_t>(ScaleMode::CENTER_CROP))
    {
        return ScaleMode(val);
    }

    return ScaleMode::FIT_TARGET_SIZE;
}

static bool parseSize(napi_env env, napi_value root, Size* size)
{
    if(!GET_INT32_BY_NAME(root, "height", size->height))
    {
        return false;
    }

    if(!GET_INT32_BY_NAME(root, "width", size->width))
    {
        return false;
    }

    return true;
}

static bool parseInitializationOptions(napi_env env, napi_value root, InitializationOptions* opts)
{
    uint32_t tmpNumber = 0;
    napi_value tmpValue = nullptr;

    if(!GET_BOOL_BY_NAME(root, "editable", opts->editable))
    {
        return false;
    }

    if(!GET_UINT32_BY_NAME(root, "alphaType", tmpNumber))
    {
        return false;
    }
    opts->alphaType = ParseAlphaType(tmpNumber);

    tmpNumber = 0;
    if(!GET_UINT32_BY_NAME(root, "pixelFormat", tmpNumber))
    {
        return false;
    }
    opts->pixelFormat = ParsePixlForamt(tmpNumber);
    

    tmpNumber = 0;
    if(!GET_UINT32_BY_NAME(root, "scaleMode", tmpNumber))
    {
        return false;
    }
    opts->scaleMode = ParseScaleMode(tmpNumber);
    
    if(!GET_NODE_BY_NAME(root, "size", tmpValue))
    {
        return false;
    }
    
    if(!parseSize(env, tmpValue, &(opts->size)))
    {
        return false;
    }
    return true;
}

static bool parseRegion(napi_env env, napi_value root, Rect* region)
{
    napi_value tmpValue = nullptr;

    if(!GET_INT32_BY_NAME(root, "x", region->left))
    {
        return false;
    }

    if(!GET_INT32_BY_NAME(root, "y", region->top))
    {
        return false;
    }

    if(!GET_NODE_BY_NAME(root, "size", tmpValue))
    {
        return false;
    }

    if(!GET_INT32_BY_NAME(tmpValue, "height", region->height))
    {
        return false;
    }

    if(!GET_INT32_BY_NAME(tmpValue, "width", region->width))
    {
        return false;
    }

    return true;
}

static bool parsePositionArea(napi_env env, napi_value root, PositionArea* area)
{
    napi_value tmpValue = nullptr;

    if(!GET_BUFFER_BY_NAME(root, "pixels", area->pixels, area->size))
    {
        return false;
    }

    if(!GET_UINT32_BY_NAME(root, "offset", area->offset))
    {
        return false;
    }

    if(!GET_UINT32_BY_NAME(root, "stride", area->stride))
    {
        return false;
    }
    
    if(!GET_NODE_BY_NAME(root, "region", tmpValue))
    {
        return false;
    }
    
    if(!parseRegion(env, tmpValue, &(area->region)))
    {
        return false;
    }
    return true;
}

static void CommonCallbackRoutine(napi_env env, PixelMapAsyncContext* &asyncContext, const napi_value &valueParam)
{
    napi_value result[2] = {0};
    napi_value retVal;
    napi_value callback = nullptr;

    napi_get_undefined(env, &result[0]);
    napi_get_undefined(env, &result[1]);

    if (asyncContext->status == SUCCESS) {
        result[1] = valueParam;
    }

    if (asyncContext->deferred) {
        if (asyncContext->status == SUCCESS) {
            napi_resolve_deferred(env, asyncContext->deferred, result[1]);
        } else {
            napi_reject_deferred(env, asyncContext->deferred, result[0]);
        }
    } else {
        napi_get_reference_value(env, asyncContext->callbackRef, &callback);
        napi_call_function(env, nullptr, callback, 2, result, &retVal);
        napi_delete_reference(env, asyncContext->callbackRef);
    }

    napi_delete_async_work(env, asyncContext->work);

    delete asyncContext;
    asyncContext = nullptr;
}

STATIC_COMPLETE_FUNC(EmptyResult)
{
    napi_value result = nullptr;
    napi_get_undefined(env, &result);

    auto context = static_cast<PixelMapAsyncContext*>(data);

    CommonCallbackRoutine(env, context, result);
}


STATIC_COMPLETE_FUNC(Uint32Result)
{
    napi_value result = nullptr;
    napi_get_undefined(env, &result);

    auto context = static_cast<PixelMapAsyncContext*>(data);

    status = napi_create_int32(env, context->resultUint32, &result);

    if(!IMG_IS_OK(status))
    {
        context->status = ERROR;
        HiLog::Error(LABEL, "napi_create_int32 failed!");
        napi_get_undefined(env, &result);
    }
    else
    {
        context->status = SUCCESS;
    }

    CommonCallbackRoutine(env, context, result);
}

PixelMapNapi::PixelMapNapi()
    :env_(nullptr), wrapper_(nullptr)
{

}

PixelMapNapi::~PixelMapNapi()
{
    if(nativePixelMap_ != nullptr)
    {
        nativePixelMap_ = nullptr;
    }

    if (wrapper_ != nullptr)
    {
        napi_delete_reference(env_, wrapper_);
    }
}

napi_value PixelMapNapi::Init(napi_env env, napi_value exports)
{
    napi_property_descriptor props[] = {
        DECLARE_NAPI_FUNCTION("readPixelsToBuffer", ReadPixelsToBuffer),
        DECLARE_NAPI_FUNCTION("readPixels", ReadPixels),
        DECLARE_NAPI_FUNCTION("writePixels", WritePixels),
        DECLARE_NAPI_FUNCTION("writeBufferToPixels", WriteBufferToPixels),
        DECLARE_NAPI_FUNCTION("getImageInfo", GetImageInfo),
        DECLARE_NAPI_FUNCTION("getBytesNumberPerRow", GetBytesNumberPerRow),
        DECLARE_NAPI_FUNCTION("getPixelBytesNumber", GetPixelBytesNumber),
        DECLARE_NAPI_FUNCTION("Release", Release),

        DECLARE_NAPI_GETTER("isEditable", GetIsEditable),
    };

    napi_property_descriptor static_prop[] = {
        DECLARE_NAPI_STATIC_FUNCTION("createPixelMap", CreatePixelMap),
    };

    napi_value constructor = nullptr;

    IMG_NAPI_CHECK_RET_D(IMG_IS_OK(
        napi_define_class(env, CLASS_NAME.c_str(), NAPI_AUTO_LENGTH,
                            Constructor, nullptr, IMG_ARRAY_SIZE(props),
                            props, &constructor)),
        nullptr,
        HiLog::Error(LABEL, "define class fail")
    );

    IMG_NAPI_CHECK_RET_D(IMG_IS_OK(
        napi_create_reference(env, constructor, 1, &sConstructor_)),
        nullptr,
        HiLog::Error(LABEL, "create reference fail")
    );

    IMG_NAPI_CHECK_RET_D(IMG_IS_OK(
        napi_set_named_property(env, exports, CLASS_NAME.c_str(), constructor)),
        nullptr,
        HiLog::Error(LABEL, "set named property fail")
    );

    IMG_NAPI_CHECK_RET_D(IMG_IS_OK(
        napi_define_properties(env, exports, IMG_ARRAY_SIZE(static_prop), static_prop)),
        nullptr,
        HiLog::Error(LABEL, "define properties fail")
    );

    HiLog::Debug(LABEL, "Init success");
    return exports;
}

std::shared_ptr<PixelMap> PixelMapNapi::GetPixelMap(napi_env env, napi_value pixelmap)
{
    std::unique_ptr<PixelMapNapi> pixelMapNapi = std::make_unique<PixelMapNapi>();

    napi_status status = napi_unwrap(env, pixelmap, reinterpret_cast<void**>(&pixelMapNapi));

    if(IMG_IS_OK(status))
    {
        return pixelMapNapi->nativePixelMap_;
    }

    return nullptr;
}

napi_value PixelMapNapi::Constructor(napi_env env, napi_callback_info info)
{
    napi_value undefineVar = nullptr;
    napi_get_undefined(env, &undefineVar);

    napi_status status;
    napi_value thisVar = nullptr;
    napi_get_undefined(env, &thisVar);

    HiLog::Debug(LABEL, "Constructor IN");
    IMG_JS_NO_ARGS(env, info, status, thisVar);

    IMG_NAPI_CHECK_RET(IMG_IS_READY(status, thisVar), undefineVar);
    std::unique_ptr<PixelMapNapi> pPixelMapNapi = std::make_unique<PixelMapNapi>();

    IMG_NAPI_CHECK_RET(IMG_NOT_NULL(pPixelMapNapi), undefineVar);

    pPixelMapNapi->env_ = env;
    pPixelMapNapi->nativePixelMap_ = sPixelMap_;

    status = napi_wrap(env, thisVar, reinterpret_cast<void*>(pPixelMapNapi.get()),
                        PixelMapNapi::Destructor, nullptr, &(pPixelMapNapi->wrapper_));
    IMG_NAPI_CHECK_RET_D(IMG_IS_OK(status), undefineVar, HiLog::Error(LABEL, "Failure wrapping js to native napi"));

    pPixelMapNapi.release();
    sPixelMap_ = nullptr;

    return thisVar;
}

void PixelMapNapi::Destructor(napi_env env, void *nativeObject, void *finalize)
{
    PixelMapNapi *pixelMapNapi = reinterpret_cast<PixelMapNapi*>(nativeObject);

    if (IMG_NOT_NULL(pixelMapNapi))
    {
        pixelMapNapi->~PixelMapNapi();
    }
}

STATIC_EXEC_FUNC(CreatePixelMap)
{
    auto context = static_cast<PixelMapAsyncContext*>(data);
    auto colors = static_cast<uint32_t*>(context->colorsBuffer);
    auto pixelmap = PixelMap::Create(colors, context->colorsBufferSize, context->opts);

    context->rPixelMap = std::move(pixelmap);

    if(IMG_NOT_NULL(context->rPixelMap))
    {
        context->status = SUCCESS;
    }
    else
    {
        context->status = ERROR;
    }
}

void PixelMapNapi::CreatePixelMapComplete(napi_env env, napi_status status, void *data)
{
    napi_value constructor = nullptr;
    napi_value result = nullptr;

    HiLog::Debug(LABEL, "CreatePixelMapComplete IN");
    auto context = static_cast<PixelMapAsyncContext*>(data);

    status = napi_get_reference_value(env, sConstructor_, &constructor);

    if (IMG_IS_OK(status)) {
        sPixelMap_ = context->rPixelMap;
        status = napi_new_instance(env, constructor, 0, nullptr, &result);
    }

    if(!IMG_IS_OK(status))
    {
        context->status = ERROR;
        HiLog::Error(LABEL, "New instance could not be obtained");
        napi_get_undefined(env, &result);
    }

    CommonCallbackRoutine(env, context, result);
    
}

napi_value PixelMapNapi::CreatePixelMap(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    napi_get_undefined(env, &result);

    int32_t refCount = 1;

    napi_status status;
    napi_value thisVar = nullptr;
    napi_value argValue[4] = {0};
    size_t argCount = 4;
    HiLog::Debug(LABEL, "CreatePixelMap IN");

    IMG_JS_ARGS(env, info, status, argCount, argValue, thisVar);

    //we are static method!
    //thisVar is nullptr here
    IMG_NAPI_CHECK_RET_D(IMG_IS_OK(status), nullptr, HiLog::Error(LABEL, "fail to napi_get_cb_info"));
    //IMG_NAPI_CHECK_RET_D(argCount >= 2, nullptr, HiLog::Error(LABEL, "CreatePixelMap args count mismatch %{public}d", argCount));

    std::unique_ptr<PixelMapAsyncContext> asyncContext = std::make_unique<PixelMapAsyncContext>();
//     status = napi_unwrap(env, thisVar, reinterpret_cast<void**>(&asyncContext->nConstructor));
    
//     IMG_NAPI_CHECK_RET_D(IMG_IS_READY(status, asyncContext->nConstructor),
//         nullptr, HiLog::Error(LABEL, "fail to unwrap context"));

    status = napi_get_arraybuffer_info(env, argValue[0], &(asyncContext->colorsBuffer), &(asyncContext->colorsBufferSize));

    IMG_NAPI_CHECK_RET_D(IMG_IS_OK(status), nullptr, HiLog::Error(LABEL, "colors mismatch"));

    IMG_NAPI_CHECK_RET_D(parseInitializationOptions(env, argValue[1], &(asyncContext->opts)),
        nullptr, HiLog::Error(LABEL, "InitializationOptions mismatch"));
    
    if(argCount == 3 && ImageNapiUtils::getType(env, argValue[argCount -1]) == napi_function)
    {
        napi_create_reference(env, argValue[argCount -1], refCount, &asyncContext->callbackRef);
    }

    if (asyncContext->callbackRef == nullptr) {
        napi_create_promise(env, &(asyncContext->deferred), &result);
    } else {
        napi_get_undefined(env, &result);
    }

    IMG_CREATE_CREATE_ASYNC_WORK(env, status, "CreatePixelMap", 
        CreatePixelMapExec, CreatePixelMapComplete, asyncContext, asyncContext->work);

    IMG_NAPI_CHECK_RET_D(IMG_IS_OK(status),
        nullptr, HiLog::Error(LABEL, "fail to create async work"));
    return result;
}

napi_value PixelMapNapi::CreatePixelMap(napi_env env, std::shared_ptr<PixelMap> pixelmap)
{

    napi_value constructor = nullptr;
    napi_value result = nullptr;
    napi_status status;

    HiLog::Debug(LABEL, "CreatePixelMap IN");
    status = napi_get_reference_value(env, sConstructor_, &constructor);

    if (IMG_IS_OK(status)) {
        sPixelMap_ = pixelmap;
        status = napi_new_instance(env, constructor, 0, nullptr, &result);
    }

    if(!IMG_IS_OK(status))
    {
        HiLog::Error(LABEL, "CreatePixelMap | New instance could not be obtained");
        napi_get_undefined(env, &result);
    }

    return result;
}

napi_value PixelMapNapi::GetIsEditable(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    napi_get_undefined(env, &result);

    napi_status status;
    napi_value thisVar = nullptr;
    size_t argCount = 0;
    HiLog::Debug(LABEL, "GetIsEditable IN");

    IMG_JS_ARGS(env, info, status, argCount, nullptr, thisVar);

    IMG_NAPI_CHECK_RET_D(IMG_IS_OK(status), result, HiLog::Error(LABEL, "fail to napi_get_cb_info"));

    std::unique_ptr<PixelMapNapi> pixelMapNapi = std::make_unique<PixelMapNapi>();
    status = napi_unwrap(env, thisVar, reinterpret_cast<void**>(&pixelMapNapi));
    
    IMG_NAPI_CHECK_RET_D(IMG_IS_READY(status, pixelMapNapi), result, HiLog::Error(LABEL, "fail to unwrap context"));

    bool isEditable = pixelMapNapi->nativePixelMap_->IsEditable();

    napi_get_boolean(env, isEditable, &result);

    return result;
}

napi_value PixelMapNapi::ReadPixelsToBuffer(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    napi_get_undefined(env, &result);

    int32_t refCount = 1;
    napi_status status;
    napi_value thisVar = nullptr;
    napi_value argValue[2] = {0};
    size_t argCount = 2;

    HiLog::Debug(LABEL, "ReadPixelsToBuffer IN");
    IMG_JS_ARGS(env, info, status, argCount, argValue, thisVar);

    IMG_NAPI_CHECK_RET_D(IMG_IS_OK(status), nullptr, HiLog::Error(LABEL, "fail to napi_get_cb_info"));
    //IMG_NAPI_CHECK_RET_D(argCount >= 1, nullptr, HiLog::Error(LABEL, "ReadPixelsToBuffer args count mismatch %{public}d", argCount));

    std::unique_ptr<PixelMapAsyncContext> asyncContext = std::make_unique<PixelMapAsyncContext>();
    status = napi_unwrap(env, thisVar, reinterpret_cast<void**>(&asyncContext->nConstructor));
    
    IMG_NAPI_CHECK_RET_D(IMG_IS_READY(status, asyncContext->nConstructor),
        nullptr, HiLog::Error(LABEL, "fail to unwrap context"));

    asyncContext->rPixelMap = asyncContext->nConstructor->nativePixelMap_;

    IMG_NAPI_CHECK_RET_D(IMG_IS_READY(status, asyncContext->rPixelMap),
        nullptr, HiLog::Error(LABEL, "empty native pixelmap"));

    status = napi_get_arraybuffer_info(env, argValue[0],
            &(asyncContext->colorsBuffer), &(asyncContext->colorsBufferSize));

    IMG_NAPI_CHECK_RET_D(IMG_IS_OK(status), nullptr, HiLog::Error(LABEL, "colors mismatch"));
    
    if(argCount == 2 && ImageNapiUtils::getType(env, argValue[argCount -1]) == napi_function)
    {
        napi_create_reference(env, argValue[argCount -1], refCount, &asyncContext->callbackRef);
    }

    if (asyncContext->callbackRef == nullptr) {
        napi_create_promise(env, &(asyncContext->deferred), &result);
    } else {
        napi_get_undefined(env, &result);
    }

    IMG_CREATE_CREATE_ASYNC_WORK(env, status, "ReadPixelsToBuffer", 
        [](napi_env env, void *data)
        {
            auto context = static_cast<PixelMapAsyncContext*>(data);
            context->status = context->rPixelMap->ReadPixels(
                context->colorsBufferSize, static_cast<uint8_t*>(context->colorsBuffer));
        }
        , EmptyResultComplete, asyncContext, asyncContext->work);

    IMG_NAPI_CHECK_RET_D(IMG_IS_OK(status),
        nullptr, HiLog::Error(LABEL, "fail to create async work"));
    return result;

}

napi_value PixelMapNapi::ReadPixels(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    napi_get_undefined(env, &result);

    int32_t refCount = 1;
    napi_status status;
    napi_value thisVar = nullptr;
    napi_value argValue[2] = {0};
    size_t argCount = 2;

    HiLog::Debug(LABEL, "ReadPixels IN");
    IMG_JS_ARGS(env, info, status, argCount, argValue, thisVar);

    IMG_NAPI_CHECK_RET_D(IMG_IS_OK(status), nullptr, HiLog::Error(LABEL, "fail to napi_get_cb_info"));
    //IMG_NAPI_CHECK_RET_D(argCount >= 1, nullptr, HiLog::Error(LABEL, "ReadPixels args count mismatch %{public}d", argCount));

    std::unique_ptr<PixelMapAsyncContext> asyncContext = std::make_unique<PixelMapAsyncContext>();
    status = napi_unwrap(env, thisVar, reinterpret_cast<void**>(&asyncContext->nConstructor));
    
    IMG_NAPI_CHECK_RET_D(IMG_IS_READY(status, asyncContext->nConstructor),
        nullptr, HiLog::Error(LABEL, "fail to unwrap context"));

    asyncContext->rPixelMap = asyncContext->nConstructor->nativePixelMap_;

    IMG_NAPI_CHECK_RET_D(IMG_IS_READY(status, asyncContext->rPixelMap),
        nullptr, HiLog::Error(LABEL, "empty native pixelmap"));
    
    IMG_NAPI_CHECK_RET_D(parsePositionArea(env, argValue[0], &(asyncContext->area)),
        nullptr, HiLog::Error(LABEL, "fail to parse position area"));

    if(argCount == 2 && ImageNapiUtils::getType(env, argValue[argCount -1]) == napi_function)
    {
        napi_create_reference(env, argValue[argCount -1], refCount, &asyncContext->callbackRef);
    }

    if (asyncContext->callbackRef == nullptr) {
        napi_create_promise(env, &(asyncContext->deferred), &result);
    } else {
        napi_get_undefined(env, &result);
    }

    IMG_CREATE_CREATE_ASYNC_WORK(env, status, "ReadPixels",
        [](napi_env env, void *data)
        {
            auto context = static_cast<PixelMapAsyncContext*>(data);
            auto area = context->area;
            context->status = context->rPixelMap->ReadPixels(
                area.size, area.offset, area.stride, area.region, static_cast<uint8_t*>(area.pixels));
        }, EmptyResultComplete, asyncContext, asyncContext->work);

    IMG_NAPI_CHECK_RET_D(IMG_IS_OK(status),
        nullptr, HiLog::Error(LABEL, "fail to create async work"));
    return result;
}

napi_value PixelMapNapi::WritePixels(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    napi_get_undefined(env, &result);

    int32_t refCount = 1;
    napi_status status;
    napi_value thisVar = nullptr;
    napi_value argValue[2] = {0};
    size_t argCount = 2;

    HiLog::Debug(LABEL, "WritePixels IN");
    IMG_JS_ARGS(env, info, status, argCount, argValue, thisVar);

    IMG_NAPI_CHECK_RET_D(IMG_IS_OK(status), nullptr, HiLog::Error(LABEL, "fail to napi_get_cb_info"));
    //IMG_NAPI_CHECK_RET_D(argCount >= 1, nullptr, HiLog::Error(LABEL, "WritePixels args count mismatch %{public}d", argCount));

    std::unique_ptr<PixelMapAsyncContext> asyncContext = std::make_unique<PixelMapAsyncContext>();
    status = napi_unwrap(env, thisVar, reinterpret_cast<void**>(&asyncContext->nConstructor));
    
    IMG_NAPI_CHECK_RET_D(IMG_IS_READY(status, asyncContext->nConstructor),
        nullptr, HiLog::Error(LABEL, "fail to unwrap context"));

    asyncContext->rPixelMap = asyncContext->nConstructor->nativePixelMap_;

    IMG_NAPI_CHECK_RET_D(IMG_IS_READY(status, asyncContext->rPixelMap),
        nullptr, HiLog::Error(LABEL, "empty native pixelmap"));
    
    IMG_NAPI_CHECK_RET_D(parsePositionArea(env, argValue[0], &(asyncContext->area)),
        nullptr, HiLog::Error(LABEL, "fail to parse position area"));

    if(argCount == 2 && ImageNapiUtils::getType(env, argValue[argCount -1]) == napi_function)
    {
        napi_create_reference(env, argValue[argCount -1], refCount, &asyncContext->callbackRef);
    }

    if (asyncContext->callbackRef == nullptr) {
        napi_create_promise(env, &(asyncContext->deferred), &result);
    } else {
        napi_get_undefined(env, &result);
    }

    IMG_CREATE_CREATE_ASYNC_WORK(env, status, "WritePixels",
        [](napi_env env, void *data)
        {
            auto context = static_cast<PixelMapAsyncContext*>(data);
            auto area = context->area;
            context->status = context->rPixelMap->WritePixels(
                static_cast<uint8_t*>(area.pixels), area.size, area.offset, area.stride, area.region);
        }, EmptyResultComplete, asyncContext, asyncContext->work);

    IMG_NAPI_CHECK_RET_D(IMG_IS_OK(status),
        nullptr, HiLog::Error(LABEL, "fail to create async work"));
    return result;

}

napi_value PixelMapNapi::WriteBufferToPixels(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    napi_get_undefined(env, &result);

    int32_t refCount = 1;
    napi_status status;
    napi_value thisVar = nullptr;
    napi_value argValue[2] = {0};
    size_t argCount = 2;

    HiLog::Debug(LABEL, "WriteBufferToPixels IN");
    IMG_JS_ARGS(env, info, status, argCount, argValue, thisVar);

    IMG_NAPI_CHECK_RET_D(IMG_IS_OK(status), nullptr, HiLog::Error(LABEL, "fail to napi_get_cb_info"));
    //IMG_NAPI_CHECK_RET_D(argCount >= 1, nullptr, HiLog::Error(LABEL, "WriteBufferToPixels args count mismatch %{public}d", argCount));

    std::unique_ptr<PixelMapAsyncContext> asyncContext = std::make_unique<PixelMapAsyncContext>();
    status = napi_unwrap(env, thisVar, reinterpret_cast<void**>(&asyncContext->nConstructor));
    
    IMG_NAPI_CHECK_RET_D(IMG_IS_READY(status, asyncContext->nConstructor),
        nullptr, HiLog::Error(LABEL, "fail to unwrap context"));

    asyncContext->rPixelMap = asyncContext->nConstructor->nativePixelMap_;

    IMG_NAPI_CHECK_RET_D(IMG_IS_READY(status, asyncContext->rPixelMap),
        nullptr, HiLog::Error(LABEL, "empty native pixelmap"));
    status = napi_get_arraybuffer_info(env, argValue[0],
        &(asyncContext->colorsBuffer), &(asyncContext->colorsBufferSize));
    
    IMG_NAPI_CHECK_RET_D(IMG_IS_OK(status),
        nullptr, HiLog::Error(LABEL, "fail to get buffer info"));

    if(argCount == 2 && ImageNapiUtils::getType(env, argValue[argCount -1]) == napi_function)
    {
        napi_create_reference(env, argValue[argCount -1], refCount, &asyncContext->callbackRef);
    }

    if (asyncContext->callbackRef == nullptr) {
        napi_create_promise(env, &(asyncContext->deferred), &result);
    } else {
        napi_get_undefined(env, &result);
    }

    IMG_CREATE_CREATE_ASYNC_WORK(env, status, "WriteBufferToPixels",
        [](napi_env env, void *data)
        {
            auto context = static_cast<PixelMapAsyncContext*>(data);
            context->status = context->rPixelMap->WritePixels(static_cast<uint8_t*>(context->colorsBuffer), context->colorsBufferSize);

        }, EmptyResultComplete, asyncContext, asyncContext->work);

    IMG_NAPI_CHECK_RET_D(IMG_IS_OK(status),
        nullptr, HiLog::Error(LABEL, "fail to create async work"));
    return result;
}

napi_value PixelMapNapi::GetImageInfo(napi_env env, napi_callback_info info)
{
    HiLog::Debug(LABEL, "GetImageInfo IN");
    return nullptr;
}

napi_value PixelMapNapi::GetBytesNumberPerRow(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    napi_get_undefined(env, &result);

    int32_t refCount = 1;
    napi_status status;
    napi_value thisVar = nullptr;
    napi_value argValue[1] = {0};
    size_t argCount = 1;

    HiLog::Debug(LABEL, "GetBytesNumberPerRow IN");
    IMG_JS_ARGS(env, info, status, argCount, argValue, thisVar);

    IMG_NAPI_CHECK_RET_D(IMG_IS_OK(status), nullptr, HiLog::Error(LABEL, "fail to napi_get_cb_info"));

    std::unique_ptr<PixelMapAsyncContext> asyncContext = std::make_unique<PixelMapAsyncContext>();
    status = napi_unwrap(env, thisVar, reinterpret_cast<void**>(&asyncContext->nConstructor));
    
    IMG_NAPI_CHECK_RET_D(IMG_IS_READY(status, asyncContext->nConstructor),
        nullptr, HiLog::Error(LABEL, "fail to unwrap context"));

    asyncContext->rPixelMap = asyncContext->nConstructor->nativePixelMap_;

    IMG_NAPI_CHECK_RET_D(IMG_IS_READY(status, asyncContext->rPixelMap),
        nullptr, HiLog::Error(LABEL, "empty native pixelmap"));

    if(argCount == 1 && ImageNapiUtils::getType(env, argValue[argCount -1]) == napi_function)
    {
        napi_create_reference(env, argValue[argCount -1], refCount, &asyncContext->callbackRef);
    }

    if (asyncContext->callbackRef == nullptr) {
        napi_create_promise(env, &(asyncContext->deferred), &result);
    } else {
        napi_get_undefined(env, &result);
    }

    IMG_CREATE_CREATE_ASYNC_WORK(env, status, "GetBytesNumberPerRow",
        [](napi_env env, void *data)
        {
            auto context = static_cast<PixelMapAsyncContext*>(data);
            context->resultUint32 = context->rPixelMap->GetRowBytes();
            context->status = SUCCESS;

        }, Uint32ResultComplete, asyncContext, asyncContext->work);

    IMG_NAPI_CHECK_RET_D(IMG_IS_OK(status),
        nullptr, HiLog::Error(LABEL, "fail to create async work"));
    return result;
}

napi_value PixelMapNapi::GetPixelBytesNumber(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    napi_get_undefined(env, &result);

    int32_t refCount = 1;
    napi_status status;
    napi_value thisVar = nullptr;
    napi_value argValue[1] = {0};
    size_t argCount = 1;

    HiLog::Debug(LABEL, "GetPixelBytesNumber IN");
    IMG_JS_ARGS(env, info, status, argCount, argValue, thisVar);

    IMG_NAPI_CHECK_RET_D(IMG_IS_OK(status), nullptr, HiLog::Error(LABEL, "fail to napi_get_cb_info"));

    std::unique_ptr<PixelMapAsyncContext> asyncContext = std::make_unique<PixelMapAsyncContext>();
    status = napi_unwrap(env, thisVar, reinterpret_cast<void**>(&asyncContext->nConstructor));
    
    IMG_NAPI_CHECK_RET_D(IMG_IS_READY(status, asyncContext->nConstructor),
        nullptr, HiLog::Error(LABEL, "fail to unwrap context"));

    asyncContext->rPixelMap = asyncContext->nConstructor->nativePixelMap_;

    IMG_NAPI_CHECK_RET_D(IMG_IS_READY(status, asyncContext->rPixelMap),
        nullptr, HiLog::Error(LABEL, "empty native pixelmap"));

    if(argCount == 1 && ImageNapiUtils::getType(env, argValue[argCount -1]) == napi_function)
    {
        napi_create_reference(env, argValue[argCount -1], refCount, &asyncContext->callbackRef);
    }

    if (asyncContext->callbackRef == nullptr) {
        napi_create_promise(env, &(asyncContext->deferred), &result);
    } else {
        napi_get_undefined(env, &result);
    }

    IMG_CREATE_CREATE_ASYNC_WORK(env, status, "GetPixelBytesNumber",
        [](napi_env env, void *data)
        {
            auto context = static_cast<PixelMapAsyncContext*>(data);
            context->resultUint32 = context->rPixelMap->GetByteCount();
            context->status = SUCCESS;

        }, Uint32ResultComplete, asyncContext, asyncContext->work);

    IMG_NAPI_CHECK_RET_D(IMG_IS_OK(status),
        nullptr, HiLog::Error(LABEL, "fail to create async work"));
    return result;
}

napi_value PixelMapNapi::Release(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    napi_get_undefined(env, &result);

    napi_status status;
    napi_value thisVar = nullptr;
    size_t argCount = 0;

    IMG_JS_ARGS(env, info, status, argCount, nullptr, thisVar);

    IMG_NAPI_CHECK_RET_D(IMG_IS_OK(status), result, HiLog::Error(LABEL, "fail to napi_get_cb_info"));

    std::unique_ptr<PixelMapNapi> pixelMapNapi = std::make_unique<PixelMapNapi>();
    status = napi_unwrap(env, thisVar, reinterpret_cast<void**>(&pixelMapNapi));
    
    IMG_NAPI_CHECK_RET_D(IMG_IS_READY(status, pixelMapNapi), result, HiLog::Error(LABEL, "fail to unwrap context"));

    pixelMapNapi->nativePixelMap_= nullptr;

    return result;
}

}  // namespace Media
}  // namespace OHOS
