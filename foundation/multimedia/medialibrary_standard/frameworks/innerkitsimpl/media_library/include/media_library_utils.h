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

#ifndef MEDIA_LIBRARY_UTILS_H
#define MEDIA_LIBRARY_UTILS_H

#include <pthread.h>
#include "album_asset.h"
#include "audio_asset.h"
#include "image_asset.h"
#include "media_asset.h"
#include "video_asset.h"

namespace OHOS {
namespace Media {
/**
 * @brief Utility class for media library
 *
 * @since 1.0
 * @version 1.0
 */
class MediaLibraryUtils {
public:
    // Static variable to save the media type request by application
    static MediaType requestedMediaType;
    // Declaration of thread condition variable and mutes
    static pthread_mutex_t mutexLock;

    static std::vector<std::unique_ptr<AudioAsset>>& GetAudioAssetsInternal();
    static std::vector<std::unique_ptr<ImageAsset>>& GetImageAssetsInternal();
    static std::vector<std::unique_ptr<MediaAsset>>& GetMediaAssetsInternal();
    static std::vector<std::unique_ptr<VideoAsset>>& GetVideoAssetsInternal();
};
}  // namespace Media
}  // namespace OHOS
#endif  // MEDIA_LIBRARY_UTILS_H