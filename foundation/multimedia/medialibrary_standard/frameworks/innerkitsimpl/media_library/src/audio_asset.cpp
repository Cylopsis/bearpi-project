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

#include "audio_asset.h"

using namespace std;

namespace AudioAssetConstants {
    const int32_t DEFAULT_AUDIO_DURATION = 0;
    const string DEFAULT_AUDIO_TITLE = "Unknown";
    const string DEFAULT_AUDIO_ARTIST = "Unknown";
    const string DEFAULT_AUDIO_MIME_TYPE = "audio/*";
}

namespace OHOS {
namespace Media {
AudioAsset::AudioAsset()
{
    duration_ = AudioAssetConstants::DEFAULT_AUDIO_DURATION;
    title_ = AudioAssetConstants::DEFAULT_AUDIO_TITLE;
    artist_ = AudioAssetConstants::DEFAULT_AUDIO_ARTIST;
    mimeType_ = AudioAssetConstants::DEFAULT_AUDIO_MIME_TYPE;
    mediaType_ = MEDIA_TYPE_AUDIO;
}

AudioAsset::~AudioAsset() = default;
}  // namespace Media
}  // namespace OHOS
