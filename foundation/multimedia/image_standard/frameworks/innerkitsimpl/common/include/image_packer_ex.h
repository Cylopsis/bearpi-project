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

#ifndef IMAGE_PACKER_EX_H
#define IMAGE_PACKER_EX_H

#include "image_packer.h"
#include "packer_stream.h"

namespace OHOS {
namespace Media {
class ImagePackerEx : public ImagePacker {
public:
    ImagePackerEx() = default;
    ~ImagePackerEx() = default;
    using ImagePacker::StartPacking;
    uint32_t StartPacking(PackerStream &outputStream, const PackOption &option);

private:
    DISALLOW_COPY_AND_MOVE(ImagePackerEx);
};
} // namespace Media
} // namespace OHOS

#endif // IMAGE_PACKER_EX_H
