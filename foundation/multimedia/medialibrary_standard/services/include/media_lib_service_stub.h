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

#ifndef MEDIA_LIB_SERVICE_STUB_H
#define MEDIA_LIB_SERVICE_STUB_H

#include "ipc_types.h"
#include "iremote_broker.h"
#include "iremote_proxy.h"
#include "iremote_stub.h"
#include "media_library.h"

namespace OHOS {
namespace Media {
class IMediaLibService : public IRemoteBroker {
public:
    virtual std::vector<std::unique_ptr<MediaAsset>> GetMediaAssets(std::string selection,
        std::vector<std::string> selectionArgs) = 0;

    virtual std::vector<std::unique_ptr<ImageAsset>> GetImageAssets(std::string selection,
        std::vector<std::string> selectionArgs) = 0;

    virtual std::vector<std::unique_ptr<AudioAsset>> GetAudioAssets(std::string selection,
        std::vector<std::string> selectionArgs) = 0;

    virtual std::vector<std::unique_ptr<VideoAsset>> GetVideoAssets(std::string selection,
        std::vector<std::string> selectionArgs) = 0;

    virtual std::vector<std::unique_ptr<AlbumAsset>> GetImageAlbumAssets(std::string selection,
        std::vector<std::string> selectionArgs) = 0;

    virtual std::vector<std::unique_ptr<AlbumAsset>> GetVideoAlbumAssets(std::string selection,
        std::vector<std::string> selectionArgs) = 0;

    virtual bool CreateMediaAsset(AssetType assetType, const MediaAsset &mediaAsset) = 0;

    virtual bool DeleteMediaAsset(AssetType assetType, const MediaAsset &mediaAsset) = 0;

    virtual bool ModifyMediaAsset(AssetType assetType, const MediaAsset &srcMediaAsset,
                                  const MediaAsset &dstMediaAsset) = 0;

    virtual bool CopyMediaAsset(AssetType assetType, const MediaAsset &srcMediaAsset,
                                const MediaAsset &dstMediaAsset) = 0;

    virtual bool CreateMediaAlbumAsset(AssetType assetType, const AlbumAsset &albumAsset) = 0;

    virtual bool DeleteMediaAlbumAsset(AssetType assetType, const AlbumAsset &albumAsset,
                                       const std::string &albumUri) = 0;

    virtual bool ModifyMediaAlbumAsset(AssetType assetType, const AlbumAsset &srcAlbumAsset,
                                       const AlbumAsset &dstAlbumAsset, const std::string &albumUri) = 0;

    DECLARE_INTERFACE_DESCRIPTOR(u"IMediaLibService");
};

class MediaLibServiceStub : public IRemoteStub<IMediaLibService> {
public:
    virtual int OnRemoteRequest(uint32_t code, MessageParcel &data,
                                MessageParcel &reply, MessageOption &option) override;

    int ProcessMediaOperationRequests(uint32_t code, MessageParcel &data,
                                  MessageParcel &reply, MessageOption &option);

    int32_t GetVideoAlbumAssetsOnRequest(MessageParcel &data, MessageParcel &reply);

    int32_t GetImageAlbumAssetsOnRequest(MessageParcel &data, MessageParcel &reply);

    int32_t GetVideoAssetsOnRequest(MessageParcel &data, MessageParcel &reply);

    int32_t GetAudioAssetsOnRequest(MessageParcel &data, MessageParcel &reply);

    int32_t GetMediaAssetsOnRequest(MessageParcel &data, MessageParcel &reply);

    int32_t GetImageAssetsOnRequest(MessageParcel &data, MessageParcel &reply);

    bool CreateMediaOnRequest(MessageParcel &data, MessageParcel &reply);

    bool DeleteMediaOnRequest(MessageParcel &data, MessageParcel &reply);

    bool ModifyMediaOnRequest(MessageParcel &data, MessageParcel &reply);

    bool CopyMediaOnRequest(MessageParcel &data, MessageParcel &reply);

    bool CreateAlbumMediaOnRequest(MessageParcel &data, MessageParcel &reply);

    bool DeleteAlbumMediaOnRequest(MessageParcel &data, MessageParcel &reply);

    bool ModifyAlbumMediaOnRequest(MessageParcel &data, MessageParcel &reply);

    int32_t WriteCommonData(const MediaAsset &asset, const MessageParcel &reply);

    int32_t WriteImageList(std::vector<std::unique_ptr<ImageAsset>> &imageAssetList,
                MessageParcel &reply);

    int32_t WriteAudioList(std::vector<std::unique_ptr<AudioAsset>> &audioAssetList,
                MessageParcel &reply);

    int32_t WriteVideoList(std::vector<std::unique_ptr<VideoAsset>> &videoAssetList,
                MessageParcel &reply);

    int32_t WriteMediaList(std::vector<std::unique_ptr<MediaAsset>> &mediaAssetList,
                MessageParcel &reply);

    int32_t WriteImageAlbum(std::vector<std::unique_ptr<AlbumAsset>> &albumAssetList,
                MessageParcel &reply);

    int32_t WriteVideoAlbum(std::vector<std::unique_ptr<AlbumAsset>> &albumAssetList,
                MessageParcel &reply);
};
} // namespace Media
} // namespace OHOS
#endif // MEDIA_LIB_SERVICE_STUB_H
