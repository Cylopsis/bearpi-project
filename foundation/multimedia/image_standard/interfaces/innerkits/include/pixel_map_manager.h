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

#ifndef PIXEL_MAP_MANAGER_H
#define PIXEL_MAP_MANAGER_H

#include "pixel_map.h"

namespace OHOS {
namespace Media {
class PixelMapManager {
public:
    explicit PixelMapManager(PixelMap *pixelMap) : pixelMap_(pixelMap)
    {}

    void FreePixels()
    {
        pixelMap_.clear();
    }

    bool Invalid()
    {
        return pixelMap_ == nullptr;
    }

    PixelMap &GetPixelMap()
    {
        return *pixelMap_;
    }

    int32_t GetByteCount()
    {
        return pixelMap_->GetByteCount();
    }

    void Ref()
    {
        pixelMap_->IncStrongRef(nullptr);
    }

    void UnRef()
    {
        pixelMap_->DecStrongRef(nullptr);
    }

    ~PixelMapManager()
    {}

private:
    ::OHOS::sptr<PixelMap> pixelMap_ = nullptr;
};
} // namespace Media
} // namespace OHOS
#endif // PIXEL_MAP_MANAGER_H
