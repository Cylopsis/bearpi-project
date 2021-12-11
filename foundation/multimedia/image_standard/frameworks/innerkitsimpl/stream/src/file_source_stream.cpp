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

#include <cerrno>
#include "directory_ex.h"
#include "file_source_stream.h"
#include "image_log.h"
#include "image_utils.h"
#include "media_errors.h"

namespace OHOS {
namespace Media {
using namespace OHOS::HiviewDFX;
using namespace std;
using namespace ImagePlugin;

FileSourceStream::FileSourceStream(std::FILE *file, size_t size, size_t offset, size_t original)
    : filePtr_(file), fileSize_(size), fileOffset_(offset), fileOriginalOffset_(original)
{}

FileSourceStream::~FileSourceStream()
{
    fclose(filePtr_);
    ResetReadBuffer();
}

unique_ptr<FileSourceStream> FileSourceStream::CreateSourceStream(const string &pathName)
{
    string realPath;
    if (!PathToRealPath(pathName, realPath)) {
        IMAGE_LOGE("[FileSourceStream]input the file path exception.");
        return nullptr;
    }
    size_t size = 0;
    if (!ImageUtils::GetFileSize(realPath, size)) {
        IMAGE_LOGE("[FileSourceStream]get the file size fail.");
        return nullptr;
    }
    FILE *filePtr = fopen(realPath.c_str(), "rb");
    if (filePtr == nullptr) {
        IMAGE_LOGE("[FileSourceStream]open file fail, error:%{public}s.", strerror(errno));
        return nullptr;
    }
    long offset = ftell(filePtr);
    if (offset < 0) {
        IMAGE_LOGE("[FileSourceStream]get the position fail.");
        fclose(filePtr);
        return nullptr;
    }
    return (unique_ptr<FileSourceStream>(new FileSourceStream(filePtr, size, offset, offset)));
}

bool FileSourceStream::Read(uint32_t desiredSize, DataStreamBuffer &outData)
{
    if (desiredSize == 0 || filePtr_ == nullptr) {
        IMAGE_LOGE("[FileSourceStream]read stream input parameter exception.");
        return false;
    }
    if (!GetData(desiredSize, outData)) {
        IMAGE_LOGE("[FileSourceStream]read fail.");
        return false;
    }
    fileOffset_ += outData.dataSize;
    return true;
}

bool FileSourceStream::Peek(uint32_t desiredSize, DataStreamBuffer &outData)
{
    if (desiredSize == 0 || filePtr_ == nullptr) {
        IMAGE_LOGE("[FileSourceStream]peek stream input parameter exception.");
        return false;
    }
    if (!GetData(desiredSize, outData)) {
        IMAGE_LOGE("[FileSourceStream]peek fail.");
        return false;
    }
    int ret = fseek(filePtr_, fileOffset_, SEEK_SET);
    if (ret != 0) {
        IMAGE_LOGE("[FileSourceStream]go to original position fail, ret:%{public}d.", ret);
        return false;
    }
    return true;
}

bool FileSourceStream::Read(uint32_t desiredSize, uint8_t *outBuffer, uint32_t bufferSize, uint32_t &readSize)
{
    if (desiredSize == 0 || outBuffer == nullptr || desiredSize > bufferSize || desiredSize > fileSize_) {
        IMAGE_LOGE("[FileSourceStream]input parameter exception, desiredSize:%{public}u, bufferSize:%{public}u,\
                   fileSize_:%{public}zu.",
                   desiredSize, bufferSize, fileSize_);
        return false;
    }
    if (!GetData(desiredSize, outBuffer, bufferSize, readSize)) {
        IMAGE_LOGE("[FileSourceStream]read fail.");
        return false;
    }
    fileOffset_ += readSize;
    return true;
}

bool FileSourceStream::Peek(uint32_t desiredSize, uint8_t *outBuffer, uint32_t bufferSize, uint32_t &readSize)
{
    if (desiredSize == 0 || outBuffer == nullptr || desiredSize > bufferSize || desiredSize > fileSize_) {
        IMAGE_LOGE("[FileSourceStream]input parameter exception, desiredSize:%{public}u, bufferSize:%{public}u,\
                   fileSize_:%{public}zu.",
                   desiredSize, bufferSize, fileSize_);
        return false;
    }
    if (!GetData(desiredSize, outBuffer, bufferSize, readSize)) {
        IMAGE_LOGE("[FileSourceStream]peek fail.");
        return false;
    }
    int ret = fseek(filePtr_, fileOffset_, SEEK_SET);
    if (ret != 0) {
        IMAGE_LOGE("[FileSourceStream]go to original position fail, ret:%{public}d.", ret);
        return false;
    }
    return true;
}

bool FileSourceStream::Seek(uint32_t position)
{
    if (position > fileSize_) {
        IMAGE_LOGE("[FileSourceStream]Seek the position greater than the file size, position:%{public}u.", position);
        return false;
    }
    size_t targetPosition = position + fileOriginalOffset_;
    fileOffset_ = ((targetPosition < fileSize_) ? targetPosition : fileSize_);
    int ret = fseek(filePtr_, fileOffset_, SEEK_SET);
    if (ret != 0) {
        IMAGE_LOGE("[FileSourceStream]go to offset position fail, ret:%{public}d.", ret);
        return false;
    }
    return true;
}

uint32_t FileSourceStream::Tell()
{
    return fileOffset_;
}

bool FileSourceStream::GetData(uint32_t desiredSize, uint8_t *outBuffer, uint32_t bufferSize, uint32_t &readSize)
{
    if (fileSize_ == fileOffset_) {
        IMAGE_LOGE("[FileSourceStream]read finish, offset:%{public}zu ,dataSize%{public}zu.", fileOffset_, fileSize_);
        return false;
    }
    if (desiredSize > (fileSize_ - fileOffset_)) {
        desiredSize = fileSize_ - fileOffset_;
    }
    size_t bytesRead = fread(outBuffer, sizeof(outBuffer[0]), desiredSize, filePtr_);
    if (bytesRead < desiredSize) {
        IMAGE_LOGE("[FileSourceStream]read fail, bytesRead:%{public}zu", bytesRead);
        return false;
    }
    readSize = desiredSize;
    return true;
}

bool FileSourceStream::GetData(uint32_t desiredSize, DataStreamBuffer &outData)
{
    if (fileSize_ == fileOffset_) {
        IMAGE_LOGE("[FileSourceStream]read finish, offset:%{public}zu ,dataSize%{public}zu.", fileOffset_, fileSize_);
        return false;
    }

    ResetReadBuffer();
    readBuffer_ = static_cast<uint8_t *>(malloc(desiredSize));
    if (readBuffer_ == nullptr) {
        IMAGE_LOGE("[FileSourceStream]malloc the desiredSize fail.");
        return false;
    }
    outData.bufferSize = desiredSize;
    if (desiredSize > (fileSize_ - fileOffset_)) {
        desiredSize = fileSize_ - fileOffset_;
    }
    size_t bytesRead = fread(readBuffer_, sizeof(uint8_t), desiredSize, filePtr_);
    if (bytesRead < desiredSize) {
        IMAGE_LOGE("[FileSourceStream]read fail, bytesRead:%{public}zu", bytesRead);
        return false;
    }
    outData.inputStreamBuffer = static_cast<uint8_t *>(readBuffer_);
    outData.dataSize = desiredSize;
    return true;
}

size_t FileSourceStream::GetStreamSize()
{
    return fileSize_;
}

uint8_t *FileSourceStream::GetDataPtr()
{
    return nullptr;
}

uint32_t FileSourceStream::GetStreamType()
{
    return ImagePlugin::FILE_STREAM_TYPE;
}

void FileSourceStream::ResetReadBuffer()
{
    if (readBuffer_ != nullptr) {
        free(readBuffer_);
        readBuffer_ = nullptr;
    }
}
} // namespace Media
} // namespace OHOS