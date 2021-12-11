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

#ifndef MEDIA_ASSET_H
#define MEDIA_ASSET_H

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <stdint.h>
#include <string>

#include "media_lib_service_const.h"

namespace OHOS {
namespace Media {
/**
 * @brief Data class for media file details
 *
 * @since 1.0
 * @version 1.0
 */
class MediaAsset {
public:
    MediaAsset();
    virtual ~MediaAsset();
    bool CreateMediaAsset(AssetType assetType);
    bool DeleteMediaAsset();
    bool ModifyMediaAsset(const MediaAsset &mediaAsset);
    bool CopyMediaAsset(const MediaAsset &mediaAsset);
    static MediaType GetMediaType(const std::string &filePath);

    int32_t id_;
    uint64_t size_;
    int32_t albumId_;
    std::string albumName_;
    std::string uri_;
    MediaType mediaType_;
    std::string name_;
    uint64_t dateAdded_;
    uint64_t dateModified_;
};
}  // namespace Media
}  // namespace OHOS
#endif  // MEDIA_ASSET_H
