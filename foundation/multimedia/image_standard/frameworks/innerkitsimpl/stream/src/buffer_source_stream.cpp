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

#include "buffer_source_stream.h"

#include <string>
#include "image_log.h"
#ifndef _WIN32
#include "securec.h"
#else
#include "memory.h"
#endif

namespace OHOS {
namespace Media {
using namespace OHOS::HiviewDFX;
using namespace std;
using namespace ImagePlugin;

BufferSourceStream::BufferSourceStream(uint8_t *data, uint32_t size, uint32_t offset)
    : inputBuffer_(data), dataSize_(size), dataOffset_(offset)
{}

BufferSourceStream::~BufferSourceStream()
{
    if (inputBuffer_ != nullptr) {
        free(inputBuffer_);
        inputBuffer_ = nullptr;
    }
}

std::unique_ptr<BufferSourceStream> BufferSourceStream::CreateSourceStream(const uint8_t *data, uint32_t size)
{
    if ((data == nullptr) || (size == 0)) {
        IMAGE_LOGE("[BufferSourceStream]input the parameter exception.");
        return nullptr;
    }
    uint8_t *dataCopy = static_cast<uint8_t *>(malloc(size));
    if (dataCopy == nullptr) {
        IMAGE_LOGE("[BufferSourceStream]malloc the input data buffer fail.");
        return nullptr;
    }
    errno_t ret = memcpy_s(dataCopy, size, data, size);
    if (ret != 0) {
        free(dataCopy);
        dataCopy = nullptr;
        IMAGE_LOGE("[BufferSourceStream]copy the input data fail, ret:%{public}d.", ret);
        return nullptr;
    }
    return (unique_ptr<BufferSourceStream>(new BufferSourceStream(dataCopy, size, 0)));
}

bool BufferSourceStream::Read(uint32_t desiredSize, DataStreamBuffer &outData)
{
    if (!Peek(desiredSize, outData)) {
        IMAGE_LOGE("[BufferSourceStream]read fail.");
        return false;
    }
    dataOffset_ += outData.dataSize;
    return true;
}

bool BufferSourceStream::Peek(uint32_t desiredSize, DataStreamBuffer &outData)
{
    if (desiredSize == 0) {
        IMAGE_LOGE("[BufferSourceStream]input the parameter exception.");
        return false;
    }
    if (dataSize_ == dataOffset_) {
        IMAGE_LOGE("[BufferSourceStream]buffer read finish, offset:%{public}zu ,dataSize%{public}zu.", dataOffset_,
                   dataSize_);
        return false;
    }
    outData.bufferSize = dataSize_ - dataOffset_;
    if (desiredSize > dataSize_ - dataOffset_) {
        desiredSize = dataSize_ - dataOffset_;
    }
    outData.dataSize = desiredSize;
    outData.inputStreamBuffer = inputBuffer_ + dataOffset_;
    IMAGE_LOGD("[BufferSourceStream]Peek end. desiredSize:%{public}d, offset:%{public}zu ,dataSize%{public}zu.",
               desiredSize, dataOffset_, dataSize_);
    return true;
}

bool BufferSourceStream::Read(uint32_t desiredSize, uint8_t *outBuffer, uint32_t bufferSize, uint32_t &readSize)
{
    if (!Peek(desiredSize, outBuffer, bufferSize, readSize)) {
        IMAGE_LOGE("[BufferSourceStream]read fail.");
        return false;
    }
    dataOffset_ += readSize;
    return true;
}

bool BufferSourceStream::Peek(uint32_t desiredSize, uint8_t *outBuffer, uint32_t bufferSize, uint32_t &readSize)
{
    if (desiredSize == 0 || outBuffer == nullptr || desiredSize > bufferSize) {
        IMAGE_LOGE("[BufferSourceStream]input the parameter exception, desiredSize:%{public}u, bufferSize:%{public}u.",
                   desiredSize, bufferSize);
        return false;
    }
    if (dataSize_ == dataOffset_) {
        IMAGE_LOGE("[BufferSourceStream]buffer read finish, offset:%{public}zu ,dataSize%{public}zu.", dataOffset_,
                   dataSize_);
        return false;
    }
    if (desiredSize > dataSize_ - dataOffset_) {
        desiredSize = dataSize_ - dataOffset_;
    }
    errno_t ret = memcpy_s(outBuffer, bufferSize, inputBuffer_ + dataOffset_, desiredSize);
    if (ret != 0) {
        IMAGE_LOGE("[BufferSourceStream]copy data fail, ret:%{public}d, bufferSize:%{public}u, offset:%{public}zu,\
                   desiredSize:%{public}u.",
                   ret, bufferSize, dataOffset_, desiredSize);
        return false;
    }
    readSize = desiredSize;
    return true;
}

uint32_t BufferSourceStream::Tell()
{
    return dataOffset_;
}

bool BufferSourceStream::Seek(uint32_t position)
{
    if (position > dataSize_) {
        IMAGE_LOGE("[BufferSourceStream]Seek the position greater than the Data Size,position:%{public}u.", position);
        return false;
    }
    dataOffset_ = position;
    return true;
}

size_t BufferSourceStream::GetStreamSize()
{
    return dataSize_;
}

uint8_t *BufferSourceStream::GetDataPtr()
{
    return inputBuffer_;
}

uint32_t BufferSourceStream::GetStreamType()
{
    return ImagePlugin::BUFFER_SOURCE_TYPE;
}
}  // namespace Media
}  // namespace OHOS