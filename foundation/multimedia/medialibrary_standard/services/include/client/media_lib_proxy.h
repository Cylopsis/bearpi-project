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

#ifndef MEDIA_LIB_PROXY_H
#define MEDIA_LIB_PROXY_H

#include "iremote_proxy.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

#include "IMediaLibraryClient.h"
#include "media_lib_service_const.h"
#include "media_lib_service_stub.h"

namespace OHOS {
namespace Media {
class MediaLibProxy : public IRemoteProxy<IMediaLibService>, public IMediaLibraryClient {
public:
    explicit MediaLibProxy(const sptr<IRemoteObject> &impl);

    virtual ~MediaLibProxy() = default;

    static IMediaLibraryClient *GetMediaLibraryClientInstance();

    std::vector<std::unique_ptr<MediaAsset>> GetMediaAssets(std::string selection,
        std::vector<std::string> selectionArgs) override;

    std::vector<std::unique_ptr<AudioAsset>> GetAudioAssets(std::string selection,
        std::vector<std::string> selectionArgs) override;

    std::vector<std::unique_ptr<VideoAsset>> GetVideoAssets(std::string selection,
        std::vector<std::string> selectionArgs) override;

    std::vector<std::unique_ptr<ImageAsset>> GetImageAssets(std::string selection,
        std::vector<std::string> selectionArgs) override;

    std::vector<std::unique_ptr<AlbumAsset>> GetImageAlbumAssets(std::string selection,
        std::vector<std::string> selectionArgs) override;

    std::vector<std::unique_ptr<AlbumAsset>> GetVideoAlbumAssets(std::string selection,
        std::vector<std::string> selectionArgs) override;

    bool CreateMediaAsset(AssetType assetType, const MediaAsset &mediaAsset) override;

    bool DeleteMediaAsset(AssetType assetType, const MediaAsset &mediaAsset) override;

    bool ModifyMediaAsset(AssetType assetType, const MediaAsset &srcMediaAsset,
                          const MediaAsset &dstMediaAsset) override;

    bool CopyMediaAsset(AssetType assetType, const MediaAsset &srcMediaAsset,
                        const MediaAsset &dstMediaAsset) override;

    bool CreateMediaAlbumAsset(AssetType assetType, const AlbumAsset &albumAsset) override;

    bool DeleteMediaAlbumAsset(AssetType assetType,  const AlbumAsset &albumAsset,
                               const std::string &albumUri) override;

    bool ModifyMediaAlbumAsset(AssetType assetType, const AlbumAsset &srcAlbumAsset,
                               const AlbumAsset &dstAlbumAsset, const std::string &albumUri) override;

private:
    static inline BrokerDelegator<MediaLibProxy> delegator_;
};
} // namespace Media
} // namespace OHOS
#endif // MEDIA_LIB_PROXY_H
