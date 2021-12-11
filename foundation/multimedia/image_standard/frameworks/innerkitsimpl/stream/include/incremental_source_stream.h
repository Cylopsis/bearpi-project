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

#ifndef INCREMENTAL_SOURCE_STREAM_H
#define INCREMENTAL_SOURCE_STREAM_H

#include <cstdint>
#include <memory>
#include <vector>
#include "image/input_data_stream.h"
#include "image_type.h"
#include "source_stream.h"

namespace OHOS {
namespace Media {
class IncrementalSourceStream : public SourceStream {
public:
    static std::unique_ptr<IncrementalSourceStream> CreateSourceStream(IncrementalMode mode);
    ~IncrementalSourceStream() = default;
    bool Read(uint32_t desiredSize, ImagePlugin::DataStreamBuffer &outData) override;
    bool Read(uint32_t desiredSize, uint8_t *outBuffer, uint32_t bufferSize, uint32_t &readSize) override;
    bool Peek(uint32_t desiredSize, ImagePlugin::DataStreamBuffer &outData) override;
    bool Peek(uint32_t desiredSize, uint8_t *outBuffer, uint32_t bufferSize, uint32_t &readSize) override;
    uint32_t Tell() override;
    bool Seek(uint32_t position) override;

    uint32_t UpdateData(const uint8_t *data, uint32_t size, bool isCompleted) override;
    bool IsStreamCompleted() override;
    size_t GetStreamSize() override;

private:
    explicit IncrementalSourceStream(IncrementalMode mode);
    IncrementalMode incrementalMode_;
    bool isFinalize_;
    std::vector<uint8_t> sourceData_;
    size_t dataSize_ = 0;
    size_t dataOffset_ = 0;
};
} // namespace Media
} // namespace OHOS

#endif // ZINCREMENTAL_INPUT_STREAM_H
