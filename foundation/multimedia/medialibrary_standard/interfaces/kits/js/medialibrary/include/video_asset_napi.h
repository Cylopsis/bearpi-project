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

#ifndef VIDEO_ASSET_NAPI_H
#define VIDEO_ASSET_NAPI_H

#include "video_asset.h"
#include "media_asset_napi.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"

namespace OHOS {
static const std::string VIDEO_ASSET_NAPI_CLASS_NAME = "VideoAsset";

class VideoAssetNapi : public MediaAssetNapi {
public:
    static napi_value Init(napi_env env, napi_value exports);
    static napi_value CreateVideoAsset(napi_env env, Media::VideoAsset &vAsset,
                                       Media::IMediaLibraryClient &mediaLibClient);

    VideoAssetNapi();
    ~VideoAssetNapi();

private:
    static void VideoAssetNapiDestructor(napi_env env, void* nativeObject, void* finalize_hint);
    static napi_value VideoAssetNapiConstructor(napi_env env, napi_callback_info info);

    static napi_value GetMimeType(napi_env env, napi_callback_info info);
    static napi_value GetWidth(napi_env env, napi_callback_info info);
    static napi_value GetHeight(napi_env env, napi_callback_info info);
    static napi_value GetDuration(napi_env env, napi_callback_info info);

    void UpdateVideoAssetInfo();

    std::string mimeType_;
    int32_t width_;
    int32_t height_;
    int32_t duration_;

    napi_env env_;
    napi_ref wrapper_;

    static napi_ref sConstructor_;
    static Media::VideoAsset *sVideoAsset_;
    static Media::IMediaLibraryClient *sMediaLibrary_;
};
} // namespace OHOS
#endif /* VIDEO_ASSET_NAPI_H */
