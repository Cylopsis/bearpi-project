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

#ifndef OSTREAM_PACKER_STREAM_H
#define OSTREAM_PACKER_STREAM_H

#include <ostream>
#include "hilog/log.h"
#include "log_tags.h"
#include "nocopyable.h"
#include "packer_stream.h"

namespace OHOS {
namespace Media {
class OstreamPackerStream : public PackerStream {
public:
    explicit OstreamPackerStream(std::ostream &outputStream);
    ~OstreamPackerStream() = default;
    bool Write(const uint8_t *buffer, uint32_t size) override;
    void Flush() override;
    int64_t BytesWritten() override;

private:
    DISALLOW_COPY(OstreamPackerStream);
    static constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {
        LOG_CORE, LOG_TAG_DOMAIN_ID_IMAGE, "OstreamPackerStream"
    };
    std::ostream *outputStream_ = nullptr;
};
} // namespace Media
} // namespace OHOS

#endif // OSTREAM_PACKER_STREAM_H