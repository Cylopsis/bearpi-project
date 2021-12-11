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

#ifndef SOURCE_STREAM_H
#define SOURCE_STREAM_H

#include <cinttypes>
#include "image/input_data_stream.h"
#include "media_errors.h"

namespace OHOS {
namespace Media {
class SourceStream : public ImagePlugin::InputDataStream {
public:
    virtual uint32_t UpdateData(const uint8_t *data, uint32_t size, bool isCompleted)
    {
        return ERR_IMAGE_DATA_UNSUPPORT;
    }
};
} // namespace Media
} // namespace OHOS

#endif // SOURCE_STREAM_H