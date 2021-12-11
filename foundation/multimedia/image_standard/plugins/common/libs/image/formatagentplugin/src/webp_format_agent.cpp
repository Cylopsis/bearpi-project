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

#include "webp_format_agent.h"
#include <cstring>

namespace OHOS {
namespace ImagePlugin {
using namespace OHOS::HiviewDFX;
using namespace MultimediaPlugin;

namespace {
constexpr HiLogLabel LABEL = { LOG_CORE, LOG_TAG_DOMAIN_ID_PLUGIN, "WebpFormatAgent" };
const std::string FORMAT_TYPE = "image/webp";
constexpr size_t WEBP_MINIMUM_LENGTH = 14;
const char *WEBP_HEADER_PRE = "RIFF";
constexpr int32_t WEBP_HEADER_PRE_LENGTH = 4;
const char *WEBP_HEADER_POST = "WEBPVP";
constexpr int32_t WEBP_HEADER_POST_LENGTH = 6;
} // namespace

std::string WebpFormatAgent::GetFormatType()
{
    return FORMAT_TYPE;
}

uint32_t WebpFormatAgent::GetHeaderSize()
{
    return WEBP_MINIMUM_LENGTH;
}

bool WebpFormatAgent::CheckFormat(const void *headerData, uint32_t dataSize)
{
    if (headerData == nullptr || dataSize == 0) {
        HiLog::Error(LABEL, "check format input parameter abnormal.");
        return false;
    }

    /*
     * WEBP starts with the following:
     * RIFFXXXXWEBPVP
     * Where XXXX is unspecified.
     */
    const char *head = static_cast<const char *>(headerData);
    return dataSize >= WEBP_MINIMUM_LENGTH && !memcmp(head, WEBP_HEADER_PRE, WEBP_HEADER_PRE_LENGTH) &&
           !memcmp(&head[8], WEBP_HEADER_POST, WEBP_HEADER_POST_LENGTH);
}
} // namespace ImagePlugin
} // namespace OHOS
