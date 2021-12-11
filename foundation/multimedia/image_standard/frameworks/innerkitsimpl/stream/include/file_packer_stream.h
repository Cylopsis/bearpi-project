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

#ifndef FILE_PACKER_STREAM_H
#define FILE_PACKER_STREAM_H

#include <fstream>
#include "hilog/log.h"
#include "log_tags.h"
#include "nocopyable.h"
#include "packer_stream.h"

namespace OHOS {
namespace Media {
class FilePackerStream : public PackerStream {
public:
    explicit FilePackerStream(const std::string &filePath);
    ~FilePackerStream() override;
    bool Write(const uint8_t *buffer, uint32_t size) override;
    void Flush() override;
    int64_t BytesWritten() override;

private:
    DISALLOW_COPY(FilePackerStream);
    static constexpr OHOS::HiviewDFX::HiLogLabel LABEL = { LOG_CORE, LOG_TAG_DOMAIN_ID_IMAGE, "FilePackerStream" };
    FILE *file_ = nullptr;
};
} // namespace Media
} // namespace OHOS

#endif // FILE_PACKER_STREAM_H