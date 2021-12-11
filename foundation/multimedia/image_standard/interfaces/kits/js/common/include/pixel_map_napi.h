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

#ifndef PIXEL_MAP_NAPI_H_
#define PIXEL_MAP_NAPI_H_

#include "pixel_map.h"
#include "image_type.h"
#include "image_source.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"

namespace OHOS {
namespace Media {
class PixelMapNapi {
public:
    PixelMapNapi();
    ~PixelMapNapi();

    static napi_value Init(napi_env env, napi_value exports);

    static napi_value CreatePixelMap(napi_env env, std::shared_ptr<PixelMap> pixelmap);
    static std::shared_ptr<PixelMap> GetPixelMap(napi_env env, napi_value pixelmap);

private:
    static napi_value Constructor(napi_env env, napi_callback_info info);
    static void Destructor(napi_env env, void *nativeObject, void *finalize);

    // readonly property
    static napi_value GetIsEditable(napi_env env, napi_callback_info info);

    // static mothod
    static napi_value CreatePixelMap(napi_env env, napi_callback_info info);
    static void CreatePixelMapComplete(napi_env env, napi_status status, void *data);

    // methods
    static napi_value ReadPixelsToBuffer(napi_env env, napi_callback_info info);
    static napi_value ReadPixels(napi_env env, napi_callback_info info);
    static napi_value WritePixels(napi_env env, napi_callback_info info);
    static napi_value WriteBufferToPixels(napi_env env, napi_callback_info info);
    static napi_value GetImageInfo(napi_env env, napi_callback_info info);
    static napi_value GetBytesNumberPerRow(napi_env env, napi_callback_info info);
    static napi_value GetPixelBytesNumber(napi_env env, napi_callback_info info);
    static napi_value getPixelBytesCount(napi_env env, napi_callback_info info);
    static napi_value Release(napi_env env, napi_callback_info info);

    static napi_ref sConstructor_;
    static std::shared_ptr<PixelMap> sPixelMap_;

    napi_env env_ = nullptr;
    napi_ref wrapper_ = nullptr;
    std::shared_ptr<PixelMap> nativePixelMap_;
    bool isRelease = false;
};
} // namespace Media
} // namespace OHOS
#endif /* PIXEL_MAP_NAPI_H_ */
