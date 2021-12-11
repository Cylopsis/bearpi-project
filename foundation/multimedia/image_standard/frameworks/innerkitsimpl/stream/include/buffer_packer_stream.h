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

#ifndef BUFFER_PACKER_STREAM_H
#define BUFFER_PACKER_STREAM_H

#include <string>
#include "hilog/log.h"
#include "log_tags.h"
#include "nocopyable.h"
#include "packer_stream.h"

namespace OHOS {
namespace Media {
class BufferPackerStream : public PackerStream {
public:
    BufferPackerStream(uint8_t *outputData, uint32_t maxSize);
    ~BufferPackerStream() = default;
    bool Write(const uint8_t *buffer, uint32_t size) override;
    int64_t BytesWritten() override;

private:
    DISALLOW_COPY(BufferPackerStream);
    static constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {
        LOG_CORE, LOG_TAG_DOMAIN_ID_IMAGE, "BufferPackerStream"
    };
    uint8_t *outputData_ = nullptr;
    uint32_t maxSize_ = 0;
    int64_t offset_ = 0;
};
} // namespace Media
} // namespace OHOS

#endif // BUFFER_PACKER_STREAM_H