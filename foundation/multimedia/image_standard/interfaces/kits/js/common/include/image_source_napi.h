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

#ifndef IMAGE_SOURCE_NAPI_H_
#define IMAGE_SOURCE_NAPI_H_

#include "pixel_map.h"
#include "image_type.h"
#include "image_source.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"

namespace OHOS {
namespace Media {
class ImageSourceNapi {
public:
    ImageSourceNapi();
    ~ImageSourceNapi();

    static napi_value Init(napi_env env, napi_value exports);

private:
    static napi_value Constructor(napi_env env, napi_callback_info info);
    static void Destructor(napi_env env, void *nativeObject, void *finalize);

    // readonly property
    static napi_value GetSupportedFormats(napi_env env, napi_callback_info info);

    // static methods
    static napi_value CreateImageSource(napi_env env, napi_callback_info info);
    static napi_value CreateIncrementalSource(napi_env env, napi_callback_info info);

    // methods
    static napi_value GetImageInfo(napi_env env, napi_callback_info info);
    static napi_value CreatePixelMap(napi_env env, napi_callback_info info);
    static napi_value GetImagePropertyInt(napi_env env, napi_callback_info info);
    static napi_value GetImagePropertyString(napi_env env, napi_callback_info info);
    static napi_value UpdateData(napi_env env, napi_callback_info info);
    static napi_value Release(napi_env env, napi_callback_info info);

    static napi_ref sConstructor_;
    static std::shared_ptr<ImageSource> sImgSrc_;

    napi_env env_ = nullptr;
    napi_ref wrapper_ = nullptr;
    std::shared_ptr<ImageSource> nativeImgSrc = nullptr;
    bool isRelease = false;
};
} // namespace Media
} // namespace OHOS
#endif /* PIXEL_MAP_NAPI_H_ */
