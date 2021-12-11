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

#ifndef ALBUM_ASSET_NAPI_H
#define ALBUM_ASSET_NAPI_H

#include "album_asset.h"
#include "video_asset_napi.h"
#include "image_asset_napi.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include <vector>
#include <algorithm>

namespace OHOS {
static const std::string ALBUM_ASSET_NAPI_CLASS_NAME = "Album";

class AlbumAssetNapi {
public:
    static napi_value Init(napi_env env, napi_value exports);
    static napi_value CreateAlbumAsset(napi_env env, AlbumType type,
                                       const std::string &albumParentPath,
                                       Media::AlbumAsset &aAsset,
                                       Media::IMediaLibraryClient &mediaLibClient);
    Media::IMediaLibraryClient* GetMediaLibClientInstance();

    AlbumAssetNapi();
    ~AlbumAssetNapi();

private:
    static void AlbumAssetNapiDestructor(napi_env env, void* nativeObject, void* finalize_hint);
    static napi_value AlbumAssetNapiConstructor(napi_env env, napi_callback_info info);

    static napi_value GetAlbumId(napi_env env, napi_callback_info info);
    static napi_value GetAlbumName(napi_env env, napi_callback_info info);
    static napi_value JSSetAlbumName(napi_env env, napi_callback_info info);
    static napi_value GetVideoAssets(napi_env env, napi_callback_info info);
    static napi_value GetImageAssets(napi_env env, napi_callback_info info);

    static napi_value CommitCreate(napi_env env, napi_callback_info info);
    static napi_value CommitDelete(napi_env env, napi_callback_info info);
    static napi_value CommitModify(napi_env env, napi_callback_info info);

    void UpdateAlbumAssetInfo();

    int32_t albumId_;
    std::string albumName_;
    std::vector<std::unique_ptr<Media::VideoAsset>> videoAssets_;
    std::vector<std::unique_ptr<Media::ImageAsset>> imageAssets_;
    std::string newAlbumName_ = "";
    AlbumType type_;
    std::string albumPath_ = "";
    Media::IMediaLibraryClient *mediaLibrary_;

    napi_env env_;
    napi_ref wrapper_;

    static napi_ref sConstructor_;
    static Media::AlbumAsset *sAlbumAsset_;
    static AlbumType sAlbumType_;
    static std::string sAlbumPath_;
    static Media::IMediaLibraryClient *sMediaLibrary_;
};

struct AlbumAsyncContext {
    napi_env env;
    napi_async_work work;
    napi_deferred deferred;
    napi_ref callbackRef;
    AlbumAssetNapi* objectInfo;
    bool status;
    std::vector<std::unique_ptr<Media::VideoAsset>> videoAssets;
    std::vector<std::unique_ptr<Media::ImageAsset>> imageAssets;
};
} // namespace OHOS
#endif /* ALBUM_ASSET_NAPI_H */
