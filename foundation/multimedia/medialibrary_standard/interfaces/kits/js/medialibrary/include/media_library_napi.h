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

#ifndef MEDIA_LIBRARY_NAPI_H
#define MEDIA_LIBRARY_NAPI_H

#include "IMediaLibraryClient.h"

#include "media_asset_napi.h"
#include "audio_asset_napi.h"
#include "video_asset_napi.h"
#include "image_asset_napi.h"
#include "album_asset_napi.h"

#include "napi/native_api.h"
#include "napi/native_node_api.h"

namespace OHOS {
static const std::string MEDIA_LIBRARY_NAPI_CLASS_NAME = "MediaLibraryHelper";

class MediaLibraryNapi {
public:
    static napi_value Init(napi_env env, napi_value exports);
    Media::IMediaLibraryClient* GetMediaLibClientInstance();

    MediaLibraryNapi();
    ~MediaLibraryNapi();

private:
    static void MediaLibraryNapiDestructor(napi_env env, void* nativeObject, void* finalize_hint);
    static napi_value MediaLibraryNapiConstructor(napi_env env, napi_callback_info info);

    static napi_value GetMediaLibraryInstance(napi_env env, napi_callback_info info);
    static napi_value GetMediaAssets(napi_env env, napi_callback_info info);
    static napi_value GetAudioAssets(napi_env env, napi_callback_info info);
    static napi_value GetVideoAssets(napi_env env, napi_callback_info info);
    static napi_value GetImageAssets(napi_env env, napi_callback_info info);
    static napi_value GetVideoAlbums(napi_env env, napi_callback_info info);
    static napi_value GetImageAlbums(napi_env env, napi_callback_info info);
    static napi_value CreateAudioAsset(napi_env env, napi_callback_info info);
    static napi_value CreateVideoAsset(napi_env env, napi_callback_info info);
    static napi_value CreateImageAsset(napi_env env, napi_callback_info info);
    static napi_value CreateAlbum(napi_env env, napi_callback_info info);

    Media::IMediaLibraryClient *mediaLibrary_;

    napi_env env_;
    napi_ref wrapper_;

    static napi_ref sConstructor_;
};

struct MediaLibraryAsyncContext {
    napi_env env;
    napi_async_work work;
    napi_deferred deferred;
    napi_ref callbackRef;
    int status;
    AssetType assetType;
    AlbumType albumType;
    MediaLibraryNapi* objectInfo;
    std::string selection;
    std::vector<std::string> selectionArgs;
    std::vector<std::unique_ptr<Media::MediaAsset>> mediaAssets;
    std::vector<std::unique_ptr<Media::AudioAsset>> audioAssets;
    std::vector<std::unique_ptr<Media::VideoAsset>> videoAssets;
    std::vector<std::unique_ptr<Media::ImageAsset>> imageAssets;
    std::vector<std::unique_ptr<Media::AlbumAsset>> albumAssets;
};
} // namespace OHOS
#endif /* MEDIA_LIBRARY_NAPI_H */
