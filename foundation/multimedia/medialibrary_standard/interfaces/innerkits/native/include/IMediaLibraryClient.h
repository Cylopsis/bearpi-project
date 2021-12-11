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

#ifndef IMEDIALIBRARYCLIENT_H
#define IMEDIALIBRARYCLIENT_H

#include "album_asset.h"
#include "audio_asset.h"
#include "image_asset.h"
#include "media_asset.h"
#include "video_asset.h"

namespace OHOS {
namespace Media {
class IMediaLibraryClient {
public:
    virtual ~IMediaLibraryClient() {}

    /**
     * @brief Returns the Media Library Instance
     *
     * @return Returns the Media Library Instance
     * @since 1.0
     * @version 1.0
     */
    static IMediaLibraryClient *GetMediaLibraryClientInstance();

     /**
     * @brief Gets list of media assets
     *
     * @param selection Selection string
     * @param selectionArgs Selection argument
     * @return Returns vector of media asset unique pointers
     * @since 1.0
     * @version 1.0
     */
    virtual std::vector<std::unique_ptr<MediaAsset>> GetMediaAssets(std::string selection,
        std::vector<std::string> selectionArgs) = 0;

    /**
     * @brief Gets list of audio assets
     *
     * @param selection Selection string
     * @param selectionArgs Selection argument
     * @return Returns vector of audio asset unique pointers
     * @since 1.0
     * @version 1.0
     */
    virtual std::vector<std::unique_ptr<AudioAsset>> GetAudioAssets(std::string selection,
        std::vector<std::string> selectionArgs) = 0;

    /**
     * @brief Gets list of video assets
     *
     * @param selection Selection string
     * @param selectionArgs Selection argument
     * @return Returns vector of video asset unique pointers
     * @since 1.0
     * @version 1.0
     */
    virtual std::vector<std::unique_ptr<VideoAsset>> GetVideoAssets(std::string selection,
        std::vector<std::string> selectionArgs) = 0;

    /**
     * @brief Gets list of image assets
     *
     * @param selection Selection string
     * @param selectionArgs Selection argument
     * @return Returns vector of image asset unique pointers
     * @since 1.0
     * @version 1.0
     */
    virtual std::vector<std::unique_ptr<ImageAsset>> GetImageAssets(std::string selection,
        std::vector<std::string> selectionArgs) = 0;

    /**
     * @brief Gets list of image asset albums
     *
     * @param selection Selection string
     * @param selectionArgs Selection argument
     * @return Returns vector of image asset album unique pointers
     * @since 1.0
     * @version 1.0
     */
    virtual std::vector<std::unique_ptr<AlbumAsset>> GetImageAlbumAssets(std::string selection,
        std::vector<std::string> selectionArgs) = 0;

    /**
     * @brief Gets list of video asset albums
     *
     * @param selection Selection string
     * @param selectionArgs Selection argument
     * @return Returns vector of video asset unique pointers
     * @since 1.0
     * @version 1.0
     */
    virtual std::vector<std::unique_ptr<AlbumAsset>> GetVideoAlbumAssets(std::string selection,
        std::vector<std::string> selectionArgs) = 0;

    /**
     * @brief Creates Media Asset
     *
     * @param assetType enum media assetType
     * @param mediaAsset MediaAsset class object
     * @return Returns status of media asset creation
     * @since 1.0
     * @version 1.0
     */
    virtual bool CreateMediaAsset(AssetType assetType, const MediaAsset &mediaAsset) = 0;

    /**
     * @brief Delete Media Asset
     *
     * @param assetType enum media assetType
     * @param mediaAsset MediaAsset class object
     * @return Returns status of media asset delete
     * @since 1.0
     * @version 1.0
     */
    virtual bool DeleteMediaAsset(AssetType assetType, const MediaAsset &mediaAsset) = 0;

    /**
     * @brief Modify Media Asset
     *
     * @param assetType enum media assetType
     * @param srcMediaAsset  source MediaAsset class object
     * @param dstMediaAsset  destination MediaAsset class object
     * @return Returns status of media asset modification
     * @since 1.0
     * @version 1.0
     */
    virtual bool ModifyMediaAsset(AssetType assetType, const MediaAsset &srcMediaAsset,
                                  const MediaAsset &dstMediaAsset) = 0;

    /**
     * @brief Copy Media Asset
     *
     * @param assetType enum media assetType
     * @param srcMediaAsset  source MediaAsset class object
     * @param dstMediaAsset  destination MediaAsset class object
     * @return Returns status of media asset copy
     * @since 1.0
     * @version 1.0
     */
    virtual bool CopyMediaAsset(AssetType assetType, const MediaAsset &srcMediaAsset,
                                const MediaAsset &dstMediaAsset) = 0;

    /**
     * @brief Creates Media Album Asset
     *
     * @param assetType enum media assetType
     * @param albumAsset AlbumAsset class object
     * @return Returns status of creation media album.
     * @since 1.0
     * @version 1.0
     */
    virtual bool CreateMediaAlbumAsset(AssetType assetType, const AlbumAsset &albumAsset) = 0;

    /**
     * @brief Delete Media Album Asset
     *
     * @param assetType enum media assetType
     * @param albumAsset AlbumAsset class object
     * @return Returns status of deleted media album.
     * @since 1.0
     * @version 1.0
     */
    virtual bool DeleteMediaAlbumAsset(AssetType assetType,  const AlbumAsset &albumAsset,
                                       const std::string &albumUri) = 0;

    /**
     * @brief Modify Media Album Asset
     *
     * @param assetType enum media assetType
     * @param srcAlbumAsset  source AlbumAsset class object
     * @param dstAlbumAsset  destination AlbumAsset class object
     * @return Returns status of media album modification
     * @since 1.0
     * @version 1.0
     */
    virtual bool ModifyMediaAlbumAsset(AssetType assetType, const AlbumAsset &srcAlbumAsset,
                                       const AlbumAsset &dstAlbumAsset, const std::string &albumUri) = 0;
};
} // namespace Media
} // namespace OHOS
#endif // IMEDIALIBRARYCLIENT_H
