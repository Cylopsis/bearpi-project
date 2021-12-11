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

#include <iostream>
#include "hilog/log.h"
#include "image_source.h"
#include "incremental_pixel_map.h"
#include "log_tags.h"
#include "media_errors.h"

namespace OHOS {
namespace Media {
using namespace OHOS::HiviewDFX;

static constexpr OHOS::HiviewDFX::HiLogLabel LABEL = { LOG_CORE, LOG_TAG_DOMAIN_ID_IMAGE, "IncrementalPixelMap" };

static IncrementalDecodingState ConvertImageStateToIncrementalState(ImageDecodingState imageState)
{
    switch (imageState) {
        case ImageDecodingState::UNRESOLVED: {
            return IncrementalDecodingState::UNRESOLVED;
        }
        case ImageDecodingState::BASE_INFO_ERROR: {
            return IncrementalDecodingState::BASE_INFO_ERROR;
        }
        case ImageDecodingState::BASE_INFO_PARSED: {
            return IncrementalDecodingState::BASE_INFO_PARSED;
        }
        case ImageDecodingState::IMAGE_DECODING: {
            return IncrementalDecodingState::IMAGE_DECODING;
        }
        case ImageDecodingState::IMAGE_ERROR: {
            return IncrementalDecodingState::IMAGE_ERROR;
        }
        case ImageDecodingState::PARTIAL_IMAGE: {
            return IncrementalDecodingState::PARTIAL_IMAGE;
        }
        case ImageDecodingState::IMAGE_DECODED: {
            return IncrementalDecodingState::IMAGE_DECODED;
        }
        default: {
            HiLog::Error(LABEL, "unexpected imageState %{public}d.", imageState);
            return IncrementalDecodingState::UNRESOLVED;
        }
    }
}

IncrementalPixelMap::~IncrementalPixelMap()
{
    if (imageSource_ == nullptr) {
        return;
    }
    DetachSource();
}

IncrementalPixelMap::IncrementalPixelMap(uint32_t index, const DecodeOptions opts, ImageSource *imageSource)
    : index_(index), opts_(opts), imageSource_(imageSource)
{
    if (imageSource_ != nullptr) {
        imageSource_->RegisterListener(static_cast<PeerListener *>(this));
    }
}

uint32_t IncrementalPixelMap::PromoteDecoding(uint8_t &decodeProgress)
{
    if (imageSource_ == nullptr) {
        if (decodingStatus_.state == IncrementalDecodingState::BASE_INFO_ERROR ||
            decodingStatus_.state == IncrementalDecodingState::IMAGE_ERROR) {
            HiLog::Error(LABEL, "promote decode failed for state %{public}d, errorDetail %{public}u.",
                         decodingStatus_.state, decodingStatus_.errorDetail);
            return decodingStatus_.errorDetail;
        }
        HiLog::Error(LABEL, "promote decode failed or terminated, image source is null.");
        return ERR_IMAGE_SOURCE_DATA;
    }
    ImageDecodingState imageState = ImageDecodingState::UNRESOLVED;
    uint32_t ret =
        imageSource_->PromoteDecoding(index_, opts_, *(static_cast<PixelMap *>(this)), imageState, decodeProgress);
    decodingStatus_.state = ConvertImageStateToIncrementalState(imageState);
    if (decodeProgress > decodingStatus_.decodingProgress) {
        decodingStatus_.decodingProgress = decodeProgress;
    }
    if (ret != SUCCESS && ret != ERR_IMAGE_SOURCE_DATA_INCOMPLETE) {
        DetachSource();
        decodingStatus_.errorDetail = ret;
        HiLog::Error(LABEL, "promote decode failed, ret=%{public}u.", ret);
    }
    if (ret == SUCCESS) {
        DetachSource();
    }
    return ret;
}

void IncrementalPixelMap::DetachFromDecoding()
{
    if (imageSource_ == nullptr) {
        return;
    }
    DetachSource();
}

const IncrementalDecodingStatus &IncrementalPixelMap::GetDecodingStatus()
{
    return decodingStatus_;
}

void IncrementalPixelMap::OnPeerDestory()
{
    imageSource_ = nullptr;
}

void IncrementalPixelMap::DetachSource()
{
    imageSource_->DetachIncrementalDecoding(*(static_cast<PixelMap *>(this)));
    imageSource_->UnRegisterListener(this);
    imageSource_ = nullptr;
}
} // namespace Media
} // namespace OHOS