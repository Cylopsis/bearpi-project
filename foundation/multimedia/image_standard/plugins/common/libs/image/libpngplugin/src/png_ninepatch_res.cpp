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

#include "png_ninepatch_res.h"
#ifdef _WIN32
#include <winsock2.h>
#else
#include <arpa/inet.h>
#endif

namespace OHOS {
namespace ImagePlugin {
static void Fill9patchOffsets(PngNinePatchRes *patch)
{
    patch->xDivsOffset = sizeof(PngNinePatchRes);
    patch->yDivsOffset = patch->xDivsOffset + (patch->numXDivs * sizeof(int32_t));
    patch->colorsOffset = patch->yDivsOffset + (patch->numYDivs * sizeof(int32_t));
}

PngNinePatchRes *PngNinePatchRes::Deserialize(void *inData)
{
    PngNinePatchRes *patch = static_cast<PngNinePatchRes *>(inData);
    patch->wasDeserialized = true;
    Fill9patchOffsets(patch);

    return patch;
}

void PngNinePatchRes::DeviceToFile()
{
    int32_t *xDivs = GetXDivs();
    for (int i = 0; i < numXDivs; i++) {
        xDivs[i] = htonl(xDivs[i]);
    }
    int32_t *yDivs = GetYDivs();
    for (int i = 0; i < numYDivs; i++) {
        yDivs[i] = htonl(yDivs[i]);
    }
    paddingTop = htonl(paddingTop);
    paddingBottom = htonl(paddingBottom);
    paddingLeft = htonl(paddingLeft);
    paddingRight = htonl(paddingRight);
    uint32_t *colors = GetColors();
    for (int i = 0; i < numColors; i++) {
        colors[i] = htonl(colors[i]);
    }
}

void PngNinePatchRes::FileToDevice()
{
    int32_t *xDivs = GetXDivs();
    for (int i = 0; i < numXDivs; i++) {
        xDivs[i] = ntohl(xDivs[i]);
    }
    int32_t *yDivs = GetYDivs();
    for (int i = 0; i < numYDivs; i++) {
        yDivs[i] = ntohl(yDivs[i]);
    }
    paddingTop = ntohl(paddingTop);
    paddingBottom = ntohl(paddingBottom);
    paddingLeft = ntohl(paddingLeft);
    paddingRight = ntohl(paddingRight);
    uint32_t *colors = GetColors();
    for (int i = 0; i < numColors; i++) {
        colors[i] = ntohl(colors[i]);
    }
}

size_t PngNinePatchRes::SerializedSize() const
{
    // The size of this struct is 32 bytes on the 32-bit target system
    return 32 + numXDivs * sizeof(int32_t) + numYDivs * sizeof(int32_t) + numColors * sizeof(uint32_t);
}
} // namespace ImagePlugin
} // namespace OHOS