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

#ifndef INPUT_DATA_STREAM_H
#define INPUT_DATA_STREAM_H

#include <cstdint>
#include "nocopyable.h"

namespace OHOS {
namespace ImagePlugin {
enum {
    BUFFER_SOURCE_TYPE,
    INPUT_STREAM_TYPE,
    FILE_STREAM_TYPE,
};

struct DataStreamBuffer {
    // Out: output a pointer containing a data buffer.
    // the buffer is managed by SourceStream, and the user does not need to alloc for a buffer himself.
    // and the buffer is guaranteed to remain valid until the next operation on the SourceStream object.
    const uint8_t *inputStreamBuffer = nullptr;
    // Out: output buffer size.
    uint32_t bufferSize = 0;
    // Out: output actual valid data size in the buffer.
    uint32_t dataSize = 0;
};

class InputDataStream : NoCopyable {
public:
    // extracts desiredSize bytes from the InputDataStream.
    virtual bool Read(uint32_t desiredSize, DataStreamBuffer &outData) = 0;

    // need to copy desiredSize bytes from the InputDataStream to outBuffer.
    virtual bool Read(uint32_t desiredSize, uint8_t *outBuffer, uint32_t bufferSize, uint32_t &readSize) = 0;

    // output the remaining data in the InputDataStream, without extracting it.
    virtual bool Peek(uint32_t desiredSize, DataStreamBuffer &outData) = 0;

    // need to copy desiredSize bytes from the InputDataStream to outBuffer and without extracting it.
    virtual bool Peek(uint32_t desiredSize, uint8_t *outBuffer, uint32_t bufferSize, uint32_t &readSize) = 0;

    // get the position of the current byte in the InputDataStream.
    virtual uint32_t Tell() = 0;

    // sets the position of the next byte to be extracted from the input stream.
    virtual bool Seek(uint32_t position) = 0;

    // get inherited class type
    virtual uint32_t GetStreamType()
    {
        return -1;
    }

    // get raw pointer for BUFFER TYPE
    virtual uint8_t *GetDataPtr()
    {
        return nullptr;
    }

    // whether the stream data is completed or not.
    virtual bool IsStreamCompleted()
    {
        return true;
    }

    // get stream size
    virtual size_t GetStreamSize()
    {
        return 0;
    }

    virtual ~InputDataStream() {}
};
} // namespace ImagePlugin
} // namespace OHOS

#endif // INPUT_DATA_STREAM_H
