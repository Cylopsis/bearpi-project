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

#ifndef PNG_NINEPATCH_RES_H
#define PNG_NINEPATCH_RES_H

#include <cstdint>
#include <cstdio>

namespace OHOS {
namespace ImagePlugin {
struct alignas(uintptr_t) PngNinePatchRes {
    PngNinePatchRes() : wasDeserialized(false), xDivsOffset(0), yDivsOffset(0), colorsOffset(0)
    {}
    ~PngNinePatchRes() = default;
    void DeviceToFile();
    void FileToDevice();
    static PngNinePatchRes *Deserialize(void *data);
    size_t SerializedSize() const;
    inline int32_t *GetXDivs() const
    {
        return reinterpret_cast<int32_t *>(reinterpret_cast<uintptr_t>(this) + xDivsOffset);
    }
    inline int32_t *GetYDivs() const
    {
        return reinterpret_cast<int32_t *>(reinterpret_cast<uintptr_t>(this) + yDivsOffset);
    }
    inline uint32_t *GetColors() const
    {
        return reinterpret_cast<uint32_t *>(reinterpret_cast<uintptr_t>(this) + colorsOffset);
    }
    int8_t wasDeserialized;
    uint8_t numXDivs;
    uint8_t numYDivs;
    uint8_t numColors;
    uint32_t xDivsOffset;
    uint32_t yDivsOffset;
    int32_t paddingLeft;
    int32_t paddingRight;
    int32_t paddingTop;
    int32_t paddingBottom;
    // The offset (from the start of this structure) to the colors array
    uint32_t colorsOffset;
} __attribute__((packed));
} // namespace ImagePlugin
} // namespace OHOS
#endif // PNG_NINEPATCH_RES_H
