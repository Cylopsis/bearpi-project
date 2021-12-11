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

#ifndef HEIF_FORMAT_AGENT_H
#define HEIF_FORMAT_AGENT_H

#include "abs_image_format_agent.h"
#include "hilog/log.h"
#include "log_tags.h"
#include "plugin_class_base.h"

namespace OHOS {
namespace ImagePlugin {
class HeifFormatAgent : public AbsImageFormatAgent, public OHOS::MultimediaPlugin::PluginClassBase {
public:
    std::string GetFormatType() override;
    uint32_t GetHeaderSize() override;
    bool CheckFormat(const void *headerData, uint32_t dataSize) override;

private:
    uint32_t Fourcc(uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4);
    uint32_t EndianSwap32(uint32_t value);
    uint64_t EndianSwap64(uint64_t value);
    bool IsHeif64(const void *buffer, const size_t bytesRead, int64_t &offset, uint64_t &chunkSize);
};
} // namespace ImagePlugin
} // namespace OHOS

#endif // HEIF_FORMAT_AGENT_H