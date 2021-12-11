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

#ifndef IMAGE_ASSET_NAPI_H
#define IMAGE_ASSET_NAPI_H

#include "image_asset.h"
#include "media_asset_napi.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"

namespace OHOS {
static const std::string IMAGE_ASSET_NAPI_CLASS_NAME = "ImageAsset";

class ImageAssetNapi : public MediaAssetNapi {
public:
    static napi_value Init(napi_env env, napi_value exports);
    static napi_value CreateImageAsset(napi_env env, Media::ImageAsset &iAsset,
                                       Media::IMediaLibraryClient &mediaLibClient);

    ImageAssetNapi();
    ~ImageAssetNapi();

private:
    static void ImageAssetNapiDestructor(napi_env env, void* nativeObject, void* finalize_hint);
    static napi_value ImageAssetNapiConstructor(napi_env env, napi_callback_info info);

    static napi_value GetMimeType(napi_env env, napi_callback_info info);
    static napi_value GetWidth(napi_env env, napi_callback_info info);
    static napi_value GetHeight(napi_env env, napi_callback_info info);

    void UpdateImageAssetInfo();

    std::string mimeType_;
    int32_t width_;
    int32_t height_;

    napi_env env_;
    napi_ref wrapper_;

    static napi_ref sConstructor_;
    static Media::ImageAsset *sImageAsset_;
    static Media::IMediaLibraryClient *sMediaLibrary_;
};
} // namespace OHOS
#endif /* IMAGE_ASSET_NAPI_H */
