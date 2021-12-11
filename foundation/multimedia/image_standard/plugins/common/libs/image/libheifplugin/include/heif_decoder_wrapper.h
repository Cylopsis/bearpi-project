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

#ifndef HEIF_DECODER_WRAPPER_H
#define HEIF_DECODER_WRAPPER_H

#include <memory>
#include "heif_decoder_interface.h"
#include "input_data_stream.h"

namespace OHOS {
namespace ImagePlugin {
class HeifDecoderWrapper {
public:
    static std::unique_ptr<HeifDecoderInterface> CreateHeifDecoderInterface(InputDataStream &stream);
};
} // namespace ImagePlugin
} // namespace OHOS

#endif // HEIF_DECODER_WRAPPER_H