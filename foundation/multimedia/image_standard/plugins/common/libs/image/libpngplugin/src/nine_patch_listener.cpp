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

#include "nine_patch_listener.h"
#include <log_tags.h>
#include <cmath>
#include "hilog/log.h"
#ifndef _WIN32
#include "securec.h"
#else
#include "memory.h"
#endif

namespace OHOS {
namespace ImagePlugin {
using namespace OHOS::HiviewDFX;

namespace {
constexpr HiLogLabel LABEL = { LOG_CORE, LOG_TAG_DOMAIN_ID_PLUGIN, "NinePatchListener" };
const std::string CHUNK_NAME = "npTc";
constexpr float FLOAT_NEAR_ZERO = (1.0f / (1 << 12));
constexpr float FHALF = 0.5f;
constexpr float NO_SCALE = 1.0f;
} // namespace

static void ScaleDivRange(int32_t *divs, int32_t count, float scale, int32_t maxValue)
{
    for (int i = 0; i < count; i++) {
        divs[i] = static_cast<int32_t>(divs[i] * scale + FHALF);
        if (i > 0 && divs[i] == divs[i - 1]) {
            divs[i]++;
        }
    }

    if (divs[count - 1] > maxValue) {
        int highestAvailable = maxValue;
        for (int i = count - 1; i >= 0; i--) {
            divs[i] = highestAvailable;
            if (i > 0 && divs[i] <= divs[i - 1]) {
                highestAvailable = divs[i] - 1;
            } else {
                break;
            }
        }
    }
}

bool NinePatchListener::ReadChunk(const std::string &tag, void *data, size_t length)
{
    if (tag == CHUNK_NAME && length >= sizeof(PngNinePatchRes)) {
        if (data == nullptr) {
            HiLog::Error(LABEL, "data is null");
            return false;
        }
        PngNinePatchRes *patch = static_cast<PngNinePatchRes *>(data);
        size_t patchSize = patch->SerializedSize();
        if (length != patchSize) {
            HiLog::Error(LABEL, "length(%{public}zu) ne patchSize(%{public}zu)", length, patchSize);
            return false;
        }
        // copy the data because it is owned by the png reader
        PngNinePatchRes *patchNew = static_cast<PngNinePatchRes *>(malloc(patchSize));
        if (patchNew == nullptr) {
            HiLog::Error(LABEL, "malloc failed");
            return false;
        }
        errno_t err = memcpy_s(patchNew, patchSize, patch, patchSize);
        if (err != 0) {
            HiLog::Error(LABEL, "memcpy failed. errno:%{public}d", err);
            free(patchNew);
            patchNew = nullptr;
            return false;
        }
        PngNinePatchRes::Deserialize(patchNew);
        patchNew->FileToDevice();
        if (patch_ != nullptr) {
            free(patch_);
        }
        patch_ = patchNew;
        patchSize_ = patchSize;
    }

    return true;
}

void NinePatchListener::Scale(float scaleX, float scaleY, int32_t scaledWidth, int32_t scaledHeight)
{
    if (patch_ == nullptr) {
        HiLog::Error(LABEL, "patch is null");
        return;
    }

    if (fabsf(scaleX - NO_SCALE) > FLOAT_NEAR_ZERO) {
        patch_->paddingLeft = static_cast<int32_t>(patch_->paddingLeft * scaleX + FHALF);
        patch_->paddingRight = static_cast<int32_t>(patch_->paddingRight * scaleX + FHALF);
        ScaleDivRange(patch_->GetXDivs(), patch_->numXDivs, scaleX, scaledWidth - 1);
    }

    if (fabsf(scaleY - NO_SCALE) > FLOAT_NEAR_ZERO) {
        patch_->paddingTop = static_cast<int32_t>(patch_->paddingTop * scaleY + FHALF);
        patch_->paddingBottom = static_cast<int32_t>(patch_->paddingBottom * scaleY + FHALF);
        ScaleDivRange(patch_->GetYDivs(), patch_->numYDivs, scaleY, scaledHeight - 1);
    }
}
} // namespace ImagePlugin
} // namespace OHOS
