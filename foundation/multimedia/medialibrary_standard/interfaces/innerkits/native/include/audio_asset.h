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

#ifndef AUDIO_ASSET_H
#define AUDIO_ASSET_H

#include "media_asset.h"

namespace OHOS {
namespace Media {
/**
 * @brief Data class for audio file details
 *
 * @since 1.0
 * @version 1.0
 */
class AudioAsset : public MediaAsset {
public:
    AudioAsset();
    virtual ~AudioAsset();

    int32_t duration_;
    std::string title_;
    std::string artist_;
    std::string mimeType_;
};
}  // namespace Media
}  // namespace OHOS
#endif  // AUDIO_ASSET_H
