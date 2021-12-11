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

#include "incremental_source_stream.h"

#include <algorithm>
#include <vector>
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

IncrementalSourceStream::IncrementalSourceStream(IncrementalMode mode)
    : incrementalMode_(mode), isFinalize_(false), dataSize_(0), dataOffset_(0)
{}

unique_ptr<IncrementalSourceStream> IncrementalSourceStream::CreateSourceStream(IncrementalMode mode)
{
    IMAGE_LOGD("[IncrementalSourceStream]mode:%{public}d.", mode);
    return (unique_ptr<IncrementalSourceStream>(new IncrementalSourceStream(mode)));
}

bool IncrementalSourceStream::Read(uint32_t desiredSize, DataStreamBuffer &outData)
{
    if (!Peek(desiredSize, outData)) {
        IMAGE_LOGE("[IncrementalSourceStream]read fail.");
        return false;
    }
    dataOffset_ += outData.dataSize;
    return true;
}

bool IncrementalSourceStream::Peek(uint32_t desiredSize, DataStreamBuffer &outData)
{
    if (desiredSize == 0) {
        IMAGE_LOGE("[IncrementalSourceStream]input the parameter exception.");
        return false;
    }
    if (sourceData_.empty() || dataSize_ == 0 || dataOffset_ >= dataSize_) {
        IMAGE_LOGE("[IncrementalSourceStream]source data exception. dataSize_:%{public}zu, dataOffset_:%{public}zu.",
                   dataSize_, dataOffset_);
        return false;
    }
    outData.bufferSize = dataSize_ - dataOffset_;
    if (desiredSize > dataSize_ - dataOffset_) {
        desiredSize = dataSize_ - dataOffset_;
    }
    outData.dataSize = desiredSize;
    outData.inputStreamBuffer = sourceData_.data() + dataOffset_;
    IMAGE_LOGD("[IncrementalSourceStream]Peek end. desiredSize:%{public}u, offset:%{public}zu, dataSize_:%{public}zu,\
               dataOffset_:%{public}zu.",
               desiredSize, dataOffset_, dataSize_, dataOffset_);
    return true;
}

bool IncrementalSourceStream::Read(uint32_t desiredSize, uint8_t *outBuffer, uint32_t bufferSize, uint32_t &readSize)
{
    if (!Peek(desiredSize, outBuffer, bufferSize, readSize)) {
        IMAGE_LOGE("[IncrementalSourceStream]read fail.");
        return false;
    }
    dataOffset_ += readSize;
    return true;
}

bool IncrementalSourceStream::Peek(uint32_t desiredSize, uint8_t *outBuffer, uint32_t bufferSize, uint32_t &readSize)
{
    if (desiredSize == 0 || outBuffer == nullptr || desiredSize > bufferSize) {
        IMAGE_LOGE("[IncrementalSourceStream]input parameter exception, desiredSize:%{public}u, bufferSize:%{public}u.",
                   desiredSize, bufferSize);
        return false;
    }
    if (sourceData_.empty() || dataSize_ == 0 || dataOffset_ >= dataSize_) {
        IMAGE_LOGE("[IncrementalSourceStream]source data exception. dataSize_:%{public}zu, dataOffset_:%{public}zu.",
                   dataSize_, dataOffset_);
        return false;
    }
    if (desiredSize > (dataSize_ - dataOffset_)) {
        desiredSize = dataSize_ - dataOffset_;
    }
    errno_t ret = memcpy_s(outBuffer, bufferSize, sourceData_.data() + dataOffset_, desiredSize);
    if (ret != 0) {
        IMAGE_LOGE("[IncrementalSourceStream]copy data fail, ret:%{public}d, bufferSize:%{public}u, offset:%{public}zu,\
                   desiredSize:%{public}u, dataSize:%{public}zu.",
                   ret, bufferSize, dataOffset_, desiredSize, dataSize_);
        return false;
    }
    readSize = desiredSize;
    return true;
}

uint32_t IncrementalSourceStream::Tell()
{
    return dataOffset_;
}

bool IncrementalSourceStream::Seek(uint32_t position)
{
    if (position >= dataSize_) {
        IMAGE_LOGE("[IncrementalSourceStream]Seek the position greater than the Data Size.");
        return false;
    }
    dataOffset_ = position;
    return true;
}

uint32_t IncrementalSourceStream::UpdateData(const uint8_t *data, uint32_t size, bool isCompleted)
{
    if (data == nullptr) {
        IMAGE_LOGE("[IncrementalSourceStream]input the parameter exception.");
        return ERR_IMAGE_DATA_ABNORMAL;
    }
    if (size == 0) {
        IMAGE_LOGD("[IncrementalSourceStream]no need to update data.");
        return SUCCESS;
    }
    if (incrementalMode_ == IncrementalMode::INCREMENTAL_DATA) {
        vector<uint8_t> newData;
        newData.resize(size);
        copy(data, data + size, newData.begin());
        sourceData_.resize(dataSize_ + size);
        sourceData_.insert(sourceData_.begin() + dataSize_, newData.begin(), newData.end());
        dataSize_ += size;
        isFinalize_ = isCompleted;
    } else {
        sourceData_.clear();
        sourceData_.resize(size);
        dataSize_ = size;
        copy(data, data + size, sourceData_.begin());
        isFinalize_ = true;
    }
    return SUCCESS;
}

bool IncrementalSourceStream::IsStreamCompleted()
{
    return isFinalize_;
}

size_t IncrementalSourceStream::GetStreamSize()
{
    return dataSize_;
}
} // namespace Media
} // namespace OHOS
