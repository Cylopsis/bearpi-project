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

#ifndef FILE_SOURCE_STREAM_H
#define FILE_SOURCE_STREAM_H

#include <cstdio>
#include <memory>
#include <string>
#include "image/input_data_stream.h"
#include "source_stream.h"

namespace OHOS {
namespace Media {
class FileSourceStream : public SourceStream {
public:
    static std::unique_ptr<FileSourceStream> CreateSourceStream(const std::string &pathName);
    ~FileSourceStream();

    bool Read(uint32_t desiredSize, ImagePlugin::DataStreamBuffer &outData) override;
    bool Read(uint32_t desiredSize, uint8_t *outBuffer, uint32_t bufferSize, uint32_t &readSize) override;
    bool Peek(uint32_t desiredSize, ImagePlugin::DataStreamBuffer &outData) override;
    bool Peek(uint32_t desiredSize, uint8_t *outBuffer, uint32_t bufferSize, uint32_t &readSize) override;
    uint32_t Tell() override;
    bool Seek(uint32_t position) override;
    size_t GetStreamSize() override;
    uint8_t *GetDataPtr() override;
    uint32_t GetStreamType() override;

private:
    DISALLOW_COPY_AND_MOVE(FileSourceStream);
    FileSourceStream(std::FILE *file, size_t size, size_t offset, size_t original);
    bool GetData(uint32_t desiredSize, uint8_t *outBuffer, uint32_t bufferSize, uint32_t &readSize);
    bool GetData(uint32_t desiredSize, ImagePlugin::DataStreamBuffer &outData);
    void ResetReadBuffer();
    std::FILE *filePtr_ = nullptr;
    size_t fileSize_ = 0;
    size_t fileOffset_ = 0;
    size_t fileOriginalOffset_ = 0;
    uint8_t *readBuffer_ = nullptr;
};
} // namespace Media
} // namespace OHOS

#endif // FILE_SOURCE_STREAM_H