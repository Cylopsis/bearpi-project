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

#ifndef MEDIA_LIBRARY_H
#define MEDIA_LIBRARY_H

#include "album_asset.h"
#include "audio_asset.h"
#include "image_asset.h"
#include "media_asset.h"
#include "video_asset.h"

namespace OHOS {
namespace Media {
/** Media scan operation success */
const int32_t MEDIA_SCAN_SUCCESS = 0;
/** Media scan operation failed */
const int32_t MEDIA_SCAN_FAIL = -1;

/**
 * @brief Media library class
 *
 * @since 1.0
 * @version 1.0
 */
class MediaLibrary {
public:
    MediaLibrary();
    virtual ~MediaLibrary();

    /**
     * Creates an instance for media livrary.
     *
     * @return a new Media Library unique pointer.
     */
    static std::unique_ptr<MediaLibrary> GetMediaLibraryInstance();

    /**
     * @brief Gets list of media assets
     *
     * @param selection Selection string
     * @param selectionArgs Selection argument
     * @return Returns vector of media asset unique pointers
     * @since 1.0
     * @version 1.0
     */
    std::vector<std::unique_ptr<MediaAsset>> GetMediaAssets(std::string selection,
                                                        std::vector<std::string> selectionArgs);

    /**
     * @brief Gets list of audio assets
     *
     * @param selection Selection string
     * @param selectionArgs Selection argument
     * @return Returns vector of audio asset unique pointers
     * @since 1.0
     * @version 1.0
     */
    std::vector<std::unique_ptr<AudioAsset>> GetAudioAssets(std::string selection,
                                                       std::vector<std::string> selectionArgs);

    /**
     * @brief Gets list of video assets
     *
     * @param selection Selection string
     * @param selectionArgs Selection argument
     * @return Returns vector of video asset unique pointers
     * @since 1.0
     * @version 1.0
     */
    std::vector<std::unique_ptr<VideoAsset>> GetVideoAssets(std::string selection,
                                                       std::vector<std::string> selectionArgs);

    /**
     * @brief Gets list of image assets
     *
     * @param selection Selection string
     * @param selectionArgs Selection argument
     * @return Returns vector of image asset unique pointers
     * @since 1.0
     * @version 1.0
     */
    std::vector<std::unique_ptr<ImageAsset>> GetImageAssets(std::string selection,
                                                       std::vector<std::string> selectionArgs);

    /**
     * @brief Gets list of image album assets
     *
     * @param selection Selection string
     * @param selectionArgs Selection argument
     * @return Returns vector of image asset album unique pointers
     * @since 1.0
     * @version 1.0
     */
    std::vector<std::unique_ptr<AlbumAsset>> GetAlbumAssets(std::string selection,
                                                            std::vector<std::string> selectionArgs,
                                                            int32_t requestType);

    /**
     * @brief Creates Media Asset
     *
     * @param assetType enum media assetType
     * @param mediaAsset MediaAsset class object
     * @return Returns status of creation
     * @since 1.0
     * @version 1.0
     */
    bool CreateMediaAsset(AssetType assetType, const MediaAsset& mediaAsset);

    /**
     * @brief Delete Media Asset
     *
     * @param assetType enum media assetType
     * @param mediaAsset MediaAsset class object
     * @return Returns status of delete
     * @since 1.0
     * @version 1.0
     */
    bool DeleteMediaAsset(AssetType assetType, const MediaAsset& mediaAsset);

    /**
     * @brief Modify Media Asset
     *
     * @param assetType enum media assetType
     * @param srcMediaAsset  source MediaAsset class object
     * @param dstMediaAsset  destination MediaAsset class object
     * @return Returns status of file modification
     * @since 1.0
     * @version 1.0
     */
    bool ModifyMediaAsset(AssetType assetType, const MediaAsset &srcMediaAsset,
                          const MediaAsset &dstMediaAsset);

    /**
     * @brief Copy Media Asset
     *
     * @param assetType enum media assetType
     * @param srcMediaAsset  source MediaAsset class object
     * @param dstMediaAsset  destination MediaAsset class object
     * @return Returns status of media copy
     * @since 1.0
     * @version 1.0
     */
    bool CopyMediaAsset(AssetType assetType, const MediaAsset& srcMediaAsset,
                        const MediaAsset& dstMediaAsset);

    /**
     * @brief Creates Media Album Asset
     *
     * @param assetType enum media assetType
     * @param albumAsset AlbumAsset class object
     * @return Returns status of creation media album.
     * @since 1.0
     * @version 1.0
     */
    bool CreateMediaAlbumAsset(AssetType assetType, const AlbumAsset& albumAsset);

    /**
     * @brief Delete Media Album Asset
     *
     * @param assetType enum media assetType
     * @param albumAsset AlbumAsset class object
     * @return Returns status of deleted media album.
     * @since 1.0
     * @version 1.0
     */
    bool DeleteMediaAlbumAsset(AssetType assetType, const AlbumAsset& albumAsset,
                               const std::string &albumUri);

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
    bool ModifyMediaAlbumAsset(AssetType assetType, const AlbumAsset& srcAlbumAsset,
                               const AlbumAsset &dstAlbumAsset, const std::string &albumUri);
};
}  // namespace Media
}  // namespace OHOS
#endif  // MEDIA_LIBRARY_H
