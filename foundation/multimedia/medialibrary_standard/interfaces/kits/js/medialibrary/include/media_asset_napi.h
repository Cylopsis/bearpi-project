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

#ifndef MEDIA_ASSET_NAPI_H
#define MEDIA_ASSET_NAPI_H

#include "media_asset.h"
#include "IMediaLibraryClient.h"
#include "medialibrary_napi_utils.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"

namespace OHOS {
static const std::string MEDIA_ASSET_NAPI_CLASS_NAME = "MediaAsset";

class MediaAssetNapi {
public:
    static napi_value Init(napi_env env, napi_value exports);
    static napi_value CreateMediaAsset(napi_env env, Media::MediaAsset &mAsset,
                                       Media::IMediaLibraryClient &mediaLibClient);

    MediaAssetNapi();
    virtual ~MediaAssetNapi();

    static napi_value GetId(napi_env env, napi_callback_info info);
    static napi_value GetUri(napi_env env, napi_callback_info info);
    static napi_value GetMediaType(napi_env env, napi_callback_info info);
    static napi_value GetName(napi_env env, napi_callback_info info);
    static napi_value GetSize(napi_env env, napi_callback_info info);
    static napi_value GetDateAdded(napi_env env, napi_callback_info info);
    static napi_value GetDateModified(napi_env env, napi_callback_info info);
    static napi_value GetAlbumName(napi_env env, napi_callback_info info);
    static napi_value GetAlbumId(napi_env env, napi_callback_info info);

    static napi_value StartCreate(napi_env env, napi_callback_info info);
    static napi_value StartModify(napi_env env, napi_callback_info info);

    static napi_value CancelCreate(napi_env env, napi_callback_info info);
    static napi_value CancelModify(napi_env env, napi_callback_info info);

    static napi_value CommitCreate(napi_env env, napi_callback_info info);
    static napi_value CommitModify(napi_env env, napi_callback_info info);
    static napi_value CommitDelete(napi_env env, napi_callback_info info);
    static napi_value CommitCopy(napi_env env, napi_callback_info info);

    static napi_value JSSetName(napi_env env, napi_callback_info info);
    static napi_value JSSetAlbumName(napi_env env, napi_callback_info info);

    void UpdateNativeMediaAsset(Media::MediaAsset& mAsset);

    void SetId(int32_t id);
    void SetUri(std::string uri);
    void SetMediaType(int32_t mediaType);
    void SetName(std::string name);
    void SetSize(uint64_t size);
    void SetDateAdded(uint64_t dateAdded);
    void SetDateModified(uint64_t dateModified);
    void SetAlbumName(std::string albumName);
    void SetAlbumId(int32_t albumId);
    void SetMediaLibraryClient(Media::IMediaLibraryClient &mediaLibrary);

private:
    static void MediaAssetNapiDestructor(napi_env env, void* nativeObject, void* finalize_hint);
    static napi_value MediaAssetNapiConstructor(napi_env env, napi_callback_info info);

    void UpdateMediaAssetInfo(Media::MediaAsset &mediaAsset);

    napi_env env_;
    napi_ref wrapper_;

    int32_t id_;
    std::string uri_;
    int32_t mediaType_;
    std::string name_;
    uint64_t size_;
    uint64_t dateAdded_;
    uint64_t dateModified_;
    std::string albumName_;
    int32_t albumId_;
    Media::IMediaLibraryClient *mediaLibrary_;

    std::string newName_ = "";
    std::string newAlbumName_ = "";
    bool startCreateFlag = false;
    bool startModifyFlag = false;

    static napi_ref sConstructor_;
    static Media::MediaAsset *sMediaAsset_;
    static Media::IMediaLibraryClient *sMediaLibrary_;
};

struct MediaAssetAsyncContext {
    napi_env env;
    napi_async_work work;
    napi_deferred deferred;
    napi_ref callbackRef;
    MediaAssetNapi* objectInfo;
    MediaAssetNapi* targetCopyObject;
    bool status;
};
} // namespace OHOS
#endif /* MEDIA_ASSET_NAPI_H */
