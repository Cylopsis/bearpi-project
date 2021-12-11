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

#include "istream_source_stream.h"
#include "image_log.h"
#include "image_utils.h"

namespace OHOS {
namespace Media {
using namespace OHOS::HiviewDFX;
using namespace std;
using namespace ImagePlugin;

IstreamSourceStream::IstreamSourceStream(unique_ptr<istream> inputStream, size_t size, size_t original, size_t offset)
    : inputStream_(move(inputStream)), streamSize_(size), streamOriginalOffset_(original), streamOffset_(offset)
{}

IstreamSourceStream::~IstreamSourceStream()
{
    ResetReadBuffer();
}

std::unique_ptr<IstreamSourceStream> IstreamSourceStream::CreateSourceStream(unique_ptr<istream> inputStream)
{
    if ((inputStream == nullptr) || (inputStream->rdbuf() == nullptr)) {
        IMAGE_LOGE("[IstreamSourceStream]input parameter exception.");
        return nullptr;
    }
    size_t streamSize = 0;
    if (!ImageUtils::GetInputStreamSize(*(inputStream.get()), streamSize)) {
        IMAGE_LOGE("[IstreamSourceStream]Get the input stream exception.");
        return nullptr;
    }
    if (streamSize == 0) {
        IMAGE_LOGE("[IstreamSourceStream]input stream size exception.");
        return nullptr;
    }
    size_t original = inputStream->tellg();
    size_t offset = original;
    return (unique_ptr<IstreamSourceStream>(new IstreamSourceStream(move(inputStream), streamSize, original, offset)));
}

bool IstreamSourceStream::Read(uint32_t desiredSize, DataStreamBuffer &outData)
{
    if (desiredSize == 0) {
        IMAGE_LOGE("[IstreamSourceStream]read stream input parameter exception.");
        return false;
    }
    if (!GetData(desiredSize, outData)) {
        IMAGE_LOGE("[IstreamSourceStream]read fail.");
        return false;
    }
    streamOffset_ += outData.dataSize;
    return true;
}

bool IstreamSourceStream::Peek(uint32_t desiredSize, DataStreamBuffer &outData)
{
    if (desiredSize == 0) {
        IMAGE_LOGE("[IstreamSourceStream]peek stream input parameter exception.");
        return false;
    }
    if (!GetData(desiredSize, outData)) {
        IMAGE_LOGE("[IstreamSourceStream]peek fail.");
        return false;
    }
    inputStream_->seekg(streamOffset_);
    return true;
}

bool IstreamSourceStream::Read(uint32_t desiredSize, uint8_t *outBuffer, uint32_t bufferSize, uint32_t &readSize)
{
    if (desiredSize == 0 || outBuffer == nullptr || desiredSize > bufferSize) {
        IMAGE_LOGE("[IstreamSourceStream]input the parameter exception, desiredSize:%{public}d, bufferSize:%{public}d.",
                   desiredSize, bufferSize);
        return false;
    }
    if (!GetData(desiredSize, outBuffer, bufferSize, readSize)) {
        IMAGE_LOGE("[IstreamSourceStream]read fail.");
        return false;
    }
    streamOffset_ += readSize;
    return true;
}

bool IstreamSourceStream::Peek(uint32_t desiredSize, uint8_t *outBuffer, uint32_t bufferSize, uint32_t &readSize)
{
    if (desiredSize == 0 || outBuffer == nullptr || desiredSize > bufferSize) {
        IMAGE_LOGE("[IstreamSourceStream]input the parameter exception, desiredSize:%{public}d, bufferSize:%{public}d.",
                   desiredSize, bufferSize);
        return false;
    }
    if (!GetData(desiredSize, outBuffer, bufferSize, readSize)) {
        IMAGE_LOGE("[IstreamSourceStream]peek fail.");
        return false;
    }
    inputStream_->seekg(streamOffset_);
    return true;
}

bool IstreamSourceStream::Seek(uint32_t position)
{
    if (position > streamSize_) {
        IMAGE_LOGE("[IstreamSourceStream]Seek the position error, position:%{public}u, streamSize_:%{public}zu.",
                   position, streamSize_);
        return false;
    }
    size_t targetPosition = position + streamOriginalOffset_;
    streamOffset_ = ((targetPosition < streamSize_) ? targetPosition : streamSize_);
    inputStream_->seekg(streamOffset_);
    return true;
}

uint32_t IstreamSourceStream::Tell()
{
    return streamOffset_;
}

bool IstreamSourceStream::GetData(uint32_t desiredSize, uint8_t *outBuffer, uint32_t bufferSize, uint32_t &readSize)
{
    if (streamSize_ == 0 || streamOffset_ >= streamSize_) {
        IMAGE_LOGE("[IstreamSourceStream]get source data fail. streamSize:%{public}zu, streamOffset:%{public}zu.",
                   streamSize_, streamOffset_);
        return false;
    }
    if (desiredSize > (streamSize_ - streamOffset_)) {
        desiredSize = (streamSize_ - streamOffset_);
    }
    if (!inputStream_->read(reinterpret_cast<char *>(outBuffer), desiredSize)) {
        IMAGE_LOGE("[IstreamSourceStream]read the inputstream fail.");
        return false;
    }
    readSize = desiredSize;
    return true;
}

bool IstreamSourceStream::GetData(uint32_t desiredSize, DataStreamBuffer &outData)
{
    if (streamSize_ == 0 || streamOffset_ >= streamSize_) {
        IMAGE_LOGE("[IstreamSourceStream]get source data fail. streamSize:%{public}zu, streamOffset:%{public}zu.",
                   streamSize_, streamOffset_);
        return false;
    }
    ResetReadBuffer();
    databuffer_ = static_cast<uint8_t *>(malloc(desiredSize));
    if (databuffer_ == nullptr) {
        IMAGE_LOGE("[IstreamSourceStream]malloc the output data buffer fail.");
        return false;
    }
    outData.bufferSize = desiredSize;
    if (desiredSize > (streamSize_ - streamOffset_)) {
        desiredSize = (streamSize_ - streamOffset_);
    }
    inputStream_->seekg(streamOffset_);
    if (!inputStream_->read(reinterpret_cast<char *>(databuffer_), desiredSize)) {
        IMAGE_LOGE("[IstreamSourceStream]read the inputstream fail.");
        return false;
    }
    outData.inputStreamBuffer = databuffer_;
    outData.dataSize = desiredSize;
    return true;
}

size_t IstreamSourceStream::GetStreamSize()
{
    return streamSize_;
}

uint8_t *IstreamSourceStream::GetDataPtr()
{
    return nullptr;
}

uint32_t IstreamSourceStream::GetStreamType()
{
    return ImagePlugin::INPUT_STREAM_TYPE;
}

void IstreamSourceStream::ResetReadBuffer()
{
    if (databuffer_ != nullptr) {
        free(databuffer_);
        databuffer_ = nullptr;
    }
}
} // namespace Media
} // namespace OHOS