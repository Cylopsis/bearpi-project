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

#ifndef NINE_PATCH_LISTENER_H
#define NINE_PATCH_LISTENER_H

#include <cstdint>
#include <cstdio>
#include <string>
#include "nocopyable.h"
#include "png_ninepatch_res.h"

namespace OHOS {
namespace ImagePlugin {
class NinePatchListener {
public:
    NinePatchListener() : patch_(nullptr), patchSize_(0)
    {}
    ~NinePatchListener()
    {
        if (patch_ != nullptr) {
            free(patch_);
            patch_ = nullptr;
        }
    }
    bool ReadChunk(const std::string &tag, void *data, size_t length);
    void Scale(float scaleX, float scaleY, int32_t scaledWidth, int32_t scaledHeight);
    PngNinePatchRes *patch_ = nullptr;
    size_t patchSize_ = 0;

private:
    DISALLOW_COPY_AND_MOVE(NinePatchListener);
};
} // namespace ImagePlugin
} // namespace OHOS
#endif // NINE_PATCH_LISTENER_H
