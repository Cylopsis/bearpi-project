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

#include "jpeg_format_agent.h"

namespace OHOS {
namespace ImagePlugin {
using namespace OHOS::HiviewDFX;
using namespace MultimediaPlugin;

static const std::string FORMAT_TYPE = "image/jpeg";
static constexpr uint8_t JPEG_HEADER[] = { 0xFF, 0xD8, 0xFF };
static constexpr OHOS::HiviewDFX::HiLogLabel LABEL = { LOG_CORE, LOG_TAG_DOMAIN_ID_PLUGIN, "JpegFormatAgent" };

std::string JpegFormatAgent::GetFormatType()
{
    return FORMAT_TYPE;
}

uint32_t JpegFormatAgent::GetHeaderSize()
{
    return sizeof(JPEG_HEADER);
}

bool JpegFormatAgent::CheckFormat(const void *headerData, uint32_t dataSize)
{
    if (headerData == nullptr) {
        HiLog::Error(LABEL, "check format failed: header data is null.");
        return false;
    }
    uint32_t headerSize = sizeof(JPEG_HEADER);
    if (dataSize < headerSize) {
        HiLog::Error(LABEL, "read head size:[%{public}u] less than header size:[%{public}u].", dataSize, headerSize);
        return false;
    }

    if (memcmp(headerData, JPEG_HEADER, headerSize) != 0) {
        HiLog::Error(LABEL, "header stamp mismatch.");
        return false;
    }
    return true;
}
} // namespace ImagePlugin
} // namespace OHOS
