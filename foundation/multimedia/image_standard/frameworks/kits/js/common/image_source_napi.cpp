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

#include "hilog/log.h"
#include "image_source_napi.h"
#include "media_errors.h"

using OHOS::HiviewDFX::HiLog;
namespace {
    constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "ImageSourceNapi"};
}

namespace OHOS {
namespace Media {
napi_ref ImageSourceNapi::sConstructor_ = nullptr;
static const std::string CLASS_NAME = "PixelMap";

struct ImageSourceAsyncContext {
    napi_env env;
    napi_async_work work;
    napi_deferred deferred;
    napi_ref callbackRef = nullptr;
    ImageSourceNapi *constructor_;
};

ImageSourceNapi::ImageSourceNapi()
{

}

ImageSourceNapi::~ImageSourceNapi()
{
    if (wrapper_ != nullptr)
    {
        napi_delete_reference(env_, wrapper_);
    }
}

napi_value ImageSourceNapi::Init(napi_env env, napi_value exports)
{
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("getImageInfo", GetImageInfo),
        DECLARE_NAPI_FUNCTION("getImagePropertyInt", GetImagePropertyInt),
        DECLARE_NAPI_FUNCTION("getImagePropertyString", GetImagePropertyString),
        DECLARE_NAPI_FUNCTION("createPixelMap", CreatePixelMap),
        DECLARE_NAPI_FUNCTION("updateData", UpdateData),
        DECLARE_NAPI_FUNCTION("release", Release),
        DECLARE_NAPI_GETTER("supportedFormats", GetSupportedFormats),
    };

    napi_property_descriptor static_prop[] = {
        DECLARE_NAPI_STATIC_FUNCTION("createImageSource", CreateImageSource),
        DECLARE_NAPI_STATIC_FUNCTION("createIncrementalSource", CreateIncrementalSource),
    };

    napi_value constructor = nullptr;
    napi_status status = napi_define_class(env, CLASS_NAME.c_str(), NAPI_AUTO_LENGTH, Constructor, nullptr,
        sizeof(properties) / sizeof(properties[0]), properties, &constructor);

    if (status != napi_ok)
    {
        HiLog::Error(LABEL, "define class fail");
        return nullptr;
    }

    status = napi_create_reference(env, constructor, 1, &sConstructor_);
    if (status != napi_ok)
    {
        HiLog::Error(LABEL, "create reference fail");
        return nullptr;
    }

    status = napi_set_named_property(env, exports, CLASS_NAME.c_str(), constructor);
    if (status != napi_ok)
    {
        HiLog::Error(LABEL, "set named property fail");
        return nullptr;
    }
    
    status = napi_define_properties(env, exports, sizeof(static_prop) / sizeof(static_prop[0]), static_prop);
    if (status != napi_ok)
    {
        HiLog::Error(LABEL, "define properties fail");
        return nullptr;
    }
    HiLog::Debug(LABEL, "Init success");
    return exports;
}

napi_value ImageSourceNapi::Constructor(napi_env env, napi_callback_info info)
{
    napi_value undefineValue = nullptr;
    napi_get_undefined(env, &undefineValue);

    napi_status status;
    napi_value thisVar = nullptr;

    status = napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr);

    if (status == napi_ok && thisVar != nullptr)
    {
        std::unique_ptr<ImageSourceNapi> pImgSrcNapi = std::make_unique<ImageSourceNapi>();
        if (pImgSrcNapi != nullptr)
        {
            pImgSrcNapi->env_ = env;
            pImgSrcNapi->nativeImgSrc = sImgSrc_;

            status = napi_wrap(env, thisVar, reinterpret_cast<void*>(pImgSrcNapi.get()),
                               ImageSourceNapi::Destructor, nullptr, &(pImgSrcNapi->wrapper_));
            if (status == napi_ok)
            {
                pImgSrcNapi.release();
                return thisVar;
            }
            else
            {
                HiLog::Error(LABEL, "Failure wrapping js to native napi");
            }
        }
    }

    return undefineValue;
}

void ImageSourceNapi::Destructor(napi_env env, void *nativeObject, void *finalize)
{
    ImageSourceNapi *pImgSrcNapi = reinterpret_cast<ImageSourceNapi*>(nativeObject);
    if (pImgSrcNapi != nullptr)
    {
        pImgSrcNapi->~ImageSourceNapi();
    }
}

static void GetSupportedFormatsExecute(napi_env env, void *data)
{

}

static void GetSupportedFormatsComplete(napi_env env, napi_status status, void *data)
{

}

napi_value ImageSourceNapi::GetSupportedFormats(napi_env env, napi_callback_info info)
{
    return nullptr;
}

napi_value ImageSourceNapi::CreateImageSource(napi_env env, napi_callback_info info)
{

    return nullptr;
}

napi_value ImageSourceNapi::CreateIncrementalSource(napi_env env, napi_callback_info info)
{
    return nullptr;
}

static void GetImageInfoExecute(napi_env env, void *data)
{

}

static void GetImageInfoComplete(napi_env env, napi_status status, void *data)
{

}

napi_value ImageSourceNapi::GetImageInfo(napi_env env, napi_callback_info info)
{
    return nullptr;

}

static void CreatePixelMapExecute(napi_env env, void *data)
{

}

static void CreatePixelMapComplete(napi_env env, napi_status status, void *data)
{

}

napi_value ImageSourceNapi::CreatePixelMap(napi_env env, napi_callback_info info)
{
    return nullptr;

}

static void GetImagePropertyIntExecute(napi_env env, void *data)
{

}

static void GetImagePropertyIntComplete(napi_env env, napi_status status, void *data)
{

}

napi_value ImageSourceNapi::GetImagePropertyInt(napi_env env, napi_callback_info info)
{
    return nullptr;

}

static void GetImagePropertyStringExecute(napi_env env, void *data)
{

}

static void GetImagePropertyStringComplete(napi_env env, napi_status status, void *data)
{

}

napi_value ImageSourceNapi::GetImagePropertyString(napi_env env, napi_callback_info info)
{
    return nullptr;

}

static void UpdateDataExecute(napi_env env, void *data)
{

}

static void UpdateDataComplete(napi_env env, napi_status status, void *data)
{

}

napi_value ImageSourceNapi::UpdateData(napi_env env, napi_callback_info info)
{
    return nullptr;

}

static void ReleaseExecute(napi_env env, void *data)
{

}

static void ReleaseComplete(napi_env env, napi_status status, void *data)
{

}

napi_value ImageSourceNapi::Release(napi_env env, napi_callback_info info)
{
    return nullptr;

}


}  // namespace Media
}  // namespace OHOS