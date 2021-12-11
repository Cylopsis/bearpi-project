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


#include "raw_format_agent.h"

namespace OHOS {
namespace ImagePlugin {
using namespace OHOS::HiviewDFX;
using namespace ImagePlugin;
using namespace MultimediaPlugin;

/*
"image/x-sony-arw",
"image/x-canon-cr2",
"image/x-adobe-dng",
"image/x-nikon-nef",
"image/x-nikon-nrw",
"image/x-olympus-orf",
"image/x-fuji-raf",
"image/x-panasonic-rw2",
"image/x-pentax-pef",
"image/x-samsung-srw",
*/
const std::string FORMAT_TYPE = "image/x-raw";
constexpr uint32_t HEADER_SIZE = 32;
static constexpr OHOS::HiviewDFX::HiLogLabel LABEL = { LOG_CORE, LOG_TAG_DOMAIN_ID_PLUGIN, "RawFormatAgent" };

std::string RawFormatAgent::GetFormatType()
{
    return FORMAT_TYPE;
}

uint32_t RawFormatAgent::GetHeaderSize()
{
    return HEADER_SIZE;
}

bool RawFormatAgent::CheckFormat(const void *headerData, uint32_t dataSize)
{
    if (headerData == nullptr) {
        HiLog::Error(LABEL, "check format failed: header data is null.");
        return false;
    }

    HiLog::Info(LABEL, "RawFormatAgent now pass all image format.");
    return true;
}
} // namespace ImagePlugin
} // namespace OHOS
