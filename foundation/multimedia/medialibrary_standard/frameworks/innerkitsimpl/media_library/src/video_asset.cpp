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

#include "video_asset.h"

using namespace std;

namespace VideoAssetConstants {
    const int32_t DEFAULT_VIDEO_WIDTH = 1280;
    const int32_t DEFAULT_VIDEO_HEIGHT = 720;
    const int32_t DEFAULT_VIDEO_DURATION = 0;
    const string DEFAULT_VIDEO_MIME_TYPE = "video/*";
}

namespace OHOS {
namespace Media {
VideoAsset::VideoAsset()
{
    width_ = VideoAssetConstants::DEFAULT_VIDEO_WIDTH;
    height_ = VideoAssetConstants::DEFAULT_VIDEO_HEIGHT;
    duration_ = VideoAssetConstants::DEFAULT_VIDEO_DURATION;
    mimeType_ = VideoAssetConstants::DEFAULT_VIDEO_MIME_TYPE;
    mediaType_ = MEDIA_TYPE_VIDEO;
}

VideoAsset::~VideoAsset() = default;
}  // namespace Media
}  // namespace OHOS
