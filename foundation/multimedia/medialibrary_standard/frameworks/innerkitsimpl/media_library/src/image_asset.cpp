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

#include "image_asset.h"

using namespace std;

namespace ImageAssetConstants {
    const int32_t DEFAULT_IMAGE_WIDTH = 1280;
    const int32_t DEFAULT_IMAGE_HEIGHT = 720;
    const string DEFAULT_IMAGE_MIME_TYPE = "image/*";
}

namespace OHOS {
namespace Media {
ImageAsset::ImageAsset()
{
    width_ = ImageAssetConstants::DEFAULT_IMAGE_WIDTH;
    height_ = ImageAssetConstants::DEFAULT_IMAGE_HEIGHT;
    mimeType_ = ImageAssetConstants::DEFAULT_IMAGE_MIME_TYPE;
    mediaType_ = MEDIA_TYPE_IMAGE;
}

ImageAsset::~ImageAsset() = default;
}  // namespace Media
}  // namespace OHOS
