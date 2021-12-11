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

#include "media_lib_service.h"
#include "iservice_registry.h"
#include "media_library.h"
#include "media_log.h"
#include "system_ability_definition.h"

using namespace std;

namespace OHOS {
namespace Media {
REGISTER_SYSTEM_ABILITY_BY_ID(MediaLibService, MEDIA_LIBRARY_SERVICE_ID, true);

MediaLibService::MediaLibService(int32_t systemAbilityId, bool runOnCreate)
    : SystemAbility(systemAbilityId, runOnCreate) { }

void MediaLibService::OnDump() { }

void MediaLibService::OnStart()
{
    MEDIA_INFO_LOG(" MediaLibService::OnStart() called");
    bool res = Publish(this);
    if (res) {
        MEDIA_DEBUG_LOG("MediaLibService OnStart res valid");
    }
    mediaLibInstance = MediaLibrary::GetMediaLibraryInstance();
    if (mediaLibInstance == nullptr) {
        MEDIA_ERR_LOG("GetMediaLibraryInstance failed");
    }
}

void MediaLibService::OnStop()
{
    MEDIA_INFO_LOG("MediaLibService::OnStop() called");
    mediaLibInstance = nullptr;
}

vector<unique_ptr<MediaAsset>> MediaLibService::GetMediaAssets(string selection, vector<string> selArgs)
{
    vector<unique_ptr<MediaAsset>> mediaAssetList;

    if (mediaLibInstance != nullptr) {
        mediaAssetList = mediaLibInstance->GetMediaAssets(selection, selArgs);
    }
    return mediaAssetList;
}

vector<unique_ptr<ImageAsset>> MediaLibService::GetImageAssets(string selection, vector<string> selArgs)
{
    vector<unique_ptr<ImageAsset>> imageAssetList;

    if (mediaLibInstance != nullptr) {
        imageAssetList = mediaLibInstance->GetImageAssets(selection, selArgs);
    }
    return imageAssetList;
}

vector<unique_ptr<AudioAsset>> MediaLibService::GetAudioAssets(string selection, vector<string> selArgs)
{
    vector<unique_ptr<AudioAsset>> audioAssetList;

    if (mediaLibInstance != nullptr) {
        audioAssetList = mediaLibInstance->GetAudioAssets(selection, selArgs);
    }
    return audioAssetList;
}

vector<unique_ptr<VideoAsset>> MediaLibService::GetVideoAssets(string selection, vector<string> selArgs)
{
    vector<unique_ptr<VideoAsset>> videoAssetList;

    if (mediaLibInstance != nullptr) {
        videoAssetList = mediaLibInstance->GetVideoAssets(selection, selArgs);
    }
    return videoAssetList;
}

vector<unique_ptr<AlbumAsset>> MediaLibService::GetImageAlbumAssets(string selection, vector<string> selArgs)
{
    vector<unique_ptr<AlbumAsset>> albumAsset;
    int32_t requestType = MediaType::MEDIA_TYPE_IMAGE;

    if (mediaLibInstance != nullptr) {
        albumAsset = mediaLibInstance->GetAlbumAssets(selection, selArgs, requestType);
    }
    return albumAsset;
}

vector<unique_ptr<AlbumAsset>> MediaLibService::GetVideoAlbumAssets(string selection, vector<string> selArgs)
{
    vector<unique_ptr<AlbumAsset>> albumAsset;
    int32_t requestType = MediaType::MEDIA_TYPE_VIDEO;

    if (mediaLibInstance != nullptr) {
        albumAsset = mediaLibInstance->GetAlbumAssets(selection, selArgs, requestType);
    }
    return albumAsset;
}

bool MediaLibService::CreateMediaAsset(AssetType assetType, const MediaAsset &mediaAsset)
{
    bool errCode = false;
    MediaAsset *asset = (MediaAsset *)&mediaAsset;

    if (mediaLibInstance != nullptr && (asset != nullptr)) {
        errCode = mediaLibInstance->CreateMediaAsset(assetType, mediaAsset);
    }

    return errCode;
}

bool MediaLibService::DeleteMediaAsset(AssetType assetType, const MediaAsset &mediaAsset)
{
    bool errCode = false;
    MediaAsset *asset = (MediaAsset *)&mediaAsset;

    if (mediaLibInstance != nullptr && (asset != nullptr)) {
        errCode = mediaLibInstance->DeleteMediaAsset(assetType, mediaAsset);
    }

    return errCode;
}

bool MediaLibService::ModifyMediaAsset(AssetType assetType, const MediaAsset &srcMediaAsset,
                                       const MediaAsset &dstMediaAsset)
{
    bool errCode = false;
    MediaAsset *srcAsset = (MediaAsset *)&srcMediaAsset;
    MediaAsset *dstAsset = (MediaAsset *)&dstMediaAsset;

    if (mediaLibInstance != nullptr && (srcAsset != nullptr) && (dstAsset != nullptr)) {
        errCode = mediaLibInstance->ModifyMediaAsset(assetType, srcMediaAsset, dstMediaAsset);
    }

    return errCode;
}

bool MediaLibService::CopyMediaAsset(AssetType assetType, const MediaAsset &srcMediaAsset,
                                     const MediaAsset &dstMediaAsset)
{
    bool errCode = false;
    MediaAsset *srcAsset = (MediaAsset *)&srcMediaAsset;
    MediaAsset *dstAsset = (MediaAsset *)&dstMediaAsset;

    if (mediaLibInstance != nullptr && (srcAsset != nullptr) && (dstAsset != nullptr)) {
        errCode = mediaLibInstance->CopyMediaAsset(assetType, srcMediaAsset, dstMediaAsset);
    }

    return errCode;
}

bool MediaLibService::CreateMediaAlbumAsset(AssetType assetType, const AlbumAsset &albumAsset)
{
    bool errCode = false;
    AlbumAsset *asset = (AlbumAsset *)&albumAsset;

    if (mediaLibInstance != nullptr && (asset != nullptr)) {
        errCode =  mediaLibInstance->CreateMediaAlbumAsset(assetType, albumAsset);
    }

    return errCode;
}

bool MediaLibService::DeleteMediaAlbumAsset(AssetType assetType, const AlbumAsset &albumAsset,
                                            const string &albumUri)
{
    bool errCode = false;
    AlbumAsset *asset = (AlbumAsset *)&albumAsset;

    if (mediaLibInstance != nullptr && (asset != nullptr) && (!albumUri.empty())) {
        errCode = mediaLibInstance->DeleteMediaAlbumAsset(assetType, albumAsset, albumUri);
    }

    return errCode;
}

bool MediaLibService::ModifyMediaAlbumAsset(AssetType assetType, const AlbumAsset &srcAlbumAsset,
                                            const AlbumAsset &dstAlbumAsset, const string &albumUri)
{
    bool errCode = false;
    AlbumAsset *srcAsset = (AlbumAsset *)&srcAlbumAsset;
    AlbumAsset *dstAsset = (AlbumAsset *)&dstAlbumAsset;

    if (mediaLibInstance != nullptr && (srcAsset != nullptr) && (dstAsset != nullptr)  &&
        (!albumUri.empty())) {
        errCode = mediaLibInstance->ModifyMediaAlbumAsset(assetType, srcAlbumAsset, dstAlbumAsset, albumUri);
    }

    return errCode;
}
} // namespace Media
} // namespace OHOS
